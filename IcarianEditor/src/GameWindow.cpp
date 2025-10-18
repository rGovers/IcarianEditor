// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Windows/GameWindow.h"

#include <imgui.h>
#include <thread>

#include "AppMain.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "Core/InputBindings.h"
#include "EngineProcess.h"
#include "FlareImGui.h"
#include "LoadingTasks/GenerateConfigLoadingTask.h"
#include "LoadingTasks/RemoteBuildLoadingTask.h"
#include "LoadingTasks/RunRemoteLoadingTask.h"
#include "LoadingTasks/SerializeAssetsLoadingTask.h"
#include "LoadingTasks/SyncRemoteBuildLoadingTask.h"
#include "Logger.h"
#include "Modals/ErrorModal.h"
#include "Modals/LoadingModal.h"
#include "Modals/SSHConnectModal.h"
#include "ProfilerData.h"
#include "Project.h"
#include "Runtime/RuntimeManager.h"

GameWindow::GameWindow(AppMain* a_app, Project* a_project) : Window
(
    "Game", 
    "Textures/WindowIcons/WindowIcon_Game.png", 
    true
)
{
    m_app = a_app;

    m_project = a_project;

    m_process = nullptr;

    m_width = 0;
    m_height = 0;

    m_flags = 0;
}
GameWindow::~GameWindow()
{
    if (m_process != nullptr)
    {
        delete m_process;
    }
}

void GameWindow::StartRemote(SSHPipe* a_sshPipe, uint16_t a_clientPort)
{
    m_process = EngineProcess::CreateRemoteProcess(a_sshPipe, a_clientPort, m_width, m_height);
}

