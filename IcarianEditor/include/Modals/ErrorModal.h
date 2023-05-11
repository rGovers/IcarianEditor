#pragma once

#include "Modals/Modal.h"

class ErrorModal : public Modal
{
private:
    std::string            m_error;

protected:

public:
    ErrorModal(const std::string_view& a_error);
    virtual ~ErrorModal();

    virtual bool Update();
};