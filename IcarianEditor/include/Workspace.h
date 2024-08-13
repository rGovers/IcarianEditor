// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <filesystem>
#include <functional>

class EditorWindow;
class RuntimeManager;

#include "Gizmos.h"

class Workspace
{
private:
    RuntimeManager*            m_runtime;

    std::filesystem::path      m_currentScene;

    std::filesystem::path      m_selectionPath;

    e_ManipulationMode         m_manipulationMode;

    std::vector<EditorWindow*> m_editorWindows;

protected:

public:
    Workspace(RuntimeManager* a_runtime);
    ~Workspace();

    inline std::filesystem::path GetCurrentScene() const
    {
        return m_currentScene;
    }
    inline void SetCurrentScene(const std::filesystem::path& a_path)
    {
        m_currentScene = a_path;
    }

    inline e_ManipulationMode GetManipulationMode() const
    {
        return m_manipulationMode;
    }
    inline void SetManipulationMode(e_ManipulationMode a_mode) 
    {
        m_manipulationMode = a_mode;
    }

    inline RuntimeManager* GetRuntime() const
    {
        return m_runtime;
    }

    void AddEditorWindow(EditorWindow* a_window);
    void RemoveEditorWindow(EditorWindow* a_window);

    void SetScene(const std::filesystem::path& a_path);

    void OpenDef(const std::filesystem::path& a_path);
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