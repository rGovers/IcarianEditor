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

    GLuint               m_vao;

    bool                 m_focused;

    unsigned char        m_inputByte;

    std::vector<bool>    m_runtimeModalState;

protected:

public:
    AppMain();
    virtual ~AppMain();

    virtual void Update(double a_delta, double a_time);

    inline void RefreshProject()
    {
        m_refresh = true;
    }

    bool GetRuntimeModalState(uint32_t a_index);
    void SetRuntimeModalState(uint32_t a_index, bool a_state);

    void PushModal(Modal* a_modal);
    void DispatchRuntimeModal(const std::string_view& a_title, const glm::vec2& a_size, uint32_t a_index);
    void RemoveRuntimeModal(uint32_t a_index);
};