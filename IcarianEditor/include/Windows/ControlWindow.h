#pragma once

#include "Windows/Window.h"

class AppMain;
class ProcessManager;
class Project;
class RuntimeManager;
class Workspace;

class ControlWindow : public Window
{
private:
    AppMain*        m_app;

    RuntimeManager* m_runtimeManager;
    ProcessManager* m_processManager;

    Project*        m_project;
    Workspace*      m_workspace;

    void TransformControls();
    
protected:

public:
    ControlWindow(AppMain* a_app, ProcessManager* a_processManager, RuntimeManager* a_runtimeManager, Workspace* a_workspace, Project* a_project);
    ~ControlWindow();

    virtual void Update(double a_delta);
};