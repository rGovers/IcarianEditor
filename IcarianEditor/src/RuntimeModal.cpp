#include "Modals/RuntimeModal.h"

#include "AppMain.h"
#include "Runtime/RuntimeManager.h"

RuntimeModal::RuntimeModal(AppMain* a_appMain, RuntimeManager* a_runtime, uint32_t a_index, const std::string_view& a_displayName, const glm::vec2& a_size) : Modal(a_displayName, a_size)
{
    m_appMain = a_appMain;
    m_runtime = a_runtime;

    m_index = a_index;
}
RuntimeModal::~RuntimeModal()
{
    void* args[] =
    {
        &m_index
    };

    m_runtime->ExecFunction("IcarianEditor.Modals", "Modal", ":DisposeModal(uint)", args);
}

bool RuntimeModal::Update()
{
    void* args[] =
    {
        &m_index
    };

    m_runtime->ExecFunction("IcarianEditor.Modals", "Modal", ":UpdateModal(uint)", args);

    return m_appMain->GetRuntimeModalState(m_index);
}
