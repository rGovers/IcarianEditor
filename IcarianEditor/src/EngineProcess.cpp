// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "EngineProcess.h"

#ifndef WIN32
#include <csignal>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cassert>

#include "AssetLibrary.h"
#include "Core/Bitfield.h"
#include "Core/DMASwapBuffer.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"
#include "Core/IPCPipe.h"
#include "Core/SocketPipe.h"
#include "EditorConfig.h"
#include "Logger.h"
#include "SSHPipe.h"

#ifndef WIN32
// Not available in all versions of glibc yet so have to manually invoke syscalls
// Using wrapper as it takes in va args and want actual arguments
// NOTE: This should be replaced if it arrives in glibc
// UPDATE: This has been added to glibc but not wide spread in downstream yet so still going to use the wrapper until it is

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

uint32_t EngineProcess::IPCID = 0;

EngineProcess::EngineProcess(IcarianCore::CommunicationPipe* a_pipe, uint32_t a_width, uint32_t a_height)
{
    m_ipcPipe = a_pipe;

    m_dmaSwaps = 0;
    m_curFrame = 0;

    m_width = a_width;
    m_height = a_height;

    m_flags = 0;

    m_pipefileID = uint32_t(-1);

    m_fps = 0.0;
    m_frameTime = 0.0;
    m_frames = 0;

    m_ups = 0.0;
    m_updateTime = 0.0;
    m_updates = 0;

#ifdef WIN32
    m_processInfo.hProcess = INVALID_HANDLE_VALUE;
    m_processInfo.hThread = INVALID_HANDLE_VALUE;

    m_processHandle = INVALID_HANDLE_VALUE;
#else
    m_process = -1;
    m_processFD = -1;
#endif

    ISETBIT(m_flags, RemoteModeBit);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)a_width, (GLsizei)a_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_dmaTexture = GLuint(-1);
}

#ifdef WIN32
EngineProcess::EngineProcess(HANDLE a_procHandle, PROCESS_INFORMATION a_procInfo, IcarianCore::CommunicationPipe* a_pipe)
{
    m_ipcPipe = a_pipe;

    m_dmaSwaps = 0;
    m_curFrame = 0;

    m_width = InitialWidth;
    m_height = InitialHeight;

    m_flags = 0;

    m_pipefileID = uint32_t(-1);

    m_fps = 0.0;
    m_frameTime = 0.0;
    m_frames = 0;

    m_ups = 0.0;
    m_updateTime = 0.0;
    m_updates = 0;

    m_processHandle = a_procHandle;
    m_processInfo = a_procInfo;

    // Drivers where being inconsistant so need 2 textures one for normal textures and another for copying from DMA textures
    // OpenGL is doing the fun thing of it is great when only working with OpenGL but falls apart when you need to interact with something from the outside world
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)a_width, (GLsizei)a_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_dmaTexture = GLuint(-1);
}
#else
EngineProcess::EngineProcess(pid_t a_proc, int a_procFd, IcarianCore::CommunicationPipe* a_pipe, uint32_t a_pipefileID, uint32_t a_width, uint32_t a_height)
{
    m_ipcPipe = a_pipe;

    m_dmaSwaps = 0;
    m_curFrame = 0;

    m_width = a_width;
    m_height = a_height;

    m_flags = 0;

    m_pipefileID = a_pipefileID;

    m_fps = 0.0;
    m_frameTime = 0.0;
    m_frames = 0;

    m_ups = 0.0;
    m_updateTime = 0.0;
    m_updates = 0;

    m_process = a_proc;
    m_processFD = a_procFd;

    // So to the people wondering does RGBA == RGBA8 and the answer is yesn't 
    // RGBA is not very well defined and upto the implementation
    // Ask me how I found out

    // Drivers where being inconsistant so need 2 textures one for normal textures and another for copying from DMA textures
    // OpenGL is doing the fun thing of it is great when only working with OpenGL but falls apart when you need to interact with something from the outside world
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)a_width, (GLsizei)a_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
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
    // You are doing something that the driver normally handles for you yes even in Vulkan
    // YOU HAVE BEEN WARNED, If you do not have a basic understanding of GPU drivers good luck!~ 
    glGenTextures(1, &m_dmaTexture);
    glBindTexture(GL_TEXTURE_2D, m_dmaTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)a_width, (GLsizei)a_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
