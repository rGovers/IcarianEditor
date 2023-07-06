#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Flare/InputBindings.h"
#include "Flare/IPCPipe.h"
#include "Flare/PipeMessage.h"
#include "Flare/WindowsHeaders.h"

#include <cstdint>
#include <filesystem>
#include <glad/glad.h>
#include <string_view>

class ProcessManager
{
private:
    static constexpr std::string_view PipeName = "IcarianEngine-IPC";

#if WIN32
    PROCESS_INFORMATION m_processInfo;

    void DestroyProc();
#else
    int                 m_process;
#endif

    FlareBase::IPCPipe* m_pipe;

    bool                m_resize;
                        
    uint32_t            m_width;
    uint32_t            m_height;
                        
    int                 m_updates;
    double              m_updateTime;
    double              m_ups;

    int                 m_frames;                    
    double              m_frameTime;
    double              m_fps;
                        
    GLuint              m_tex;
    
    void PollMessage(bool a_blockError = false);

    void Terminate();

protected:

public:
    ProcessManager();
    ~ProcessManager();

    inline bool IsRunning() const
    {
#if WIN32
        return m_processInfo.hProcess != INVALID_HANDLE_VALUE && m_processInfo.hThread != INVALID_HANDLE_VALUE && m_pipe != nullptr;
#else
        return m_process > 0 && m_pipe != nullptr;
#endif
    }

    inline GLuint GetImage() const
    {
        return m_tex;
    }

    inline uint32_t GetWidth()
    {
        return m_width;
    }
    inline uint32_t GetHeight()
    {
        return m_height;
    }

    inline double GetFPS() const
    {
        return m_fps;
    }
    inline double GetUPS() const
    {
        return m_ups;
    }

    void SetSize(uint32_t a_width, uint32_t a_height);
   
    void PushCursorPos(const glm::vec2& a_cPos);
    void PushMouseState(unsigned char a_state);
    void PushKeyboardState(FlareBase::KeyboardState& a_state);

    bool Start(const std::filesystem::path& a_workingDir);
    void Update();
    void Stop();
};