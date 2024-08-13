// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <list>
#include <vector>

class AppMain;
class AssetLibrary;
class FileDialogBlock;
class Project;

class BuildProjectModal : public Modal
{
private:
    AppMain*                         m_app;
    AssetLibrary*                    m_library;
    Project*                         m_project;

    FileDialogBlock*                 m_fileDialogBlock;

    uint32_t                         m_selectedExport;
    std::vector<std::string>         m_exportOptions;

    void OptionsDisplay();

protected:

public:
    BuildProjectModal(AppMain* a_app, AssetLibrary* a_library, Project* a_project, const std::vector<std::string>& a_exportOptions);
    virtual ~BuildProjectModal();

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