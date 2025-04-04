// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "ProcessManager.h"

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
#include <thread>

#include "Core/DMASwapBuffer.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/IPCPipe.h"
#include "Core/SocketPipe.h"
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

    m_flags = 0;

    m_width = 1280;
    m_height = 720;

    // Drivers where being inconsistant so need 2 textures one for normal textures and another for copying from DMA textures
    // OpenGL is doing the fun thing of it is great when only working with OpenGL but falls apart when you need to interact with something from the outside world
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_width, (GLsizei)m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Hic sunt dracones
    // Be very careful when doing stuff with this texture in this class OpenGL can do fun stuff and cause race conditions in the driver that cannot be controlled 
    // as OpenGL does not allow sync without bring the driver to a grinding halt
    // Keep in mind the data this is being populated with is from Vulkan so take appropriate measures
    // Once the data has been copied it is safe to use externally
    // You are doing something that the driver normal handles for you yes even in Vulkan
    // YOU HAVE BEEN WARNED, If you do not have a basic understanding of GPU drivers good luck!~ 
    glGenTextures(1, &m_dmaTexture);
    glBindTexture(GL_TEXTURE_2D, m_dmaTexture);
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
    glDeleteTextures(1, &m_texture);
    glDeleteTextures(1, &m_dmaTexture);

    if (m_remotePipe != nullptr)
    {
        delete m_remotePipe;
    }
}

bool ProcessManager::IsRunning() const
{
    if (IISBITSET(m_flags, RemoteModeBit))
    {
        return m_ipcPipe != nullptr && m_ipcPipe->IsAlive();
    }

#ifdef WIN32
    return m_processInfo.hProcess != INVALID_HANDLE_VALUE && m_processInfo.hThread != INVALID_HANDLE_VALUE && m_ipcPipe != nullptr;
#else
    return m_process > 0 && m_ipcPipe != nullptr && m_ipcPipe->IsAlive();
#endif
}

bool ProcessManager::IsRemoteConnected() const
{
    return m_remotePipe != nullptr && m_remotePipe->IsAlive();
}

