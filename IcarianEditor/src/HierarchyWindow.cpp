#include "Windows/HierarchyWindow.h"

#include "Runtime/RuntimeManager.h"

HierarchyWindow::HierarchyWindow(RuntimeManager* a_runtime) : Window("Hierarchy", "Textures/WindowIcons/WindowIcon_Hierarchy.png")
{
    m_runtime = a_runtime;
}
HierarchyWindow::~HierarchyWindow()
{

}

void HierarchyWindow::Update(double a_delta)
{
    m_runtime->ExecFunction("IcarianEditor.Windows", "HierarchyWindow", ":OnGUI()", nullptr);
}