#endif

EngineProcess::~EngineProcess()
{
    IDEFER(
    {
        if (m_ipcPipe != nullptr)
        {
            delete m_ipcPipe;
        }
    });

    IDEFER(
    {
        if (m_pipefileID != uint32_t(-1))
        {
            AssetLibrary::DestroyAssetCommandBuffer(m_pipefileID);
        }
    });

#ifdef WIN32
    IDEFER(
    {
        if (m_processInfo.hProcess != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_processInfo.hProcess);
        }
        if (m_processInfo.hThread != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_processInfo.hThread);
        }
    });
#else
    IDEFER(
    {
        if (m_process > 0)
        {
            kill(m_process, SIGKILL);

            waitpid(m_process, NULL, 0);
        }
    });

    IDEFER(
    {
        if (m_processFD > 0)
        {
            close(m_processFD);
        }
    });
#endif

    IDEFER(FlushDMAImages());

    IDEFER(glDeleteTextures(1, &m_texture));
    IDEFER(
    {
        if (m_dmaTexture != GLuint(-1))
        {
            glDeleteTextures(1, &m_dmaTexture);
        }
    });

    if (m_ipcPipe != nullptr)
    {
        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_Close }))
        {
            return;
        }

        const float timeout = EditorConfig::GetEngineShutdownTimeout();
        const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

        while (IsAlive())
        {
            const std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            const std::chrono::duration<double> delta = end - start;

            if (delta.count() > timeout)
            {
                Logger::Error("Failed to close IcarianEngine Instance");

                return;
            }

            std::queue<IcarianCore::PipeMessage> msgs;
            Update(&msgs);

            while (!msgs.empty())
            {
                const IcarianCore::PipeMessage& msg = msgs.front();
                msgs.pop();

                if (msg.Data != nullptr)
                {
                    delete[] msg.Data;
                }
            }

            DMAUpdate();
        }
    }
}

bool EngineProcess::IsAlive() const
{
    if (!IISBITSET(m_flags, RemoteModeBit))
    {
#ifdef WIN32
        if (m_processInfo.hProcess == INVALID_HANDLE_VALUE || m_processInfo.hThread == INVALID_HANDLE_VALUE)
        {
            return false;
        }
#else
        if (m_process < 0)
        {
            return false;
        }
#endif
    }

    return IsPipeAlive();
}

bool EngineProcess::IsRemote() const
{
    return IISBITSET(m_flags, RemoteModeBit);
}

bool EngineProcess::IsDMAMode() const
{
    return IISBITSET(m_flags, DMAModeBit);
}

#ifndef WIN32
static std::filesystem::path GetAddr(const std::string_view& a_addr)
{
    const std::filesystem::path tmpAddr = std::filesystem::temp_directory_path();

    return tmpAddr / a_addr;
}
#endif

