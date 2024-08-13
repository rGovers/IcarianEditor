// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Workspace.h"

#include <imgui.h>

#include "Core/IcarianDefer.h"
#include "Runtime/RuntimeManager.h"
#include "Windows/EditorWindow.h"

static Workspace* Instance = nullptr;

#include "WorkspaceInterop.h"

#define WORKSPACE_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) BIND_FUNCTION(a_runtime, namespace, klass, name);

WORKSPACE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

Workspace::Workspace(RuntimeManager* a_runtime)
{
    m_runtime = a_runtime;

    m_manipulationMode = ManipulationMode_Translate;

    WORKSPACE_EXPORT_TABLE(WORKSPACE_RUNTIME_ATTACH);

    Instance = this;
}
Workspace::~Workspace()
{
    
}

void Workspace::AddEditorWindow(EditorWindow* a_window)
{
    m_editorWindows.emplace_back(a_window);
}
void Workspace::RemoveEditorWindow(EditorWindow* a_window)
{
    for (auto iter = m_editorWindows.begin(); iter != m_editorWindows.end(); ++iter)
    {
        if (*iter == a_window)
        {
            m_editorWindows.erase(iter);

            return;
        }
    }
}

void Workspace::SetScene(const std::filesystem::path& a_path)
{
    m_currentScene = a_path;

    for (EditorWindow* wind : m_editorWindows)
    {
        wind->Refresh();
    }
}

void Workspace::OpenDef(const std::filesystem::path& a_path)
{
    MonoDomain* editorDomain = m_runtime->GetEditorDomain();

    const std::u32string str = a_path.u32string();
    MonoString* pathString = mono_string_from_utf32((mono_unichar4*)str.c_str());

    void* args[] =
    {
        pathString
    };

    m_runtime->ExecFunction("IcarianEditor", "Workspace", ":PushDef(string)", args);
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