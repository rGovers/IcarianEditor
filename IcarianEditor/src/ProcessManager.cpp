#include "ProcessManager.h"
#include "Flare/IcarianDefer.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#ifndef WIN32
#include <csignal>
#include <unistd.h>
#endif

#include <cstdlib>
#include <filesystem>
#include <string>

#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "ProfilerData.h"

static std::filesystem::path GetAddr(const std::string_view& a_addr)
{
    return std::filesystem::temp_directory_path() / a_addr;
}

ProcessManager::ProcessManager()
{
    m_pipe = nullptr;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_width, (GLsizei)m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
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

#if WIN32    
    WSACleanup();
#endif
}

void ProcessManager::Terminate()
{
    if (m_pipe != nullptr)
    {
        delete m_pipe;
        m_pipe = nullptr;
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

bool ProcessManager::Start(const std::filesystem::path& a_workingDir)
{
    Logger::Message("Spawning IcarianEngine Instance");

    ProfilerData::Clear();

    const std::string workingDirArg = "--wDir=" + a_workingDir.string();
#if WIN32
    const FlareBase::IPCPipe* serverPipe = FlareBase::IPCPipe::Create(GetAddr(PipeName).string());
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
    }
    
    m_pipe = serverPipe->Accept();
    if (m_pipe == nullptr)
    {
        Logger::Error("Failed to connect to IcarianEngine");
        Terminate();

        return false;
    }

    const glm::ivec2 data = glm::ivec2((int)m_width, (int)m_height);

    if (!m_pipe->Send({ FlareBase::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data }))
    {
        Logger::Error("Failed to send resize message to IcarianEngine");
        Terminate();

        return false;
    }

    return true;
#else
    if (m_process == -1)
    {
        const FlareBase::IPCPipe* serverPipe = FlareBase::IPCPipe::Create(GetAddr(PipeName).string());
        if (serverPipe == nullptr)
        {
            Logger::Error("Failed to create IPC Pipe");

            return false;
        }
        // ICARIAN_DEFER_del(serverPipe);
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
            if (execl("./IcarianNative", "--headless", workingDirArg.c_str(), nullptr) < 0)
            {
                printf("Failed to start process");
                perror("execl");
                assert(0);
            }
        }
        else
        {            
            // State is correct cause in the parent process so can use inbuilt stuff
            m_pipe = serverPipe->Accept();
            if (m_pipe == nullptr)
            {
                Logger::Error("Failed to connect to IcarianEngine");
                kill(m_process, SIGTERM);   
                m_process = -1;

                return false;
            }

            const glm::ivec2 data = glm::ivec2((int)m_width, (int)m_height);

            if (!m_pipe->Send({ FlareBase::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data }))
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
    std::queue<FlareBase::PipeMessage> messages;

    if (!m_pipe->Receive(&messages))
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
        const FlareBase::PipeMessage msg = messages.front();
        // ICARIAN_DEFER(msg, if (msg.Data != nullptr) { delete[] msg.Data; });
        IDEFER(
        if (msg.Data != nullptr)
        {
            delete[] msg.Data;
        });
        messages.pop();

        switch (msg.Type)
        {
        case FlareBase::PipeMessageType_PushFrame:
        {
            if (msg.Length == m_width * m_height * 4)
            {
                glBindTexture(GL_TEXTURE_2D, m_tex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, msg.Data);
            }

            break;
        }
        case FlareBase::PipeMessageType_FrameData:
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
        case FlareBase::PipeMessageType_UpdateData:
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
        case FlareBase::PipeMessageType_Message:
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
        case FlareBase::PipeMessageType_ProfileScope:
        {
            ProfilerData::PushData(*(ProfileScope*)msg.Data);

            break;
        }
        case FlareBase::PipeMessageType_Close:
        {
#if WIN32
            m_processInfo.hProcess = INVALID_HANDLE_VALUE;
            m_processInfo.hThread = INVALID_HANDLE_VALUE;
#else
            m_process = -1;
#endif
            delete m_pipe;
            m_pipe = nullptr;

            return;
        }
        case FlareBase::PipeMessageType_Null:
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
#if WIN32
        m_processInfo.hProcess = INVALID_HANDLE_VALUE;
        m_processInfo.hThread = INVALID_HANDLE_VALUE;
#else
        m_process = -1;
#endif

        if (m_pipe != nullptr)
        {
            delete m_pipe;
            m_pipe = nullptr;
        }

        return;
    }

    PollMessage();

    if (m_pipe == nullptr)
    {
        return;
    }

    // Engine only pushes one frame at a time
    // Do it this way so the editor does not get overwhelmed with frame data
    // Cause extreme lag if I do not throttle the push frames ~1 fps
    // IPCs are only so fast
    if (!m_pipe->Send({ FlareBase::PipeMessageType_UnlockFrame }))
    {
        Logger::Error("Failed to send unlock frame message to IcarianEngine");

        Terminate();

        return; 
    }

    if (m_resize)
    {
        m_resize = false;

        const glm::ivec2 size = glm::ivec2((int)m_width, (int)m_height);

        if (!m_pipe->Send({ FlareBase::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&size}))
        {
            Logger::Error("Failed to send resize message to IcarianEngine");

            Terminate();

            return;   
        }
    }
}
void ProcessManager::Stop()
{
    Logger::Message("Stopping IcarianEngine Instance");
    
    if (!m_pipe->Send({ FlareBase::PipeMessageType_Close }))
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
}
void ProcessManager::SetSize(uint32_t a_width, uint32_t a_height)
{
    if (m_width != a_width || m_height != a_height)
    {
        m_width = a_width;
        m_height = a_height;

        m_resize = true;
    }
}
void ProcessManager::PushCursorPos(const glm::vec2& a_cPos)
{
    if (IsRunning())
    {
        if (!m_pipe->Send({ FlareBase::PipeMessageType_CursorPos, sizeof(glm::vec2), (char*)&a_cPos}))
        {
            Logger::Error("Failed to send cursor position message to IcarianEngine");

            Terminate();

            return;
        }
    }
}
void ProcessManager::PushMouseState(unsigned char a_state)
{
    if (IsRunning())
    {
        if (!m_pipe->Send({ FlareBase::PipeMessageType_MouseState, sizeof(unsigned char), (char*)&a_state }))
        {
            Logger::Error("Failed to send mouse state message to IcarianEngine");

            Terminate();

            return;
        }
    }
}
void ProcessManager::PushKeyboardState(FlareBase::KeyboardState& a_state)
{
    if (IsRunning())
    {
        if (!m_pipe->Send({ FlareBase::PipeMessageType_KeyboardState, FlareBase::KeyboardState::ElementCount, (char*)a_state.ToData() }))
        {
            Logger::Error("Failed to send keyboard state message to IcarianEngine");

            Terminate();

            return;
        }
    }
}