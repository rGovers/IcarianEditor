#pragma once

#include "Modals/Modal.h"

enum e_EditorConfigTab
{
    EditorConfigTab_General,
    EditorConfigTab_End
};

class EditorConfigModal : public Modal
{
private:
    e_EditorConfigTab m_currentTab = EditorConfigTab_General;

    static void GeneralTab();
    
protected:

public:
    EditorConfigModal();
    virtual ~EditorConfigModal();

    virtual bool Update();
};