GLuint ProcessManager::GetImage() const
{
    if (IISBITSET(m_flags, DMAModeBit))
    {
        return m_dmaTexture;
    }

    return m_texture;
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

#ifdef WIN32
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

bool ProcessManager::ConnectRemotePassword(const std::string_view& a_user, const std::string_view& a_addr, uint16_t a_port, uint16_t a_clientPort, bool a_compress)
{
#ifndef WIN32
    if (m_remotePipe != nullptr)
    {
        return false;
    }

    m_clientPort = a_clientPort;
    m_remotePipe = SSHPipe::ConnectPassword(a_user, a_addr, a_port, a_compress);

    return m_remotePipe != nullptr;
#endif

    return false;
}

bool ProcessManager::Start(const std::filesystem::path& a_workingDir)
{
    if (m_ipcPipe != nullptr)
    {
        return false;
    }

    Logger::Message("Spawning IcarianEngine Instance");

    ICLEARBIT(m_flags, DMAModeBit);
    m_dmaSwaps = 0;
    m_curFrame = 0;

    ProfilerData::Clear();

    ISETBIT(m_flags, CaptureInputBit);
    m_cursorState = CursorState_Normal;

    const std::string workingDirArg = "--wDir=" + a_workingDir.string();
#ifdef WIN32
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

    // Been having issues with IPC on Windows so using a socket on localhost instead
    // Not the best but it works
    m_ipcPipe = IcarianCore::SocketPipe::Connect("127.0.0.1", 9001);
    if (m_ipcPipe == nullptr)
    {
        Logger::Error("Failed to connect to IcarianEngine");
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

    ICLEARBIT(m_flags, RemoteModeBit);

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

            ICLEARBIT(m_flags, RemoteModeBit);

            return true;
        }
    }
#endif

    return false;
}
bool ProcessManager::StartRemote()
{
#ifdef WIN32
    // Disable for now and keep as Linux only until I figure out details
    return false;
#endif

    if (m_ipcPipe != nullptr)
    {
        return false;
    }

    if (m_remotePipe == nullptr || !m_remotePipe->IsAlive())
    {
        return false;
    }

    Logger::Message("Spawning Remote IcarianEngine Instance");

#ifdef WIN32
    m_processInfo.hProcess = INVALID_HANDLE_VALUE;
    m_processInfo.hThread = INVALID_HANDLE_VALUE;

    m_processHandle = INVALID_HANDLE_VALUE;
#else    
    m_process = -1;
#endif

    ICLEARBIT(m_flags, DMAModeBit);
    m_dmaSwaps = 0;
    m_curFrame = 0;

    ProfilerData::Clear();

    ISETBIT(m_flags, CaptureInputBit);
    m_cursorState = CursorState_Normal;

    const std::string addr = m_remotePipe->GetAddr();

    const std::filesystem::path tempPath = m_remotePipe->GetTempDirectory();
    const std::filesystem::path path = tempPath / "IcarianRemote";

    const std::string pathStr = path.generic_string();
    const std::string cmd = "cd \"" + pathStr + "\"";

    m_remotePipe->Send(cmd.c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const std::string args = "--remote-headless"
        " --remote-port=" + std::to_string(m_clientPort);

    switch (m_remotePipe->GetHostOS()) 
    {
    case SSHHostOS_WindowsPowerCMD:
    case SSHHostOS_WindowsPowershell:
    {
        const std::string cmdStr = "IcarianNative.exe " + args;

        m_remotePipe->Send(cmdStr.c_str());

        break;
    }
    case SSHHostOS_Linux:
    {
        const std::filesystem::path exePath = path / "IcarianNative";

        const std::string pathStr = exePath.generic_string();

        const std::string cmdStr = pathStr + " " + args;

        m_remotePipe->Send(cmdStr.c_str());

        break;
    }
    default:
    {
        ICARIAN_ASSERT(0);

        break;
    }
    }

    m_ipcPipe = IcarianCore::SocketPipe::Connect(addr, m_clientPort);
    if (m_ipcPipe == nullptr)
    {
        Logger::Error("Failed to create connection to IcarianEngine");

        return false;
    }

    const glm::ivec2 data = glm::ivec2((int)m_width, (int)m_height);
    if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data }))
    {
        Logger::Error("Failed to send resize message to IcarianEngine");

        Terminate();

        return false;
    }

    ISETBIT(m_flags, RemoteModeBit);

    return true;
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
            ICLEARBIT(m_flags, DMAModeBit);

            if (msg.Length == m_width * m_height * 4)
            {
                glBindTexture(GL_TEXTURE_2D, m_texture);
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
            ISETBIT(m_flags, DMAModeBit);

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
            ISETBIT(m_flags, DMAModeBit);

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

    if (IISBITSET(m_flags, ResizeBit))
    {
        ICLEARBIT(m_flags, ResizeBit);

        const glm::ivec2 size = glm::ivec2((int)m_width, (int)m_height);

        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&size}))
        {
            FlushDMAImages();

            Logger::Error("Failed to send resize message to IcarianEngine");

            Terminate();

            return;   
        }
    }

    if (IISBITSET(m_flags, DMAModeBit))
    {
        while (m_dmaSwaps > 0)
        {
            IDEFER(--m_dmaSwaps);

            const DMASwapchainImage& img = m_dmaImages[m_curFrame];

            constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
            glWaitSemaphoreEXT(img.EndSemaphore, 0, NULL, 1, &img.Texture, &Layout);

            if (img.Width == m_width && img.Height == m_height)
            {
                glCopyImageSubData(img.Texture, GL_TEXTURE_2D, 0, 0, 0, 0, m_dmaTexture, GL_TEXTURE_2D, 0, 0, 0, 0, img.Width, img.Height, 1);
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

        glBindTexture(GL_TEXTURE_2D, m_dmaTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_width, (GLsizei)m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

        ISETBIT(m_flags, ResizeBit);
    }
}

void ProcessManager::CaptureFrame()
{
    if (IsRunning())
    {
        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_CaptureFrame }))
        {
            Logger::Error("Failed to send capture frame message to IcarianEngine");

            Terminate();

            return;
        }
    }
}

void ProcessManager::PushCursorPos(const glm::vec2& a_cPos)
{
    if (IISBITSET(m_flags, CaptureInputBit) && IsRunning())
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
    if (IISBITSET(m_flags, CaptureInputBit) && IsRunning())
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
    if (IISBITSET(m_flags, CaptureInputBit) && IsRunning())
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