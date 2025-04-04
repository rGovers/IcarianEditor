// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Windows/GameWindow.h"

#include <imgui.h>

#include "AppMain.h"
#include "Core/IcarianLambda.h"
#include "Core/InputBindings.h"
#include "FlareImGui.h"
#include "LoadingTasks/GenerateConfigLoadingTask.h"
#include "LoadingTasks/RemoteBuildLoadingTask.h"
#include "LoadingTasks/RunRemoteLoadingTask.h"
#include "LoadingTasks/SerializeAssetsLoadingTask.h"
#include "LoadingTasks/SyncRemoteBuildLoadingTask.h"
#include "Modals/ErrorModal.h"
#include "Modals/LoadingModal.h"
#include "Modals/SSHConnectModal.h"
#include "ProcessManager.h"
#include "Project.h"
#include "Runtime/RuntimeManager.h"

GameWindow::GameWindow(AppMain* a_app, AssetLibrary* a_library, ProcessManager* a_processManager, Project* a_project) : Window("Game", "Textures/WindowIcons/WindowIcon_Game.png")
{
    m_app = a_app;

    m_library = a_library;
    m_processManager = a_processManager;
    m_project = a_project;
}
GameWindow::~GameWindow()
{

}

void GameWindow::Update(double a_delta)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 vMinIm = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMaxIm = ImGui::GetWindowContentRegionMax();
    const ImVec2 sizeIm = { vMaxIm.x - vMinIm.x, vMaxIm.y - vMinIm.y };

    m_processManager->SetSize((uint32_t)sizeIm.x, (uint32_t)sizeIm.y);

    const bool locked = m_processManager->GetCursorState() == CursorState_Locked;

    // The oh fuck the app has taken input away button stop giving control
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent))
    {
        const bool captureInput = !m_processManager->GetCaptureInput();

        m_processManager->SetCaptureInput(captureInput);

        if (captureInput && locked)
        {
            m_app->SetCursorState(CursorState_Locked);
        }
        else
        {
            m_app->SetCursorState(CursorState_Normal);
        }
    }

    const bool focused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();
    const bool captureInput = m_processManager->GetCaptureInput();

    if (captureInput && (focused || locked))
    {
        const ImGuiStyle& style = ImGui::GetStyle();

        if (locked)
        {
            m_app->SetCursorState(CursorState_Locked);

            const glm::vec2 cursorPos = m_app->GetCursorPos();

            const glm::vec2 delta = cursorPos - m_lastCursorPos;
            m_processManager->PushCursorPos(delta);

            m_lastCursorPos = cursorPos;
        }
        else 
        {
            m_app->SetCursorState(CursorState_Normal);

            const ImVec2 mousePosIm = ImGui::GetMousePos();

            const glm::vec2 cPos = glm::vec2(mousePosIm.x - (winPos.x + vMinIm.x), mousePosIm.y - (winPos.y + vMinIm.y));

            m_processManager->PushCursorPos(cPos);
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

        m_processManager->PushMouseState(mouseState);

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

        m_processManager->PushKeyboardState(state);
    }

    ImGui::Image((ImTextureID)(uintptr_t)m_processManager->GetImage(), sizeIm);

    const bool isRunning = m_processManager->IsRunning();
    const bool isRemote = m_processManager->IsRemoteConnected();
    const bool isRemoteRunning = m_processManager->IsRemoteRunning();

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

    const ImVec2 rectMin = ImVec2(winPos.x + halfSize.x - winHalfSize.x, winPos.y + 40.0f);
    const ImVec2 rectMax = ImVec2(winPos.x + halfSize.x + winHalfSize.x, winPos.y + 40.0f + winSize.y);

    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(30, 30, 30, 150), 2.0f);

    ImGui::SetCursorPos(ImVec2(halfSize.x - winHalfSize.x + 5.0f, 45.0f));

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
            m_processManager->Stop();

            m_app->SetCursorState(CursorState_Normal);
        }

        if (!isRemoteRunning)
        {
            ImGui::SameLine();

            if (FlareImGui::ImageButton("Capture Frame", "Textures/Icons/Controls_Screencapture.png", glm::vec2(25.0f), false))
            {
                m_processManager->CaptureFrame();
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

            const std::filesystem::path cachePath = m_project->GetCachePath();

            m_processManager->Start(cachePath);
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

                LoadingTask* tasks[] = 
                {
                    new RemoteBuildLoadingTask(m_processManager, m_project),
                    new GenerateConfigLoadingTask(remotePath, name, "Vulkan"),
                    new SerializeAssetsLoadingTask(remotePath, m_project, m_library),
                    new SyncRemoteBuildLoadingTask(m_processManager, m_project),
                    new RunRemoteLoadingTask(m_processManager)
                };

                m_app->PushModal(new LoadingModal(tasks, sizeof(tasks) / sizeof(*tasks)));
            }
        }
        else
        {
            if (FlareImGui::ImageButton("Connect", "Textures/Icons/Controls_Remote.png", glm::vec2(25.0f), false))
            {
                m_app->PushModal(new SSHConnectModal(m_app, m_processManager));
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