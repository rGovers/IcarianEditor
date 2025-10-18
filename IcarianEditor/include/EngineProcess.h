// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#endif

#include <cstdint>
#include <filesystem>
#include <glad/glad.h>
#include <queue>

#include "Core/CommunicationPipe.h"
#include "Core/InputBindings.h"

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

enum e_EngineFrameWaitStatus
{
    EngineFrameWaitStatus_Sucess,
    EngineFrameWaitStatus_Wait,
    EngineFrameWaitStatus_InvalidMode,
    EngineFrameWaitStatus_Reset,
};

class EngineProcess
{
private:
    static constexpr char PipeName[] = "IcarianEditor-IPC";

    static constexpr uint32_t DMAModeBit = 0;
    static constexpr uint32_t RemoteModeBit = 1;
    static constexpr uint32_t ResizeBit = 2;

    static constexpr float UPSUpdateRate = 4.0f;
    static constexpr float FPSUpdateRate = 4.0f;

    static uint32_t IPCID;

#ifdef WIN32
    PROCESS_INFORMATION             m_processInfo;
    HANDLE                          m_processHandle;

    void DestroyProc();
#else
    pid_t                           m_process;
    int                             m_processFD;
#endif

    IcarianCore::CommunicationPipe* m_ipcPipe;

    std::vector<DMASwapchainImage>  m_dmaImages;

    double                          m_updateTime;
    double                          m_frameTime;

    float                           m_ups;
    float                           m_fps;

    uint32_t                        m_curFrame;
    uint32_t                        m_dmaSwaps;

    uint32_t                        m_width;
    uint32_t                        m_height;

    GLuint                          m_texture;
    GLuint                          m_dmaTexture;

    uint32_t                        m_pipefileID;

    uint16_t                        m_updates;
    uint16_t                        m_frames;

    uint8_t                         m_flags;

    EngineProcess(IcarianCore::CommunicationPipe* a_pipe, uint32_t a_width, uint32_t a_height);

#ifdef WIN32
    EngineProcess(HANDLE a_procHandle, PROCESS_INFORMATION a_procInfo, IcarianCore::CommunicationPipe* a_pipe);
#else
    EngineProcess(pid_t a_proc, int a_procFd, IcarianCore::CommunicationPipe* a_pipe, uint32_t a_pipefileID, uint32_t a_width, uint32_t a_height);
#endif

    void FlushDMAImages();

protected:

public:
    ~EngineProcess();

    inline float GetFPS() const
    {
        return m_fps;
    }
    inline float GetUPS() const
    {
        return m_ups;
    }

    bool IsDMAMode() const;

    bool IsAlive() const;
    bool IsPipeAlive() const;
    bool IsRemote() const;

    GLuint GetImage() const;

    void SetSize(uint32_t a_width, uint32_t a_height);

    void PushCursorPos(const glm::vec2& a_cPos);
    void PushMouseState(uint8_t a_state);
    void PushKeyboardState(const IcarianCore::KeyboardState& a_state);

    void CaptureFrame();

    void SendRuntimeMessage(const std::string_view& a_string, const void* a_data, uint32_t a_dataLength);

    static EngineProcess* CreateProcess(const std::filesystem::path& a_workingDir, uint32_t a_width, uint32_t a_height, uint32_t a_threadCount);
    static EngineProcess* CreateRemoteProcess(SSHPipe* a_sshPipe, uint16_t a_clientPort, uint32_t a_width, uint32_t a_height);

    bool Update(std::queue<IcarianCore::PipeMessage>* a_msgs);
    void DMAUpdate();

    void SignalImage();
    e_EngineFrameWaitStatus WaitImage();
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