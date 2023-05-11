#include "ProcessManager.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#ifndef WIN32
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "Logger.h"
#include "ProfilerData.h"

static std::filesystem::path GetAddr(const std::string_view& a_addr)
{
    return std::filesystem::temp_directory_path() / a_addr;
}

ProcessManager::ProcessManager()
{
    const std::filesystem::path addrStr = GetAddr(PipeName);

#if WIN32
    m_pipeSock = INVALID_SOCKET;

    ZeroMemory(&m_processInfo, sizeof(m_processInfo));
    m_processInfo.hProcess = INVALID_HANDLE_VALUE;
    m_processInfo.hThread = INVALID_HANDLE_VALUE;
    
    // Failsafe for unsafe close
    DeleteFileA(addrStr.string().c_str());
    
    WSADATA wsaData = { };
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        Logger::Error("Failed to start WSA");
        assert(0);
    }
    
    m_serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_serverSock == INVALID_SOCKET)
    {
        Logger::Error("Failed creating IPC");
        perror("socket");
        assert(0);
    }
    
    sockaddr_un addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, addrStr.string().c_str());

    if (bind(m_serverSock, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        Logger::Error("Failed binding IPC");
        perror("bind");
        assert(0);
    }
#else
    m_pipeSock = -1;
    m_process = -1;   

    // Failsafe for unsafe close 
    // Frees the IPC from past instances
    unlink(addrStr.c_str());
    
    m_serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_serverSock < 0)
    {
        Logger::Error("Failed creating IPC");
        perror("socket");
        assert(0);
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, addrStr.c_str());

    if (bind(m_serverSock, (struct sockaddr*)&addr, SUN_LEN(&addr)) < 0)
    {
        Logger::Error("Failed binding IPC");
        perror("bind");
        assert(0);
    }
#endif
    
    if (listen(m_serverSock, 10) < 0)
    {
        Logger::Error("Failed setting IPC listen");
        perror("listen");
        assert(0);
    }

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
    closesocket(m_serverSock);

    const std::filesystem::path addrStr = GetAddr(PipeName);
    
    DeleteFileA(addrStr.string().c_str());
    
    WSACleanup();
#else
    close(m_serverSock);
#endif
}

#if WIN32
void ProcessManager::DestroyProc()
{
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
}
#endif

FlareBase::PipeMessage ProcessManager::ReceiveMessage()
{
    FlareBase::PipeMessage msg;

#if WIN32
    const int size = recv(m_pipeSock, (char*)&msg, FlareBase::PipeMessage::Size, 0);
    if (size == SOCKET_ERROR)
    {
        Logger::Error("Connection Error: " + std::to_string(WSAGetLastError()));
        m_pipeSock = INVALID_SOCKET;

        return FlareBase::PipeMessage();
    }
    if (size >= FlareBase::PipeMessage::Size)
    {
        msg.Data = new char[msg.Length];
        char* dataBuffer = msg.Data;
        uint32_t len = (uint32_t)(dataBuffer - msg.Data);
        while (len < msg.Length)
        {
            const int ret = recv(m_pipeSock, dataBuffer, (int)(msg.Length - len), 0);
            if (ret != SOCKET_ERROR)
            {
                dataBuffer += ret;

                len = (uint32_t)(dataBuffer - msg.Data);
            }
            else
            {
                m_pipeSock = INVALID_SOCKET;

                return FlareBase::PipeMessage();
            }
        }

        return msg;
    }
#else
    const uint32_t size = (uint32_t)read(m_pipeSock, &msg, FlareBase::PipeMessage::Size);
    if (size >= FlareBase::PipeMessage::Size)
    {
        msg.Data = new char[msg.Length];
        char* dataBuffer = msg.Data;
        uint32_t len = (uint32_t)(dataBuffer - msg.Data);
        while (len < msg.Length)
        {
            dataBuffer += read(m_pipeSock, dataBuffer, msg.Length - len);

            len = (uint32_t)(dataBuffer - msg.Data);
        }

        return msg;
    }
#endif
    
    return FlareBase::PipeMessage();
}
void ProcessManager::PushMessage(const FlareBase::PipeMessage& a_message) const
{
#if WIN32
    send(m_pipeSock, (const char*)&a_message, FlareBase::PipeMessage::Size, 0);
    if (a_message.Data != nullptr)
    {
        send(m_pipeSock, a_message.Data, (int)a_message.Length, 0);
    }
#else
    write(m_pipeSock, &a_message, FlareBase::PipeMessage::Size);
    if (a_message.Data != nullptr)
    {
        write(m_pipeSock, a_message.Data, a_message.Length);
    }
#endif
}

void ProcessManager::InitMessage() const
{
    Logger::Message("Connected to IcarianEngine");

    const glm::ivec2 data = glm::ivec2((int)m_width, (int)m_height);

    PushMessage({ FlareBase::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&data });
}

