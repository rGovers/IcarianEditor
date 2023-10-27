#pragma once

#include "Modals/Modal.h"

#include "EditorConfig.h"

enum e_EditorConfigTab
{
    EditorConfigTab_General,
    EditorConfigTab_KeyBindings,
    EditorConfigTab_ExternalTools,
    EditorConfigTab_End
};

class EditorConfigModal : public Modal
{
private:
    static constexpr float ItemWidth = 150.0f;

    e_EditorConfigTab m_currentTab = EditorConfigTab_General;
    e_KeyBindTarget   m_keyBindTarget = KeyBindTarget_Null;

    static void GeneralTab();
    void KeyBindingsTab();
    static void ExternalToolsTab();

protected:

public:
    EditorConfigModal();
    virtual ~EditorConfigModal();

    virtual bool Update();
};