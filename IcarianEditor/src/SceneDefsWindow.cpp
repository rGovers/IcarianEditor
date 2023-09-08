#include "Windows/SceneDefsWindow.h"

#include <imgui.h>

#include "GUI.h"
#include "Runtime/RuntimeManager.h"

SceneDefsWindow::SceneDefsWindow(RuntimeManager* a_runtime) : Window("Scene Definitions")
{
    m_runtime = a_runtime;
}
SceneDefsWindow::~SceneDefsWindow()
{

}

void SceneDefsWindow::Update(double a_delta)
{
    GUI::SetWidth(ImGui::GetWindowSize().x);

    m_runtime->ExecFunction("IcarianEditor.Windows", "SceneDefsWindow", ":OnGUI()", nullptr);
}