bool ProcessManager::Start(const std::filesystem::path& a_workingDir)
{
    Logger::Message("Spawning IcarianEngine Instance");

    ProfilerData::Clear();

    const std::string workingDirArg = "--wDir=" + a_workingDir.string();
#if WIN32
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
    
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(m_serverSock, &fdSet);
    if (select((int)(m_serverSock + 1), &fdSet, NULL, NULL, &tv) <= 0)
    {
        Logger::Error("Failed connecting to IcarianEngine");

        return false;
    }

    if (FD_ISSET(m_serverSock, &fdSet))
    {
        m_pipeSock = accept(m_serverSock, NULL, NULL);
        if ((int)m_pipeSock < 0)
        {
            Logger::Error("Failed connecting to IcarianEngine");

            return false;
        }

        InitMessage();

        return true;
    }
    
    Logger::Error("Failed to start IcarianEngine");

    DestroyProc();
#else
    if (m_process == -1)
    {
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
            if (execl("./IcarianNative", "--headless", workingDirArg.c_str(), nullptr) < 0)
            {
                printf("Failed to start process");
                perror("execl");
                assert(0);
            }
        }
        else
        {
            // Communicating with the engine
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET(m_serverSock, &fdSet);

            if (select(m_serverSock + 1, &fdSet, NULL, NULL, &tv) <= 0)
            {
                Logger::Error("Failed connecting to IcarianEngine");

                return false;
            }

            if (FD_ISSET(m_serverSock, &fdSet))
            {
                m_pipeSock = accept(m_serverSock, NULL, NULL);
                if (m_pipeSock < 0)
                {
                    Logger::Error("Failed connecting to IcarianEngine");

                    return false;
                }

                InitMessage();

                return true;
            }
            
            Logger::Error("Failed to start IcarianEngine");

            kill(m_process, SIGTERM);

            m_process = -1;
        }
    }
#endif

    return false;
}

void ProcessManager::PollMessage()
{
    const FlareBase::PipeMessage msg = ReceiveMessage();

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
            Logger::Message(str, false);

            break;
        }
        case LoggerMessageType_Warning:
        {
            Logger::Warning(str, false);

            break;
        }
        case LoggerMessageType_Error:
        {
            Logger::Error(str, false);

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
        break;
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

    if (msg.Data != nullptr)
    {
        delete[] msg.Data;
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

        return;
    }

#if WIN32
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 5;

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(m_pipeSock, &fdSet);
    if (m_pipeSock != INVALID_SOCKET && select((int)(m_pipeSock + 1), &fdSet, NULL, NULL, &tv) > 0)
    {
        PollMessage();
    }
#else
    struct pollfd fds;
    fds.fd = m_pipeSock;
    fds.events = POLLIN;

    while (poll(&fds, 1, 1) > 0)
    {
        if (fds.revents & (POLLNVAL | POLLERR | POLLHUP))
        {
            m_pipeSock = -1;

            return;
        }

        if (fds.revents & POLLIN)
        {
            PollMessage();
        }
    }
#endif

    // Engine only pushes one frame at a time
    // Do it this way so the editor does not get overwhelmed with frame data
    // Cause extreme lag if I do not throttle the push frames ~1 fps
    // IPCs are only so fast
    PushMessage({ FlareBase::PipeMessageType_UnlockFrame });

    if (m_resize)
    {
        m_resize = false;

        const glm::ivec2 size = glm::ivec2((int)m_width, (int)m_height);

        PushMessage({ FlareBase::PipeMessageType_Resize, sizeof(glm::ivec2), (char*)&size});
    }
}
void ProcessManager::Stop()
{
    Logger::Message("Stopping IcarianEngine Instance");
    
    PushMessage({ FlareBase::PipeMessageType_Close });

#if WIN32
    while (m_pipeSock != INVALID_SOCKET && m_processInfo.hProcess != INVALID_HANDLE_VALUE && m_processInfo.hThread != INVALID_HANDLE_VALUE)
    {
        PollMessage();
    }
    
    if (m_pipeSock != INVALID_SOCKET)
    {
        closesocket(m_pipeSock);
        m_pipeSock = INVALID_SOCKET;
    }
#else
    while (m_process != -1)
    {
        PollMessage();
    }

    close(m_pipeSock);
    m_pipeSock = -1;
#endif
    
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
void ProcessManager::PushCursorPos(const glm::vec2& a_cPos) const
{
    if (IsRunning())
    {
        PushMessage({ FlareBase::PipeMessageType_CursorPos, sizeof(glm::vec2), (char*)&a_cPos});
    }
}
void ProcessManager::PushMouseState(unsigned char a_state) const
{
    if (IsRunning())
    {
        PushMessage({ FlareBase::PipeMessageType_MouseState, sizeof(unsigned char), (char*)&a_state });
    }
}
void ProcessManager::PushKeyboardState(FlareBase::KeyboardState& a_state) const
{
    if (IsRunning())
    {
        PushMessage({ FlareBase::PipeMessageType_KeyboardState, FlareBase::KeyboardState::ElementCount, (char*)a_state.ToData() });
    }
}