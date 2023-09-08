#pragma once

#include "Windows/Window.h"

class RuntimeManager;

class SceneDefsWindow : public Window
{
private:
    RuntimeManager* m_runtime;
    
protected:

public:
    SceneDefsWindow(RuntimeManager* a_runtime);
    ~SceneDefsWindow();

    virtual void Update(double a_delta);
};