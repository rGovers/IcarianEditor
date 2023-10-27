#pragma once

#include "Modals/Modal.h"

enum e_EditorConfigTab
{
    EditorConfigTab_General,
    EditorConfigTab_KeyBindings,
    EditorConfigTab_ExternalTools,
    EditorConfigTab_End
};

enum e_KeyBindTarget
{
    KeyBindTarget_None,
    KeyBindTarget_Translate,
    KeyBindTarget_Rotate,
    KeyBindTarget_Scale,
};

class EditorConfigModal : public Modal
{
private:
    static constexpr float ItemWidth = 150.0f;

    e_EditorConfigTab m_currentTab = EditorConfigTab_General;
    e_KeyBindTarget   m_keyBindTarget = KeyBindTarget_None;

    static void GeneralTab();
    void KeyBindingsTab();
    static void ExternalToolsTab();

protected:

public:
    EditorConfigModal();
    virtual ~EditorConfigModal();

    virtual bool Update();
};