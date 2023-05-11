#pragma once

#include "Windows/Window.h"

class ProcessManager;

class GameWindow : public Window
{
private:
    ProcessManager* m_processManager;
    
protected:

public:
    GameWindow(ProcessManager* a_processManager);
    ~GameWindow();

    virtual void Update(double a_delta);
};