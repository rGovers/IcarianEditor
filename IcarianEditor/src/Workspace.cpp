// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Workspace.h"

#include <imgui.h>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Runtime/RuntimeManager.h"

static Workspace* Instance = nullptr;

#include "EditorWorkspaceInterop.h"

WORKSPACE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

Workspace::Workspace()
{
    m_manipulationMode = ManipulationMode_Translate;

    WORKSPACE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
Workspace::~Workspace()
{

}

void Workspace::Init()
{
    if (Instance == nullptr)
    {
        Instance = new Workspace();
    }
}
void Workspace::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

std::filesystem::path Workspace::GetCurrentScene()
{
    ICARIAN_ASSERT(Instance != nullptr);

    return Instance->m_currentScene;
}
void Workspace::SetCurrentScene(const std::filesystem::path& a_path)
{
    ICARIAN_ASSERT(Instance != nullptr);

    Instance->m_currentScene = a_path;
}

e_ManipulationMode Workspace::GetManipulationMode()
{
    ICARIAN_ASSERT(Instance != nullptr);

    return Instance->m_manipulationMode;
}
void Workspace::SetManipulationMode(e_ManipulationMode a_mode) 
{
    ICARIAN_ASSERT(Instance != nullptr);

    Instance->m_manipulationMode = a_mode;
}

void Workspace::OpenDef(const std::filesystem::path& a_path)
{
    ICARIAN_ASSERT(Instance != nullptr);

    const std::u16string str = a_path.u16string();
    MonoString* pathString = mono_string_from_utf16((mono_unichar2*)str.c_str());

    void* args[] =
    {
        pathString
    };

    RuntimeManager::ExecFunction("IcarianEditor", "Workspace", ":PushDef(string)", args);
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
