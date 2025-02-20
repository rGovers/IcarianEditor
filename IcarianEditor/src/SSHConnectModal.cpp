// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/SSHConnectModal.h"

#include <imgui.h>

#include "AppMain.h"
#include "FlareImGui.h"
#include "Modals/ErrorModal.h"
#include "Modals/SSHConnectedModal.h"
#include "ProcessManager.h"

SSHConnectModal::SSHConnectModal(AppMain* a_app, ProcessManager* a_processManager) : Modal("Connect SSH")
{
    m_app = a_app;
    m_processManager = a_processManager;

    m_user[0] = 0;
    m_addr[0] = 0;
    m_port = 22;

    m_compress = true;

    // *Crushes Scouter*
    m_clientPort = 9001;
}
SSHConnectModal::~SSHConnectModal()
{

}

bool SSHConnectModal::Update()
{
    FlareImGui::Label("User");
    ImGui::SameLine();
    ImGui::InputText("##User", m_user, sizeof(m_user));

    FlareImGui::Label("Address");
    ImGui::SameLine();
    ImGui::InputText("##Address", m_addr, sizeof(m_addr));

    FlareImGui::Label("SSH Port");
    ImGui::SameLine();
    ImGui::DragInt("##SSHPort", &m_port, 1.0f, 0, 65535);

    FlareImGui::Label("Compress");
    ImGui::SameLine();
    ImGui::Checkbox("##Compress", &m_compress);

    ImGui::Separator();

    FlareImGui::Label("Client Port");
    ImGui::SameLine();
    ImGui::DragInt("##ClientPort", &m_clientPort, 1.0f, 0, 65535);

    if (ImGui::Button("Connect"))
    {
        if (m_port > 65535 || m_port < 0)
        {
            m_app->PushModal(new ErrorModal("Invalid SSH Port"));

            return true;
        }

        if (m_clientPort > 65535 || m_clientPort < 0)
        {
            m_app->PushModal(new ErrorModal("Invalid Client Port"));

            return true;
        }

        if (!m_processManager->ConnectRemotePassword(m_user, m_addr, (uint16_t)m_port, (uint16_t)m_clientPort, m_compress))
        {
            m_app->PushModal(new ErrorModal("Failed to connect"));

            return true;
        }

        SSHPipe* sshPipe = m_processManager->GetRemotePipe();

        m_app->PushModal(new SSHConnectedModal(sshPipe));

        return false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        return false;
    }

    return true;
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