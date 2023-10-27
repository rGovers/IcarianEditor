#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Windows/Window.h"

class AppMain;
class ProcessManager;
class Project;
class RuntimeManager;

class GameWindow : public Window
{
private:
    AppMain*        m_app;

    ProcessManager* m_processManager;
    RuntimeManager* m_runtimeManager;
    Project*        m_project;
    
    glm::vec2       m_lastCursorPos;
protected:

public:
    GameWindow(AppMain* a_app, ProcessManager* a_processManager, RuntimeManager* a_runtime, Project* a_project);
    ~GameWindow();

    virtual void Update(double a_delta);
};