#include "Windows/PropertiesWindow.h"

#include <imgui.h>

#include "GUI.h"
#include "Runtime/RuntimeManager.h"

PropertiesWindow::PropertiesWindow(RuntimeManager* a_runtime) : Window("Properties", "Textures/WindowIcons/WindowIcon_Properties.png")
{
    m_runtime = a_runtime;
}
PropertiesWindow::~PropertiesWindow()
{

}

void PropertiesWindow::Update(double a_delta)
{
    GUI::SetWidth(ImGui::GetWindowSize().x);

    m_runtime->ExecFunction("IcarianEditor.Windows", "PropertiesWindow", ":OnGUI", NULL);
}