EngineProcess* EngineProcess::CreateProcess(const std::filesystem::path& a_workingDir, uint32_t a_width, uint32_t a_height, uint32_t a_threadCount)
{
    IERRBLOCK;

    const uint32_t pipefileID = AssetLibrary::CreateAssetCommandBuffer();
    IERRDEFER(AssetLibrary::DestroyAssetCommandBuffer(pipefileID));

    const uint32_t ipcID = IPCID++;

    const std::string ipcIDStr = std::to_string(ipcID);

    const std::string workingDirArg = "--wDir=" + a_workingDir.generic_string();
    const std::string pipefileArg = "--pipefile=" + std::to_string(pipefileID);
    const std::string ipcArg = "--ipc-id=" + ipcIDStr;
    const std::string threadArg = "--threads=" + std::to_string(a_threadCount);

    // TODO: Windows is completely broken dropping support for now but need to re-implement down the line
    // Got in that annoying position that Linux works, remote Windows execution worked, Windows on WINE works but Native Windows was fucked
    // Prefer drop a platform then poorly support it
    // Dropping support as spending 70% of time debugging WIN32 for minimal users with a 1 man dev team on the engineering side
#ifdef WIN32
    ICARIAN_ASSERT(0);
#else
    const std::filesystem::path pipeAddr = GetAddr(PipeName + ipcIDStr);
    const std::string pipeAddrStr = pipeAddr.generic_string();

    const IcarianCore::IPCPipe* serverPipe = IcarianCore::IPCPipe::Create(pipeAddrStr);
    IERRCHECKRET(serverPipe != nullptr, nullptr);
    IDEFER(delete serverPipe);

    // This is a bit odd leaving this here as a note
    // This create another copy of the process on Unix systems 
    // You can tell if you are the parent or child process based on the return result
    // If I am the child process I run the execute process which overwrites the current process with the new process
    // Unix systems are a bit odd but it works so I am not gonna question it
    const pid_t process = fork();

    if (process < 0)
    {
        // If we are in here likely very bad things are happening
        ITRIGGERERRRET(nullptr);
    }
    else if (process == 0)
    {
        // Starting the engine
        // In a weird state cause in a forked process so doing stuff C style
        // Once execution is started state is normal again
        if (execl("./IcarianNative", "--headless", workingDirArg.c_str(), pipefileArg.c_str(), ipcArg.c_str(), threadArg.c_str(), NULL) < 0)
        {
            printf("Failed to start process \n");
            perror("execl");
            assert(0);
        }
    }
    else
    {
        IERRDEFER(
        {
            kill(process, SIGKILL);

            waitpid(process, NULL, 0);
        });

        const float timeout = EditorConfig::GetEnginePipeTimeout();

        // State is correct cause in the parent process so can use inbuilt stuff
        IcarianCore::IPCPipe* ipcPipe = serverPipe->Accept(timeout);
        IERRCHECKRET(ipcPipe != nullptr, nullptr);
        IERRDEFER(delete ipcPipe);

        const glm::ivec2 data = glm::ivec2((int)a_width, (int)a_height);
        IERRCHECKRET(ipcPipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data }), nullptr);

        // Need to file descriptor for the process for later
        const int processFd = sys_pidfd_open(process, 0);
        IERRCHECKRET(processFd >= 0, nullptr);

        return new EngineProcess(process, processFd, ipcPipe, pipefileID, a_width, a_height);
    }
#endif

    return nullptr;
}
EngineProcess* EngineProcess::CreateRemoteProcess(SSHPipe* a_sshPipe, uint16_t a_clientPort, uint32_t a_width, uint32_t a_height)
{
    IERRBLOCK;

#ifdef WIN32
    return nullptr;
#else
    IERRCHECKRET(a_sshPipe != nullptr, nullptr);
    IERRCHECKRET(a_sshPipe->IsAlive(), nullptr);

    const std::string addr = a_sshPipe->GetAddr();

    const std::filesystem::path tempPath = a_sshPipe->GetTempDirectory();
    const std::filesystem::path path = tempPath / "IcarianRemote";

    const std::string pathStr = path.generic_string();

    const std::string cdCmd = "cd \"" + pathStr + "\"";

    IERRCHECKRET(a_sshPipe->Send(cdCmd.c_str()), nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const std::string args = "--remote-headless"
        " --remote-port=" + std::to_string(a_clientPort);

    const e_SSHHostOS hostOS = a_sshPipe->GetHostOS();
    switch (hostOS) 
    {
    case SSHHostOS_WindowsPowerCMD:
    case SSHHostOS_WindowsPowershell:
    {
        const std::string cmdStr = "IcarianNative.exe " + args;

        a_sshPipe->Send(cmdStr.c_str());

        break;
    }
    case SSHHostOS_Linux:
    {
        const std::filesystem::path exePath = path / "IcarianNative";

        const std::string pathStr = exePath.generic_string();

        const std::string cmdStr = pathStr + " " + args;

        a_sshPipe->Send(cmdStr.c_str());

        break;
    }
    default:
    {
        ITRIGGERERRRET(nullptr);
    }
    }

    IcarianCore::SocketPipe* ipcPipe = IcarianCore::SocketPipe::Connect(addr, a_clientPort);
    IERRCHECKRET(ipcPipe != nullptr, nullptr);
    IERRDEFER(delete ipcPipe);

    const glm::ivec2 data = glm::ivec2((int)a_width, (int)a_height);
    IERRCHECKRET(ipcPipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data }), nullptr);

    return new EngineProcess(ipcPipe, a_width, a_height);
