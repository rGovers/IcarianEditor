#pragma once

#include "Windows/Window.h"

class RuntimeManager;

class HierarchyWindow : public Window
{
private:
    RuntimeManager* m_runtime;

protected:

public:
    HierarchyWindow(RuntimeManager* a_runtime);
    ~HierarchyWindow();

    virtual void Update(double a_delta);
};