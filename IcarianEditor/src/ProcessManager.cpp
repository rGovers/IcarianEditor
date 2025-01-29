// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "ProcessManager.h"
#include "Core/IPCPipe.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#ifndef WIN32
#include <csignal>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#include <cstdlib>
#include <filesystem>
#include <string>

#include "Core/DMASwapBuffer.h"
#include "Core/IcarianDefer.h"
#include "Logger.h"
#include "ProfilerData.h"
#include "SSHPipe.h"

static std::filesystem::path GetAddr(const std::string_view& a_addr)
{
    return std::filesystem::temp_directory_path() / a_addr;
}

#ifndef WIN32
// Not available in all versions of glibc yet so have to manually invoke syscalls
// Using wrapper as it takes in va args and want actual arguments
// NOTE: This should be replaced if it arrives in glibc

// https://man7.org/linux/man-pages/man2/pidfd_open.2.html
static int sys_pidfd_open(pid_t a_pid, unsigned int a_flags)
{
    return (int)syscall(SYS_pidfd_open, a_pid, a_flags);
}

// NOTE: As it is still new flags is reserved so just pass 0
// https://man7.org/linux/man-pages/man2/pidfd_getfd.2.html
static int sys_pidfd_getfd(int a_pidfd, int a_targetfd, unsigned int a_flags)
{
    return (int)syscall(SYS_pidfd_getfd, a_pidfd, a_targetfd, a_flags);
}
#endif

ProcessManager::ProcessManager()
{
    m_remotePipe = nullptr;
    m_dmaSwaps = 0;

    m_ipcPipe = nullptr;

#if WIN32
    WSADATA wsaData = { };
    ICARIAN_ASSERT_MSG_R(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0, "Failed to start WSA");
#endif

    m_resize = false;

    m_width = 1280;
    m_height = 720;

    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_width, (GLsizei)m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_fps = 0.0;
    m_frameTime = 0.0;
    m_frames = 0;

    m_ups = 0.0;
    m_updateTime = 0.0;
    m_updates = 0;
}   
ProcessManager::~ProcessManager()
{
    glDeleteTextures(1, &m_tex);

    if (m_remotePipe != nullptr)
    {
        delete m_remotePipe;
    }

#if WIN32    
    WSACleanup();
#endif
}

bool ProcessManager::IsRemoteConnected() const
{
    return m_remotePipe != nullptr && m_remotePipe->IsAlive();
}

void ProcessManager::FlushDMAImages()
{
    m_curFrame = 0;
    m_dmaSwaps = 0;

    for (const DMASwapchainImage& image : m_dmaImages)
    {
        glDeleteTextures(1, &image.Texture);
        glDeleteMemoryObjectsEXT(1, &image.MemoryObject);
        
        glDeleteSemaphoresEXT(1, &image.StartSemaphore);
        glDeleteSemaphoresEXT(1, &image.EndSemaphore);
    }

    m_dmaImages.clear();
}
void ProcessManager::Terminate()
{
    if (m_ipcPipe != nullptr)
    {
        delete m_ipcPipe;
        m_ipcPipe = nullptr;
    }

#if WIN32
    if (m_processInfo.hProcess != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_processInfo.hProcess);
        m_processInfo.hProcess = INVALID_HANDLE_VALUE;
    }
    if (m_processInfo.hThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_processInfo.hThread);
        m_processInfo.hThread = INVALID_HANDLE_VALUE;
    }
#else
    if (m_process > 0)
    {
        kill(m_process, SIGTERM);
        m_process = -1;
    }
#endif
}

bool ProcessManager::ConnectRemotePassword(const std::string_view& a_addr, uint16_t a_port, uint16_t a_scpPort, uint16_t a_clientPort)
{
#ifndef WIN32
    if (m_remotePipe != nullptr)
    {
        return false;
    }

    m_clientPort = a_clientPort;
    m_remotePipe = SSHPipe::ConnectPassword(a_addr, a_port, a_scpPort);

    return m_remotePipe != nullptr;
#endif

    return false;
}