#endif

    return nullptr;
}

bool EngineProcess::IsPipeAlive() const
{
    return m_ipcPipe != nullptr && m_ipcPipe->IsAlive();
}

GLuint EngineProcess::GetImage() const
{
    if (IISBITSET(m_flags, DMAModeBit))
    {
        return m_dmaTexture;
    }

    return m_texture;
}

void EngineProcess::SetSize(uint32_t a_width, uint32_t a_height)
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

bool EngineProcess::Update(std::queue<IcarianCore::PipeMessage>* a_msgs)
{
    ICARIAN_ASSERT(a_msgs != nullptr);

    if (m_ipcPipe == nullptr || !m_ipcPipe->IsAlive())
    {
        return false;
    }

    if (IISBITSET(m_flags, ResizeBit))
    {
        ICLEARBIT(m_flags, ResizeBit);

        const glm::ivec2 size = glm::ivec2((int)m_width, (int)m_height);

        if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&size}))
        {
            FlushDMAImages();

            Logger::Error("Failed to send resize message to IcarianEngine");

            return false;
        }
    }

    std::queue<IcarianCore::PipeMessage> msgs;
    if (!m_ipcPipe->Receive(&msgs))
    {
        return false;
    }

    while (!msgs.empty())
    {
        const IcarianCore::PipeMessage msg = msgs.front();
        msgs.pop();

        switch (msg.Type) 
        {
        case IcarianCore::PipeMessageType_Null:
        {
            while (!msgs.empty())
            {
                const IcarianCore::PipeMessage& msg = msgs.front();

                if (msg.Data != NULL)
                {
                    delete[] msg.Data;
                }

                msgs.pop();
            }

            return false;
        }
        case IcarianCore::PipeMessageType_PushFrame:
        {
            IDEFER(delete[] msg.Data);

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
            IDEFER(delete[] msg.Data);

            const double delta = *(double*)(msg.Data + 0);

            ++m_frames;

            m_frameTime -= delta;
            if (m_frameTime < -1.0)
            {
                // We are out of range so can assume the FPS is zero
                // Reset
                // This mostly occurs in the editor window as we do not update when hidden
                m_fps = 0.0f;
                m_frameTime = (1.0 / FPSUpdateRate);
                m_frames = 0;
            }
            else if (m_frameTime <= 0.0)
            {
                m_fps = m_frames * FPSUpdateRate;
                m_frameTime += (1.0 / FPSUpdateRate);
                m_frames = 0;
            }

            break;
        }
        case IcarianCore::PipeMessageType_PushDMASwapFDBuffer:
        {
            IDEFER(delete[] msg.Data);

#ifdef WIN32
            Logger::Error("DMA FD message on Windows");

            break;
#else
            ISETBIT(m_flags, DMAModeBit);

            const DMASwapBufferFD& swapBuffer = *(DMASwapBufferFD*)msg.Data;

            // I am a fucking idiot it is the process file descriptor it needs not the process id
            // We need to remap the file descriptors as they will not be valid in this process due to different descriptor tables
            // Sigh.... fuck OpenGL drivers have to pick between undefined behaviour and leaking the file descriptor
            // I love non spec compliant drivers
            // OpenGL spec states that import hands ownership of the fd to OpenGL and all operations after are undefined behaviour
            const int imageFD = sys_pidfd_getfd(m_processFD, swapBuffer.ImageFD, 0);
            const int startSemaphore = sys_pidfd_getfd(m_processFD, swapBuffer.StartSemaphore, 0);
            const int endSemaphore = sys_pidfd_getfd(m_processFD, swapBuffer.EndSemaphore, 0);

            const GLuint memoryObject = ILAMBDA(
            {
                GLuint val;
                glCreateMemoryObjectsEXT(1, &val);
                // Hmm weird do not know where the extra data is coming from for swapBuffer.Size but ehh as long as it works
                glImportMemoryFdEXT(val, (GLuint64)swapBuffer.Size + swapBuffer.Offset, GL_HANDLE_TYPE_OPAQUE_FD_EXT, imageFD);

                ILRETURN val;
            });

            const GLuint textureHandle = ILAMBDA(
            {
                GLuint val;
                glCreateTextures(GL_TEXTURE_2D, 1, &val);
                glTextureStorageMem2DEXT(val, 1, GL_RGBA8, (GLsizei)swapBuffer.Width, (GLsizei)swapBuffer.Height, memoryObject, swapBuffer.Offset);
                glTextureParameteri(val, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTextureParameteri(val, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTextureParameteri(val, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(val, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                ILRETURN val;
            });
            const GLuint startSemaphoreHandle = ILAMBDA(
            {
                GLuint val;
                glGenSemaphoresEXT(1, &val);
                glImportSemaphoreFdEXT(val, GL_HANDLE_TYPE_OPAQUE_FD_EXT, startSemaphore);

                ILRETURN val;
            });

            const DMASwapchainImage image = 
            {
                .MemoryObject = memoryObject,
                .Texture = textureHandle,
                .StartSemaphore = startSemaphoreHandle,
                .EndSemaphore = ILAMBDA(
                {
                    GLuint val;
                    glGenSemaphoresEXT(1, &val);
                    glImportSemaphoreFdEXT(val, GL_HANDLE_TYPE_OPAQUE_FD_EXT, endSemaphore);

                    ILRETURN val;
                }),
                .Width = swapBuffer.Width,
                .Height = swapBuffer.Height,
                .Offset = swapBuffer.Offset,
            };

            constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
            glSignalSemaphoreEXT(startSemaphoreHandle, 0, NULL, 1, &textureHandle, &Layout);

            m_dmaImages.emplace_back(image);
#endif

            break;
        }
        case IcarianCore::PipeMessageType_PushDMASwapHandleBuffer:
        {
            IDEFER(delete[] msg.Data);

#ifndef WIN32
            Logger::Error("DMA Handle message on non Windows OS");

            break;
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

            const GLuint memoryObject = ILAMBDA(
            {
                GLuint val;
                glCreateMemoryObjectsEXT(1, &val);
                glImportMemoryWin32HandleEXT(val, (GLuint64)swapBuffer.Size + swapBuffer.Offset, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, imageHandle);

                ILRETURN val;
            });

            const GLuint textureHandle = ILAMBDA(
            {
                GLuint val;
                glCreateTextures(GL_TEXTURE_2D, 1, &val);
                glTextureStorageMem2DEXT(val, 1, GL_RGBA8, (GLsizei)swapBuffer.Width, (GLsizei)swapBuffer.Height, image.MemoryObject, swapBuffer.Offset);
                glTextureParameteri(val, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(val, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(val, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(val, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                ILRETURN val;
            });

            const GLuint startSemaphoreHandle = ILAMBDA(
            {
                GLuint val;
                glGenSemaphoresEXT(1, &val);
                glImportSemaphoreWin32HandleEXT(val, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, startSemaphore);
            });

            const DMASwapchainImage image = 
            {
                .MemoryObject = memoryObject,
                .Texture = textureHandle,
                .StartSemaphore = startSemaphoreHandle,
                .EndSemaphore = ILAMBDA(
                {
                    GLuint val;
                    glGenSemaphoresEXT(1, &val);
                    glImportSemaphoreWin32HandleEXT(val, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, endSemaphore);

                    ILRETURN val;
                }),
                .Width = swapBuffer.Width,
                .Height = swapBuffer.Height,
                .Offset = swapBuffer.Offset,
            };

            constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
            glSignalSemaphoreEXT(startSemaphoreHandle, 0, NULL, 1, &textureHandle, &Layout);

            m_dmaImage.emplace_back(image);
#endif

            break;
        }
        case IcarianCore::PipeMessageType_FlushDMASwapFDBuffer:
        case IcarianCore::PipeMessageType_FlushDMASwapHandleBuffer:
        {
            IDEFER(delete[] msg.Data);

            FlushDMAImages();

            break;
        }
        case IcarianCore::PipeMessageType_DMASwap:
        {
            IDEFER(delete[] msg.Data);

            ++m_dmaSwaps;

            break;
        }
        case IcarianCore::PipeMessageType_UpdateData:
        {
            IDEFER(delete[] msg.Data);

            const double delta = *(double*)(msg.Data + 0);

            ++m_updates;

            m_updateTime -= delta;
            if (m_updateTime < -1.0)
            {
                // We are out of range so can assume the UPS is zero
                // Reset
                m_ups = 0;
                m_updateTime = (1.0 / UPSUpdateRate);
                m_updates = 0;
            }
            else if (m_updateTime <= 0.0)
            {
                m_ups = m_updates * UPSUpdateRate;
                m_updateTime += (1.0 / UPSUpdateRate);
                m_updates = 0;
            }

            break;
        }
        case IcarianCore::PipeMessageType_Close:
        {
#ifdef WIN32
            m_processInfo.hProcess = INVALID_HANDLE_VALUE;
            m_processInfo.hThread = INVALID_HANDLE_VALUE;

            m_processHandle = INVALID_HANDLE_VALUE;
#else
            // Process will be a zombie until it is waited upon or the parent dies
            waitpid(m_process, NULL, 0);

            m_process = -1;
#endif

            if (m_ipcPipe != nullptr)
            {
                delete m_ipcPipe;
                m_ipcPipe = nullptr;
            }

            break;
        }
        default:
        {
            a_msgs->push(msg);

            break;
        }
        }
    }

    return true;
}

void EngineProcess::DMAUpdate()
{
    if (!IISBITSET(m_flags, DMAModeBit))
    {
        return;
    }

    const uint32_t imageCount = (uint32_t)m_dmaImages.size();
    if (m_curFrame >= imageCount)
    {
        return;
    }

    while (m_dmaSwaps > 0)
    {
        IDEFER(--m_dmaSwaps);

        const DMASwapchainImage& img = m_dmaImages[m_curFrame];

        constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
        glWaitSemaphoreEXT(img.EndSemaphore, 0, NULL, 1, &img.Texture, &Layout);

        if (m_dmaSwaps == 1 && img.Width == m_width && img.Height == m_height)
        {
            glCopyImageSubData
            (
                img.Texture,
                GL_TEXTURE_2D,
                0,
                0, 0, 0,
                m_dmaTexture,
                GL_TEXTURE_2D,
                0,
                0, 0, 0,
                (GLsizei)img.Width, (GLsizei)img.Height, 1
            );
        }

        m_curFrame = (m_curFrame + 1) % imageCount;

        const DMASwapchainImage& nextImage = m_dmaImages[m_curFrame];
        glSignalSemaphoreEXT(nextImage.StartSemaphore, 0, NULL, 1, &nextImage.Texture, &Layout);
    }
}

void EngineProcess::SignalImage()
{
    if (!IISBITSET(m_flags, DMAModeBit))
    {
        return;
    }

    const uint32_t imageCount = (uint32_t)m_dmaImages.size();
    if (m_curFrame >= imageCount)
    {
        return;
    }

    const DMASwapchainImage& img = m_dmaImages[m_curFrame];

    constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
    glSignalSemaphoreEXT(img.StartSemaphore, 0, NULL, 1, &img.Texture, &Layout);
}
e_EngineFrameWaitStatus EngineProcess::WaitImage()
{
    if (!IISBITSET(m_flags, DMAModeBit))
    {
        return EngineFrameWaitStatus_InvalidMode;
    }

    const uint32_t imageCount = (uint32_t)m_dmaImages.size();
    if (m_curFrame >= imageCount)
    {
        return EngineFrameWaitStatus_Reset;
    }

    if (m_dmaSwaps <= 0)
    {
        return EngineFrameWaitStatus_Wait;
    }

    --m_dmaSwaps;
    const DMASwapchainImage& img = m_dmaImages[m_curFrame];

    constexpr GLenum Layout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
    glWaitSemaphoreEXT(img.EndSemaphore, 0, NULL, 1, &img.Texture, &Layout);

    if (img.Width == m_width && img.Height == m_height)
    {
        glCopyImageSubData
        (
            img.Texture,
            GL_TEXTURE_2D,
            0,
            0, 0, 0,
            m_dmaTexture,
            GL_TEXTURE_2D,
            0,
            0, 0, 0,
            (GLsizei)img.Width, (GLsizei)img.Height, 1
        );
    }

    m_curFrame = (m_curFrame + 1) % imageCount;

    return EngineFrameWaitStatus_Sucess;
}

void EngineProcess::FlushDMAImages()
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

void EngineProcess::PushCursorPos(const glm::vec2& a_cPos)
{
    if (!IsPipeAlive())
    {
        return;
    }

    if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_CursorPos, sizeof(glm::vec2), (char*)&a_cPos}))
    {
        Logger::Error("Failed to send cursor position message to IcarianEngine");

        return;
    }
}
void EngineProcess::PushMouseState(uint8_t a_state)
{
    if (!IsPipeAlive())
    {
        return;
    }

    if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_MouseState, sizeof(uint8_t), (char*)&a_state }))
    {
        Logger::Error("Failed to send mouse state message to IcarianEngine");

        return;
    }
}
void EngineProcess::PushKeyboardState(const IcarianCore::KeyboardState& a_state)
{
    if (!IsPipeAlive())
    {
        return;
    }

    if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_KeyboardState, IcarianCore::KeyboardState::ElementCount, (char*)a_state.ToData() }))
    {
        Logger::Error("Failed to send keyboard state message to IcarianEngine");

        return;
    }
}

