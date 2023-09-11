#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Windows/Window.h"

class Application;
class ProcessManager;

class GameWindow : public Window
{
private:
    Application*    m_app;
    ProcessManager* m_processManager;
    
    glm::vec2       m_lastCursorPos;
protected:

public:
    GameWindow(Application* a_app, ProcessManager* a_processManager);
    ~GameWindow();

    virtual void Update(double a_delta);
};