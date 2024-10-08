// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <glad/glad.h>
#include <string_view>

#include "Core/InputBindings.h"
#include "Core/IPCPipe.h"
#include "Core/PipeMessage.h"
#include "Core/WindowsHeaders.h"

#include "EngineInputInteropStructures.h"

class ProcessManager
{
private:
    static constexpr std::string_view PipeName = "IcarianEngine-IPC";

#if WIN32
    PROCESS_INFORMATION   m_processInfo;

    void DestroyProc();
#else
    int                   m_process;
#endif      

    IcarianCore::IPCPipe* m_pipe;

    bool                  m_resize;

    uint32_t              m_width;
    uint32_t              m_height;
                        
    bool                  m_captureInput;
    e_CursorState         m_cursorState;

    int                   m_updates;
    double                m_updateTime;
    double                m_ups;

    int                   m_frames;                    
    double                m_frameTime;
    double                m_fps;
                        
    GLuint                m_tex;
    
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

    inline bool GetCaptureInput() const
    {
        return m_captureInput;
    }
    inline void SetCaptureInput(bool a_capture)
    {
        m_captureInput = a_capture;
    }
    inline e_CursorState GetCursorState() const
    {
        return m_cursorState;
    }

    void SetSize(uint32_t a_width, uint32_t a_height);
   
    void PushCursorPos(const glm::vec2& a_cPos);
    void PushMouseState(uint8_t a_state);
    void PushKeyboardState(const IcarianCore::KeyboardState& a_state);

    bool Start(const std::filesystem::path& a_workingDir);
    void Update();
    void Stop();
};

// MIT License
// 
// Copyright (c) 2024 River Govers
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