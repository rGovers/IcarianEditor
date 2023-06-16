#include "Modals/LoadingModal.h"

#include <imgui.h>

#include "LoadingTasks/LoadingTask.h"

LoadingModal::LoadingModal(LoadingTask* const* a_tasks, uint32_t a_taskCount) : Modal("Loading", glm::vec2(200, 100))
{
    m_taskCount = a_taskCount;
    m_currentTask = 0;

    m_tasks = new LoadingTask*[a_taskCount];
    for (uint32_t i = 0; i < a_taskCount; ++i)
    {
        m_tasks[i] = a_tasks[i];
    }
}
LoadingModal::~LoadingModal()
{
    for (uint32_t i = 0; i < m_taskCount; ++i)
    {
        delete m_tasks[i];
    }

    delete[] m_tasks;
}

bool LoadingModal::Update()
{
    ImGui::Text("%s", "Running Tasks...");

    m_tasks[m_currentTask++]->Run();

    return m_currentTask < m_taskCount;
}