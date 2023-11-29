#pragma once

#include "Windows/Window.h"

class RuntimeManager;

class TimelineWindow : public Window
{
private:
    static constexpr float TimelineOffset = 240.0f;

    RuntimeManager* m_runtime;

protected:

public:
    TimelineWindow(RuntimeManager* a_runtime);
    ~TimelineWindow();

    virtual void Update(double a_delta);
};