#pragma once

#include "Modals/Modal.h"

class AppMain;
class RuntimeManager;

class RuntimeModal : public Modal
{
private: 
    AppMain*        m_appMain;
    RuntimeManager* m_runtime;

    uint32_t        m_index;

protected:

public:
    RuntimeModal(AppMain* a_app, RuntimeManager* a_runtime, uint32_t a_index, const std::string_view& a_displayName, const glm::vec2& a_size);
    virtual ~RuntimeModal();

    virtual bool Update();
};