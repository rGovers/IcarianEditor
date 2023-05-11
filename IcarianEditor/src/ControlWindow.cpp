#include "Windows/ControlWindow.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "AppMain.h"
#include "FlareImGui.h"
#include "Modals/ErrorModal.h"
#include "ProcessManager.h"
#include "Project.h"
#include "Runtime/RuntimeManager.h"
#include "Workspace.h"

ControlWindow::ControlWindow(AppMain* a_app, ProcessManager* a_processManager, RuntimeManager* a_runtimeManager, Workspace* a_workspace, Project* a_project) : Window("Control")
{
    m_app = a_app;

    m_runtimeManager = a_runtimeManager;
    m_processManager = a_processManager;

    m_workspace = a_workspace;
    m_project = a_project;
}
ControlWindow::~ControlWindow()
{

}

void ControlWindow::TransformControls()
{
    ImGui::BeginGroup();

    if (FlareImGui::ImageButton("Translate", "Textures/Icons/Icon_Translate.png", glm::vec2(16.0f)))
    {
        m_workspace->SetManipulationMode(ManipulationMode_Translate);
    }

    ImGui::SameLine();

    if (FlareImGui::ImageButton("Rotate", "Textures/Icons/Icon_Rotate.png", glm::vec2(16.0f)))
    {
        m_workspace->SetManipulationMode(ManipulationMode_Rotate);
    }

    ImGui::SameLine();

    if (FlareImGui::ImageButton("Scale", "Textures/Icons/Icon_Scale.png", glm::vec2(16.0f)))
    {
        m_workspace->SetManipulationMode(ManipulationMode_Scale);
    }

    ImGui::EndGroup();
}

void ControlWindow::Update(double a_delta)
{
    ImGui::BeginGroup();

    bool running = m_processManager->IsRunning();
    if (FlareImGui::ImageSwitchButton("Run Engine", "Textures/Icons/Controls_Stop.png", "Textures/Icons/Controls_Play.png", &running, glm::vec2(32.0f)))
    {
        if (running)
        {
            if (!m_runtimeManager->IsBuilt())
            {
                m_app->PushModal(new ErrorModal("Cannot start with build errors"));

                return;
            }

            m_processManager->Start(m_project->GetCachePath());
        }
        else
        {
            m_processManager->Stop();
        }
    }

    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();

    ImGui::Columns((int)glm::max(1.0f, (ImGui::GetWindowWidth() - 100.0f) / 100.0f));

    TransformControls();

    ImGui::NextColumn();

    ImGui::EndGroup();
}