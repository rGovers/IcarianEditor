// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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
    ImGui::Text("[%d/%d] Running Tasks....", m_currentTask, m_taskCount);

    m_tasks[m_currentTask++]->Run();

    return m_currentTask < m_taskCount;
}

// MIT License
// 
// Copyright (c) 2025 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.