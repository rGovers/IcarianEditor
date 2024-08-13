// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "Modals/Modal.h"
#include "Project.h"

#include <filesystem>

class AppMain;
class Project;

class RenamePathModal : public Modal
{
private:
    // Generally safe with 2048 characters because Windows still has issues with
    // paths longer than 260 characters despite adding support for longer paths in
    // Windows 10. 
    // Would have to do Windows specific code to use the full 32,767 characters.
    // Because the setting is controller by the user it is not worth the effort.
    // Just picked a number that seemed reasonable for UNIX systems as it just works with them.
    static constexpr uint32_t BufferSize = 2048;

    AppMain*              m_app;
    Project*              m_project;

    std::filesystem::path m_path;

    char                  m_name[BufferSize];

protected:

public:
    RenamePathModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path);
    virtual ~RenamePathModal();

    virtual bool Update();
};

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