void GameWindow::Update(double a_delta)
{
    if (m_process != nullptr && !m_process->IsAlive())
    {
        delete m_process;
        m_process = nullptr;

        ICLEARBIT(m_flags, CloseBit);
    }

    if (IISBITSET(m_flags, CloseBit))
    {
        if (m_process != nullptr)
        {
            delete m_process;
            m_process = nullptr;
        }

        ICLEARBIT(m_flags, CloseBit);
    }

    if (IISBITSET(m_flags, ProfilerSessionBit) && m_process == nullptr)
    {
        if (IISBITSET(m_flags, ProfilerSessionBit))
        {
            ProfilerData::EndSession();

            ICLEARBIT(m_flags, ProfilerSessionBit);
        }
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (ImGui::BeginMenuBar())
    {
        IDEFER(ImGui::EndMenuBar());

        if (m_process != nullptr)
        {
            const float width = ImGui::GetWindowWidth();

            const uint32_t fps = (uint32_t)m_process->GetFPS();
            const uint32_t ups = (uint32_t)m_process->GetUPS();

            const std::string fpsText = "FPS: " + std::to_string(fps);
            const std::string upsText = "UPS: " + std::to_string(ups);

            const ImVec2 fpsSize = ImGui::CalcTextSize(fpsText.c_str());
            const ImVec2 upsSize = ImGui::CalcTextSize(upsText.c_str());

            const float fpsOffset = fpsSize.x + 20.0f;
            const float upsOffset = fpsOffset + upsSize.x + 20.0f;

            ImGui::SetCursorPosX(width - fpsOffset);
            ImGui::Text("%s", fpsText.c_str());

            ImGui::SetCursorPosX(width - upsOffset);
            ImGui::Text("%s", upsText.c_str());
        }
    }

    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 vMinIm = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMaxIm = ImGui::GetWindowContentRegionMax();
    const ImVec2 sizeIm = { vMaxIm.x - vMinIm.x, vMaxIm.y - vMinIm.y };

    const bool locked = m_app->GetGameCursorState() == CursorState_Locked;
    const bool focused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

    m_width = (uint32_t)sizeIm.x;
    m_height = (uint32_t)sizeIm.y;

    SSHPipe* sshPipe = m_app->GetSSHPipe();

    const bool isRunning = m_process != nullptr && m_process->IsAlive();
    if (isRunning)
    {
        m_process->SetSize(m_width, m_height);

        const bool captureInput = m_app->CapturesInput();

        if (captureInput && (focused || locked))
        {
            if (locked)
            {
                m_app->SetGameCursorState(CursorState_Locked);

                const glm::vec2 cursorPos = m_app->GetCursorPos();

                const glm::vec2 delta = cursorPos - m_lastCursorPos;
                m_process->PushCursorPos(delta);

                m_lastCursorPos = cursorPos;
            }
            else
            {
                m_app->SetGameCursorState(CursorState_Normal);

                const ImVec2 mousePosIm = ImGui::GetMousePos();

                const glm::vec2 cPos = glm::vec2(mousePosIm.x - (winPos.x + vMinIm.x), mousePosIm.y - (winPos.y + vMinIm.y));

                m_process->PushCursorPos(cPos);
            }

            uint8_t mouseState = 0;
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                mouseState |= 0b1 << MouseButton_Left;
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                mouseState |= 0b1 << MouseButton_Middle;
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
                mouseState |= 0b1 << MouseButton_Right;
            }

            m_process->PushMouseState(mouseState);

            IcarianCore::KeyboardState state;
            for (uint32_t i = 0; i < KeyCode_Last; ++i)
            {
                const ImGuiKey key = FlareImGui::ImGuiKeyTable[i];
                if (key != ImGuiKey_None)
                {
                    if (ImGui::IsKeyDown(key))
                    {
                        state.SetKey((e_KeyCode)i, true);
                    }
                }
            }

            m_process->PushKeyboardState(state);
        }

        std::queue<IcarianCore::PipeMessage> messages;
        m_process->Update(&messages);

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
            case IcarianCore::PipeMessageType_SetCursorState:
            {
                // Not sure the best way to handle this as there can be multiple windows but the cursor can only be in 1 state at a time in the desktop environment
                // Currently I just use the last set cursor state but not sure the best way to handle it need to think for a bit
                m_app->SetGameCursorState(*(e_CursorState*)msg.Data);

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
                if (IISBITSET(m_flags, ProfilerSessionBit))
                {
                    ProfilerData::PushData(*(ProfileScope*)msg.Data);
                }

                break;
            }
            case IcarianCore::PipeMessageType_RuntimeMessage:
            {
                // Ignore for now as the engine should not need to send runtime messages back and forth to the editor in the game window at this current stage

                break;
            }
            default:
            {
                Logger::Error("Editor: Invalid Pipe Message: " + std::to_string(msg.Type) + " " + std::to_string(msg.Length));

                break;
            }
            }
        }

        if (m_process->IsAlive())
        {
            m_process->DMAUpdate();
        }

        const GLuint imageHandle = m_process->GetImage();
        ImGui::Image((ImTextureID)(uintptr_t)imageHandle, sizeIm);
    }

    const bool isRemote = m_app->GetSSHPipe() != nullptr;
    const bool isRemoteRunning = isRunning && isRemote && m_process->IsRemote();

    const uint32_t buttonCount = ILAMBDA(
    {
        if (isRunning)
        {
            if (isRemoteRunning)
            {
                ILRETURN 1;
            }

            ILRETURN 2;
        }

#ifdef WIN32
        ILRETURN 1;
#else
        ILRETURN 2;
#endif
    });

    const ImVec2 halfSize = ImVec2(sizeIm.x * 0.5f, sizeIm.y * 0.5f);

    const glm::vec2 winSize = glm::vec2(buttonCount * 45.0f, 40.0f);
    const glm::vec2 winHalfSize = winSize * 0.5f;

    const ImVec2 rectMin = ImVec2(winPos.x + halfSize.x - winHalfSize.x, winPos.y + TrayOffset);
    const ImVec2 rectMax = ImVec2(winPos.x + halfSize.x + winHalfSize.x, winPos.y + TrayOffset + winSize.y);

    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(30, 30, 30, 150), 2.0f);

    ImGui::SetCursorPos(ImVec2(halfSize.x - winHalfSize.x + 5.0f, TrayOffset + 5.0f));

    if (isRunning)
    {
        const char* stopTexture = ILAMBDA(
        {
            if (isRemoteRunning)
            {
                ILRETURN "Textures/Icons/Controls_StopRemote.png";
            }

            ILRETURN "Textures/Icons/Controls_Stop.png";
        });

        if (FlareImGui::ImageButton("Stop Game", stopTexture, glm::vec2(25.0f), false))
        {
            // Annoying with the way ImGui works as call order matters and prefer not create a buffer texture as that seems like a waste of VRAM
            // Defer closing the application to the next frame
            ISETBIT(m_flags, CloseBit);
        }

        if (!isRemoteRunning)
        {
            ImGui::SameLine();

            if (FlareImGui::ImageButton("Capture Frame", "Textures/Icons/Controls_Screencapture.png", glm::vec2(25.0f), false))
            {
                m_process->CaptureFrame();
            }
        }
    }
    else
    {
        if (FlareImGui::ImageButton("Run Game", "Textures/Icons/Controls_Play.png", glm::vec2(25.0f), false))
        {
            if (!RuntimeManager::IsBuilt())
            {
                m_app->PushModal(new ErrorModal("Cannot start with build errors"));

                return;
            }

            ICLEARBIT(m_flags, ProfilerSessionBit);
            if (ProfilerData::StartSession())
            {
                ISETBIT(m_flags, ProfilerSessionBit);
            }

            const std::filesystem::path cachePath = m_project->GetCachePath();

            m_app->SetGameCursorState(CursorState_Normal);

            // We are in a multi process enviroment so use less CPU threads because we end up overallocating otherwise and tank performance due to context switching
            // Yes it is counter intuitive that we give less threads to improve performance but a context switch is very expensive and pulls us under 30 FPS even on a very strong CPU
            // Engine defaults to 1/2 of the process so use a 1/4 when in the editor environment
            // And we never allocate everything as the system still needs threads for background things in modern systems
            // Rule of thumb performance goes up sharply to the number of physical cores 
            // Up slightly to the number of threads on a CPU if there is points it can do a CPU context switch over an OS context switch
            // Then starts going down after you exceed the number of threads as OS context switches are needed
            // I am ignoring big-little based CPUs as they complicate things and just a rule of thumb
            const uint32_t threadCount = (uint32_t)std::thread::hardware_concurrency() / 4;
            m_process = EngineProcess::CreateProcess(cachePath, m_width, m_height, threadCount);
        }

#ifndef WIN32
        ImGui::SameLine();

        if (isRemote)
        {
            if (FlareImGui::ImageButton("Remote Run Game", "Textures/Icons/Controls_PlayRemote.png", glm::vec2(25.0f), false))
            {
                const std::filesystem::path cachePath = m_project->GetCachePath();
                const std::filesystem::path remotePath = cachePath / "RemoteCore";

                const std::string name = m_project->GetName();

                const e_SSHHostOS hostOS = sshPipe->GetHostOS();
                const e_SSHHostArchitecture hostArch = sshPipe->GetHostArchitecture();

                const uint16_t clientPort = m_app->GetClientPort();

                LoadingTask* tasks[] = 
                {
                    new RemoteBuildLoadingTask(hostOS, hostArch, m_project),
                    new GenerateConfigLoadingTask(remotePath, name, "Vulkan"),
                    new SerializeAssetsLoadingTask(remotePath, m_project),
                    new SyncRemoteBuildLoadingTask(sshPipe, m_project),
                    new RunRemoteLoadingTask(sshPipe, clientPort, this)
                };

                m_app->PushModal(new LoadingModal(tasks, sizeof(tasks) / sizeof(*tasks)));
            }
        }
        else
        {
            if (FlareImGui::ImageButton("Connect", "Textures/Icons/Controls_Remote.png", glm::vec2(25.0f), false))
            {
                m_app->PushModal(new SSHConnectModal(m_app));
            }
        }
#endif
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
