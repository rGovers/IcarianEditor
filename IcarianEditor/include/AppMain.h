#pragma once

#include "Application.h"

#include <vector>

#include "Windows/Window.h"
#include "Modals/Modal.h"

class AssetLibrary;
class Project;
class ProcessManager;
class RuntimeManager;
class RuntimeStorage;
class Workspace;

class AppMain : public Application
{
private:
    static constexpr int SaveBit = 0;
    static constexpr int LoadBit = 1;

    double               m_titleSet;

    bool                 m_refresh;

    std::vector<Window*> m_windows;
    std::vector<Modal*>  m_modals;

    AssetLibrary*        m_assets;
    Project*             m_project;
    ProcessManager*      m_process;
    RuntimeManager*      m_runtime;
    RuntimeStorage*      m_rStorage;
    Workspace*           m_workspace;

    bool                 m_focused;

    unsigned char        m_inputByte;

protected:

public:
    AppMain();
    virtual ~AppMain();

    virtual void Update(double a_delta, double a_time);

    inline void RefreshProject()
    {
        m_refresh = true;
    }

    void PushModal(Modal* a_modal);
};