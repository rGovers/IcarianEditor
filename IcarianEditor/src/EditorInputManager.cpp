// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "EditorInputManager.h"

#include "Runtime/RuntimeManager.h"

#include "EngineInputInteropStructures.h"

static EditorInputManager* Instance = nullptr;

#define EDITORINPUTMANAGER_BINDING_FUNCTION_TABLE(F) \
    F(glm::vec2, IcarianEngine, Input, GetCursorPos, { return Instance->GetCursorPos(); }) \
    \
    F(uint32_t, IcarianEngine, Input, GetMouseDownState, { return (uint32_t)Instance->IsMouseDown((e_MouseButton)a_button); }, uint32_t a_button) \
    F(uint32_t, IcarianEngine, Input, GetMousePressedState, { return (uint32_t)Instance->IsMousePressed((e_MouseButton)a_button); }, uint32_t a_button) \
    F(uint32_t, IcarianEngine, Input, GetMouseReleasedState, { return (uint32_t)Instance->IsMouseReleased((e_MouseButton)a_button); }, uint32_t a_button) \
    \
    F(uint32_t, IcarianEngine, Input, GetKeyDownState, { return (uint32_t)Instance->IsKeyDown((e_KeyCode)a_keyCode); }, uint32_t a_keyCode) \
    F(uint32_t, IcarianEngine, Input, GetKeyPressedState, { return (uint32_t)Instance->IsKeyPressed((e_KeyCode)a_keyCode); }, uint32_t a_keyCode) \
    F(uint32_t, IcarianEngine, Input, GetKeyReleasedState, { return (uint32_t)Instance->IsKeyReleased((e_KeyCode)a_keyCode); }, uint32_t a_keyCode) \

EDITORINPUTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

EditorInputManager::EditorInputManager()
{
    m_curPos = glm::vec2(0);

    m_mouseButton = 0;

    EDITORINPUTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
}
EditorInputManager::~EditorInputManager()
{
    

}

void EditorInputManager::Init()
{
    if (Instance == nullptr)
    {
        Instance = new EditorInputManager();
    }   
}
void EditorInputManager::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void EditorInputManager::SetCursorPos(const glm::vec2& a_pos)
{
    Instance->m_curPos = a_pos;
}
glm::vec2 EditorInputManager::GetCursorPos()
{
    return Instance->m_curPos;
}

void EditorInputManager::SetMouseButton(e_MouseButton a_button, bool a_state)
{
    const uint32_t buttonIndex = a_button * 2 + 0;
    const uint32_t prevButtonIndex = a_button * 2 + 1;

    // const bool prevState = m_mouseButton & 0b1 << buttonIndex;
    const bool prevState = IISBITSET(Instance->m_mouseButton, buttonIndex);
    if (a_state)
    {
        if (!(prevState))
        {
            uint32_t but = (uint32_t)a_button;
            void* args[] =
            {
                &but
            };

            RuntimeManager::ExecFunction("IcarianEngine", "Input", ":MousePressedEvent(uint)", args);
        }

        ISETBIT(Instance->m_mouseButton, buttonIndex);
    }
    else
    {
        if (prevState)
        {
            uint32_t but = (uint32_t)a_button;
            void* args[] =
            {
                &but
            };

            RuntimeManager::ExecFunction("IcarianEngine", "Input", ":MouseReleasedEvent(uint)", args);
        }

        ICLEARBIT(Instance->m_mouseButton, buttonIndex);
    }

    ITOGGLEBIT(prevState, Instance->m_mouseButton, prevButtonIndex);
}

void EditorInputManager::SetKeyboardKey(e_KeyCode a_keyCode, bool a_state)
{
    const bool prevState = Instance->m_curKeyState.IsKeyDown(a_keyCode);

    Instance->m_curKeyState.SetKey(a_keyCode, a_state);
    if (a_state)
    {
        if (!prevState)
        {
            uint32_t key = (uint32_t)a_keyCode;
            void* args[]
            {
                &key
            };

            RuntimeManager::ExecFunction("IcarianEngine", "Input", ":KeyPressedEvent(uint)", args);
        }   
    }
    else if (prevState)
    {
        uint32_t key = (uint32_t)a_keyCode;
        void* args[]
        {
            &key
        };
        
        RuntimeManager::ExecFunction("IcarianEngine", "Input", ":KeyReleasedEvent(uint)", args);
    }

    Instance->m_prevKeyState.SetKey(a_keyCode, prevState);
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