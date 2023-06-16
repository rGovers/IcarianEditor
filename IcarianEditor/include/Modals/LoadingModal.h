#pragma once

#include "Modals/Modal.h"

class LoadingTask;

class LoadingModal : public Modal
{
private:
    LoadingTask** m_tasks;
    uint32_t      m_taskCount;
    uint32_t      m_currentTask;

protected:

public:
    LoadingModal(LoadingTask* const* a_tasks, uint32_t a_taskCount);
    ~LoadingModal();

    bool Update() override;
};