bool ProcessManager::Start(const std::filesystem::path& a_workingDir)
{
    Logger::Message("Spawning IcarianEngine Instance");

    m_dmaMode = false;
    m_dmaSwaps = 0;
    m_curFrame = 0;

    ProfilerData::Clear();

    m_captureInput = true;
    m_cursorState = CursorState_Normal;

    const std::string workingDirArg = "--wDir=" + a_workingDir.string();
#if WIN32
    const IcarianCore::IPCPipe* serverPipe = IcarianCore::IPCPipe::Create(GetAddr(PipeName).string());
    if (serverPipe == nullptr)
    {
        Logger::Error("Failed to create IPC Pipe");

        return false;
    }
    IDEFER(delete serverPipe);

    const std::string args = "IcarianNative.exe --headless " + workingDirArg;

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&m_processInfo, sizeof(m_processInfo));

    si.cb = sizeof(si);

    const BOOL result = CreateProcess
    (
        "IcarianNative.exe",
        (LPSTR)args.c_str(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &m_processInfo
    );
    if (!result)
    {
        Logger::Error("Failed to spawn process: " + std::to_string(GetLastError()));

        return false;
    }
    
    m_processHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, m_processInfo.dwProcessId);
    if (m_processHandle == INVALID_HANDLE_VALUE)
    {
        Logger::Error("Failed to open process: " + std::to_string(GetLastError()));
        Terminate();

        return false;
    }

    m_pipe = serverPipe->Accept();
    if (m_pipe == nullptr)
    {
        Logger::Error("Failed to connect to IcarianEngine");
        Terminate();

        return false;
    }

    const glm::ivec2 data = glm::ivec2((int)m_width, (int)m_height);

    if (!m_pipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data }))
    {
        Logger::Error("Failed to send resize message to IcarianEngine");
        Terminate();

        return false;
    }

    return true;
#else
    if (m_process == -1)
    {
        const IcarianCore::IPCPipe* serverPipe = IcarianCore::IPCPipe::Create(GetAddr(PipeName).string());
        if (serverPipe == nullptr)
        {
            Logger::Error("Failed to create IPC Pipe");

            return false;
        }
        IDEFER(delete serverPipe);

        // This is a bit odd leaving this here as a note
        // This create another copy of the process on unix systems 
        // You can tell if you are the parent or child process based on the return result
        // If I am the child process I run the execute process which overwrites the current process with the new process
        // Unix systems are a bit odd but it works so I am not gonna question it
        m_process = fork();

        if (m_process < 0)
        {
            Logger::Error("Process Fork Failed");
            perror("fork");

            return false;
        }
        else if (m_process == 0)
        {
            // Starting the engine
            // In a weird state cause in a forked process so doing stuff C style
            // Once execution is started state is normal again
            if (execl("./IcarianNative", "--headless", workingDirArg.c_str(), NULL) < 0)
            {
                printf("Failed to start process \n");
                perror("execl");
                assert(0);
            }
        }
        else
        {            
            // State is correct cause in the parent process so can use inbuilt stuff
            m_ipcPipe = serverPipe->Accept();
            if (m_ipcPipe == nullptr)
            {
                Logger::Error("Failed to connect to IcarianEngine");

                kill(m_process, SIGTERM);   
                m_process = -1;

                return false;
            }
            
            // Need to file descriptor for the process for later
            m_processFD = sys_pidfd_open(m_process, 0);
            if (m_processFD < 0)
            {
                Logger::Error("Failed to get pid fd for IcarianEngine");

                Terminate();

                return false;
            }

            const glm::ivec2 data = glm::ivec2((int)m_width, (int)m_height);
            if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data }))
            {
                Logger::Error("Failed to send resize message to IcarianEngine");

                Terminate();

                return false;
            }

            return true;
        }
    }
#endif

    return false;
}

