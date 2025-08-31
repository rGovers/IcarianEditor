// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "Application.h"

#include <string>
#include <vector>

class Modal;
class Project;
class RuntimeStorage;
class SSHPipe;
class Window;
class Workspace;

#include "EngineInputInteropStructures.h"

class AppMain : public Application
{
private:
    static constexpr uint32_t FocusedBit = 0;
    static constexpr uint32_t CaptureInputBit = 1;

    static constexpr uint32_t MoveBit = 0;
    static constexpr uint32_t TopResizeBit = 1;
    static constexpr uint32_t BottomResizeBit = 2;
    static constexpr uint32_t LeftResizeBit = 3;
    static constexpr uint32_t RightResizeBit = 4;

    static constexpr float ResizeThreshold = 2.5f;
    static constexpr float MenuBarSize = 32.5f;

    double               m_titleSet;

    std::string          m_fpsText;

    SSHPipe*             m_remotePipe;

    std::vector<Window*> m_windows;
    std::vector<Modal*>  m_modals;

    Project*             m_project;
    RuntimeStorage*      m_rStorage;
    Workspace*           m_workspace;

    glm::vec2            m_startWindowPos;
    glm::vec2            m_startWindowSize;
    glm::vec2            m_startMousePos;

    GLuint               m_vao;

    uint16_t             m_clientPort;
    e_CursorState        m_cursorState;

    uint8_t              m_windowActions;
    uint8_t              m_flags;

    std::vector<bool>    m_runtimeModalState;

protected:

public:
    AppMain();
    virtual ~AppMain();

    virtual void Update(double a_delta, double a_time);

    bool GetRuntimeModalState(uint32_t a_index);
    void SetRuntimeModalState(uint32_t a_index, bool a_state);

    bool ConnectRemote(const std::string_view& a_user, const std::string_view& a_addr, uint16_t a_port, uint16_t a_clientPort, bool a_compress);

    bool CapturesInput() const;

    inline void SetGameCursorState(e_CursorState a_state)
    {
        m_cursorState = a_state;
    }
    inline e_CursorState GetGameCursorState() const
    {
        return m_cursorState;
    }

    inline SSHPipe* GetSSHPipe() const
    {
        return m_remotePipe;
    }

    inline uint16_t GetClientPort() const
    {
        return m_clientPort;
    }

    void PushModal(Modal* a_modal);
    void DispatchRuntimeModal(const std::string_view& a_title, const glm::vec2& a_size, uint32_t a_index);
    void RemoveRuntimeModal(uint32_t a_index);
};

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
