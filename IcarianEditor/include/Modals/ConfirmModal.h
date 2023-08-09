#pragma once

#include "Modals/Modal.h"

class ConfirmModalData
{
private:

protected:

public:
    ConfirmModalData() { }
    virtual ~ConfirmModalData() { } 

    virtual void Confirm() = 0;
    virtual void Cancel() { };
};

class ConfirmModal : public Modal
{
private:
    std::string       m_text;

    ConfirmModalData* m_data;

protected:

public:
    ConfirmModal(const std::string_view& a_text, ConfirmModalData* a_data);
    virtual ~ConfirmModal();

    virtual bool Update();
};