void ProcessManager::PollMessage(bool a_blockError)
{
    std::queue<IcarianCore::PipeMessage> messages;

    if (!m_ipcPipe->Receive(&messages))
    {
        if (!a_blockError)
        {
            Logger::Error("Failed to receive message from IcarianEngine");
        }

        Terminate();

        return;
    }

    while (!messages.empty())
    {
        const IcarianCore::PipeMessage msg = messages.front();
        IDEFER(
        if (msg.Data != nullptr)
        {
            delete[] msg.Data;
        });
        messages.pop();

        switch (msg.Type)
        {
        case IcarianCore::PipeMessageType_PushFrame:
        {
            m_dmaMode = false;

            if (msg.Length == m_width * m_height * 4)
            {
                glBindTexture(GL_TEXTURE_2D, m_tex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, msg.Data);
            }

            break;
        }
        case IcarianCore::PipeMessageType_FrameData:
        {
            const double delta = *(double*)(msg.Data + 0);
            const double time = *(double*)(msg.Data + 4);

            ++m_frames;

            m_frameTime -= delta;
            if (m_frameTime <= 0)
            {
                m_fps = m_frames * 2;
                m_frameTime += 0.5;
                m_frames = 0;
            }

            break;
        }
        case IcarianCore::PipeMessageType_PushDMASwapFDBuffer:
        {
#ifdef WIN32
            Logger::Error("DMA FD message on Windows");
#else
            m_dmaMode = true;

            const DMASwapBufferFD& swapBuffer = *(DMASwapBufferFD*)msg.Data;

            // I am a fucking idiot it is the process file descriptor it needs not the process id
            // We need to remap the file descriptors as they will not be valid in this process due to different descriptor tables
            const int imageFD = sys_pidfd_getfd(m_processFD, swapBuffer.ImageFD, 0);
            const int startSemaphore = sys_pidfd_getfd(m_processFD, swapBuffer.StartSemaphore, 0);
            const int endSemaphore = sys_pidfd_getfd(m_processFD, swapBuffer.EndSemaphore, 0);

            DMASwapchainImage image = 
            {
                .Width = swapBuffer.Width,
                .Height = swapBuffer.Height,
                .Offset = swapBuffer.Offset,
            };

            // Hmm weird do not know where the extra data is coming from for swapBuffer.Size but ehh as long as it works
            glCreateMemoryObjectsEXT(1, &image.MemoryObject);
            glImportMemoryFdEXT(image.MemoryObject, (GLuint64)swapBuffer.Size + swapBuffer.Offset, GL_HANDLE_TYPE_OPAQUE_FD_EXT, imageFD);

            glCreateTextures(GL_TEXTURE_2D, 1, &image.Texture);
            glTextureStorageMem2DEXT(image.Texture, 1, GL_RGBA8, (GLsizei)swapBuffer.Width, (GLsizei)swapBuffer.Height, image.MemoryObject, swapBuffer.Offset);
            glTextureParameteri(image.Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(image.Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(image.Texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(image.Texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glGenSemaphoresEXT(1, &image.StartSemaphore);
            glImportSemaphoreFdEXT(image.StartSemaphore, GL_HANDLE_TYPE_OPAQUE_FD_EXT, startSemaphore);

            glGenSemaphoresEXT(1, &image.EndSemaphore);
            glImportSemaphoreFdEXT(image.EndSemaphore, GL_HANDLE_TYPE_OPAQUE_FD_EXT, endSemaphore);

            // Shotgun the semaphores it is fine for it in the first couple frames it will reach a steady state on its own
            constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
            glSignalSemaphoreEXT(image.StartSemaphore, 0, NULL, 1, &image.Texture, &Layout);

            m_dmaImages.emplace_back(image);
#endif

            break;
        }
        case IcarianCore::PipeMessageType_PushDMASwapHandleBuffer:
        {
#ifndef WIN32
            Logger::Error("DMA Handle message on non Windows OS");
#else
            m_dmaMode = true;

            const DMASwapBufferHandle& swapBuffer = *(DMASwapBufferHandle*)msg.Data;

            HANDLE processHandle = GetCurrentProcess();

            // Different process so need to remap the HANDLE to be valid in the current proccess
            // Urgh... Had to go through Windows access control documentation and still did not get an answer so fuck it winging it, 
            // meanwhile Linux was just do they have a Unix domain socket open and sent and recieved data cool they have access
            // Windows documentation is good until you read other documentation
            HANDLE imageHandle;
            HANDLE startSemaphore;
            HANDLE endSemaphore;
            DuplicateHandle(m_processHandle, swapBuffer.ImageHandle, processHandle, &imageHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
            DuplicateHandle(m_processHandle, swapBuffer.StartSemaphore, processHandle, &startSemaphore, 0, FALSE, DUPLICATE_SAME_ACCESS);
            DuplicateHandle(m_processHandle, swapBuffer.EndSemaphore, processHandle, &endSemaphore, 0, FALSE, DUPLICATE_SAME_ACCESS);

            DMASwapchainImage image = 
            {
                .Width = swapBuffer.Width,
                .Height = swapBuffer.Height,
                .Offset = swapBuffer.Offset,
            };

            glCreateMemoryObjectsEXT(1, &image.MemoryObject);
            glImportMemoryWin32HandleEXT(image.MemoryObject, (GLuint64)swapBuffer.Size + swapBuffer.Offset, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, imageHandle);

            glCreateTextures(GL_TEXTURE_2D, 1, &image.Texture);
            glTextureStorageMem2DEXT(image.Texture, 1, GL_RGBA8, (GLsizei)swapBuffer.Width, (GLsizei)swapBuffer.Height, image.MemoryObject, swapBuffer.Offset);
            glTextureParameteri(image.Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(image.Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(image.Texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(image.Texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glGenSemaphoresEXT(1, &image.StartSemaphore);
            glImportSemaphoreWin32HandleEXT(image.StartSemaphore, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, startSemaphore);

            glGenSemaphoresEXT(1, &image.EndSemaphore);
            glImportSemaphoreWin32HandleEXT(image.EndSemaphore, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, endSemaphore);

            constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
            glSignalSemaphoreEXT(image.StartSemaphore, 0, NULL, 1, &image.Texture, &Layout);

            m_dmaImages.emplace_back(image);
#endif

            break;
        }
        case IcarianCore::PipeMessageType_FlushDMASwapFDBuffer:
        case IcarianCore::PipeMessageType_FlushDMASwapHandleBuffer:
        {
            FlushDMAImages();

            break;
        }
        case IcarianCore::PipeMessageType_DMASwap:
        {
            ++m_dmaSwaps;

            break;
        }
        case IcarianCore::PipeMessageType_SetCursorState:
        {
            m_cursorState = *(e_CursorState*)msg.Data;

            break;
        }
        case IcarianCore::PipeMessageType_UpdateData:
        {
            const double delta = *(double*)(msg.Data + 0);
            const double time = *(double*)(msg.Data + 4);

            ++m_updates;

            m_updateTime -= delta;
            if (m_updateTime <= 0)
            {
                m_ups = m_updates * 2;
                m_updateTime += 0.5f;
                m_updates = 0;
            }

            break;
        }
        case IcarianCore::PipeMessageType_Message:
        {
            constexpr uint32_t TypeSize = sizeof(e_LoggerMessageType);

            const std::string_view str = std::string_view(msg.Data + TypeSize, msg.Length - TypeSize);

            switch (*(e_LoggerMessageType*)msg.Data)
            {
            case LoggerMessageType_Message:
            {
                Logger::Message(str, false, false);

                break;
            }
            case LoggerMessageType_Warning:
            {
                Logger::Warning(str, false, false);

                break;
            }
            case LoggerMessageType_Error:
            {
                Logger::Error(str, false, false);

                break;
            }
            }

            break;
        }
        case IcarianCore::PipeMessageType_ProfileScope:
        {
            ProfilerData::PushData(*(ProfileScope*)msg.Data);

            break;
        }
        case IcarianCore::PipeMessageType_Close:
        {
#if WIN32
            m_processInfo.hProcess = INVALID_HANDLE_VALUE;
            m_processInfo.hThread = INVALID_HANDLE_VALUE;
#else
            m_process = -1;
#endif  
            if (m_ipcPipe != nullptr)
            {
                delete m_ipcPipe;
                m_ipcPipe = nullptr;
            }

            return;
        }
        case IcarianCore::PipeMessageType_Null:
        {
            break;
        }
        default:
        {
            Logger::Error("Editor: Invalid Pipe Message: " + std::to_string(msg.Type) + " " + std::to_string(msg.Length));

            break;
        }
        }
    }
}

void ProcessManager::Update()
{
    if (!IsRunning())
    {
#ifdef WIN32
        m_processInfo.hProcess = INVALID_HANDLE_VALUE;
        m_processInfo.hThread = INVALID_HANDLE_VALUE;
#else
        m_process = -1;
#endif

        if (m_ipcPipe != nullptr)
        {
            delete m_ipcPipe;
            m_ipcPipe = nullptr;
        }

        return;
    }

    PollMessage();

    if (m_ipcPipe == nullptr)
    {
        return;
    }

    // Engine only pushes one frame at a time in non DMA mode
    // Do it this way so the editor does not get overwhelmed with frame data
    // Cause extreme lag if I do not throttle the push frames ~1 fps
    // IPC pipes are only so fast
    if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_UnlockFrame }))
    {
        Logger::Error("Failed to send unlock frame message to IcarianEngine");

        Terminate();

        return; 
    }

    if (m_resize)
    {
        m_resize = false;

        const glm::ivec2 size = glm::ivec2((int)m_width, (int)m_height);

        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&size}))
        {
            FlushDMAImages();

            Logger::Error("Failed to send resize message to IcarianEngine");

            Terminate();

            return;   
        }
    }

    if (m_dmaMode)
    {
        while (m_dmaSwaps > 0)
        {
            IDEFER(--m_dmaSwaps);

            const DMASwapchainImage& img = m_dmaImages[m_curFrame];

            constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
            glWaitSemaphoreEXT(img.EndSemaphore, 0, NULL, 1, &img.Texture, &Layout);

            if (img.Width == m_width && img.Height == m_height)
            {
                // glTextureStorageMem2DEXT(img.Texture, 1, GL_RGBA8, (GLsizei)img.Width, (GLsizei)img.Height, img.MemoryObject, img.Offset);
                glCopyImageSubData(img.Texture, GL_TEXTURE_2D, 0, 0, 0, 0, m_tex, GL_TEXTURE_2D, 0, 0, 0, 0, img.Width, img.Height, 1);
            }

            m_curFrame = (m_curFrame + 1) % (uint32_t)m_dmaImages.size();

            const DMASwapchainImage& nextImage = m_dmaImages[m_curFrame];
            glSignalSemaphoreEXT(nextImage.StartSemaphore, 0, NULL, 1, &nextImage.Texture, &Layout);
        }
    }
}
void ProcessManager::Stop()
{
    Logger::Message("Stopping IcarianEngine Instance");

    if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_Close }))
    {
        Logger::Error("Failed to send close message to IcarianEngine");

        Terminate();

        return;
    }

    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    while (IsRunning())
    {
        const std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double> delta = end - start;
        if (delta.count() > 5.0)
        {
            Logger::Error("Failed to close IcarianEngine Instance");

            Terminate();

            return;
        }

        PollMessage(true);        
    }    

    m_cursorState = CursorState_Normal;
}
void ProcessManager::SetSize(uint32_t a_width, uint32_t a_height)
{
    if (m_width != a_width || m_height != a_height)
    {
        m_width = a_width;
        m_height = a_height;
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_width, (GLsizei)m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

        m_resize = true;
    }
}
void ProcessManager::PushCursorPos(const glm::vec2& a_cPos)
{
    if (m_captureInput && IsRunning())
    {
        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_CursorPos, sizeof(glm::vec2), (char*)&a_cPos}))
        {
            Logger::Error("Failed to send cursor position message to IcarianEngine");

            Terminate();

            return;
        }
    }
}
void ProcessManager::PushMouseState(uint8_t a_state)
{
    if (m_captureInput && IsRunning())
    {
        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_MouseState, sizeof(uint8_t), (char*)&a_state }))
        {
            Logger::Error("Failed to send mouse state message to IcarianEngine");

            Terminate();

            return;
        }
    }
}
void ProcessManager::PushKeyboardState(const IcarianCore::KeyboardState& a_state)
{
    if (m_captureInput && IsRunning())
    {
        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_KeyboardState, IcarianCore::KeyboardState::ElementCount, (char*)a_state.ToData() }))
        {
            Logger::Error("Failed to send keyboard state message to IcarianEngine");

            Terminate();

            return;
        }
    }
}

// MIT License
// 
// Copyright (c) 2025 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.