void EngineProcess::CaptureFrame()
{
    if (!IsPipeAlive())
    {
        return;
    }

    if (!m_ipcPipe->Send({ IcarianCore::PipeMessageType_CaptureFrame }))
    {
        Logger::Error("Failed to send capture frame message to IcarianEngine");

        return;
    }
}

void EngineProcess::SendRuntimeMessage(const std::string_view& a_string, const void* a_data, uint32_t a_dataLength)
{
    if (!IsPipeAlive())
    {
        return;
    }

    const uint32_t strLen = (uint32_t)a_string.length();

    if (a_data == nullptr && a_dataLength <= 0)
    {
        const uint32_t bufferSize = strLen + 1;

        char* dat = (char*)malloc(strLen + 1);
        IDEFER(free(dat));

        memcpy(dat, a_string.data(), strLen);
        dat[strLen] = 0;

        m_ipcPipe->Send({ IcarianCore::PipeMessageType_RuntimeMessage, bufferSize, dat });

        return;
    }

    const uint32_t bufferSize = strLen + 1 + a_dataLength;

    char* dat = (char*)malloc(bufferSize);
    IDEFER(free(dat));

    memset(dat, 0, bufferSize);

    memcpy(dat, a_string.data(), strLen);
    memcpy(dat + strLen + 1, a_data, a_dataLength);

    m_ipcPipe->Send({ IcarianCore::PipeMessageType_RuntimeMessage, bufferSize, dat });
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