// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Modals/OpenProjectModal.h"

#include <imgui.h>

#include "AppMain.h"
#include "FileDialogBlock.h"
#include "Modals/ErrorModal.h"

constexpr static const char* const Filters[] = { ".icproj", "*" };
constexpr uint32_t FilterCount = sizeof(Filters) / sizeof(*Filters);

OpenProjectModal::OpenProjectModal(AppMain* a_app, Callback a_callback) : Modal("Open Project", glm::vec2(640, 480))
{
    m_app = a_app;

    m_callback = a_callback;

    m_fileDialogBlock = new FileDialogBlock(glm::vec2(-1, -1), false, FilterCount, Filters);
}
OpenProjectModal::~OpenProjectModal()
{
    delete m_fileDialogBlock;
}

bool OpenProjectModal::Update()
{
    std::filesystem::path path;
    std::string name;

    const e_FileDialogStatus status = m_fileDialogBlock->ShowFileDialog(&path, &name);

    switch (status)
    {
    case FileDialogStatus_Ok:
    {
        if (!std::filesystem::exists(path))
        {
            m_app->PushModal(new ErrorModal("Directory does not exist"));

            return true;
        }

        if (name.empty())
        {
            m_app->PushModal(new ErrorModal("Invalid Name"));

            return true;
        }

        m_callback(path, name);

        return false;
    }
    case FileDialogStatus_Error:
    {
        m_app->PushModal(new ErrorModal("File dialog error"));

        break;
    }
    case FileDialogStatus_Cancel:
    {
        return false;
    }
    default:
    {
        break;
    }
    }

    return true;
}

// MIT License
// 
// Copyright (c) 2024 River Govers
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