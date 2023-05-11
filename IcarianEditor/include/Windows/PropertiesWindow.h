#pragma once

#include "Windows/Window.h"

class RuntimeManager;

class PropertiesWindow : public Window
{
private:
    RuntimeManager* m_runtime;

protected:

public:
    PropertiesWindow(RuntimeManager* a_runtime);
    ~PropertiesWindow();

    virtual void Update(double a_delta);
};