// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#endif

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <glad/glad.h>
#include <string_view>

#include "Core/InputBindings.h"
#include "Core/CommunicationPipe.h"

#include "EngineInputInteropStructures.h"

class SSHPipe;

struct DMASwapchainImage
{
    GLuint MemoryObject;
    GLuint Texture;
    GLuint StartSemaphore;
    GLuint EndSemaphore;
    uint32_t Width;
    uint32_t Height;
    uint64_t Offset;
};

class ProcessManager
{
    private:
    static constexpr char PipeName[] = "IcarianEngine-IPC";
    
    static constexpr uint32_t RemoteModeBit = 0;
    static constexpr uint32_t ResizeBit = 1;
    static constexpr uint32_t DMAModeBit = 2;
    static constexpr uint32_t CaptureInputBit = 3;

#ifdef WIN32
    PROCESS_INFORMATION             m_processInfo;
    HANDLE                          m_processHandle;

    void DestroyProc();
#else
    pid_t                           m_process;
    int                             m_processFD;
#endif   

    uint32_t                        m_curFrame;
    uint32_t                        m_dmaSwaps;
    std::vector<DMASwapchainImage>  m_dmaImages;

    SSHPipe*                        m_remotePipe;
    IcarianCore::CommunicationPipe* m_ipcPipe;

    double                          m_updateTime;
    double                          m_ups;

    double                          m_frameTime;
    double                          m_fps;

    uint32_t                        m_width;
    uint32_t                        m_height;

    int                             m_updates;
    int                             m_frames;                    

    GLuint                          m_texture;
    GLuint                          m_dmaTexture;

    e_CursorState                   m_cursorState;
    uint16_t                        m_clientPort;
    
    uint8_t                         m_flags;

    void PollMessage(bool a_blockError = false);

    void FlushDMAImages();
    void Terminate();

protected:

public:
    ProcessManager();
    ~ProcessManager();

    bool IsRunning() const;

    // This makes me uncomfortable handing over a SSHPipe
    inline SSHPipe* GetRemotePipe() const
    {
#ifdef WIN32
        return nullptr;
#else
        return m_remotePipe;
#endif
    }

    bool IsRemoteConnected() const;
    inline bool IsRemoteRunning() const
    {
        return IISBITSET(m_flags, RemoteModeBit) && IsRunning();
    }

    GLuint GetImage() const;

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
        return IISBITSET(m_flags, CaptureInputBit);
    }
    inline void SetCaptureInput(bool a_capture)
    {
        ITOGGLEBIT(a_capture, m_flags, CaptureInputBit);
    }
    inline e_CursorState GetCursorState() const
    {
        return m_cursorState;
    }

    void SetSize(uint32_t a_width, uint32_t a_height);
   
    void CaptureFrame();

    void PushCursorPos(const glm::vec2& a_cPos);
    void PushMouseState(uint8_t a_state);
    void PushKeyboardState(const IcarianCore::KeyboardState& a_state);

    bool ConnectRemotePassword(const std::string_view& a_user, const std::string_view& a_addr, uint16_t a_port, uint16_t a_clientPort, bool a_compress);

    bool Start(const std::filesystem::path& a_workingDir);
    bool StartRemote();
    
    void Update();

    void Stop();
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