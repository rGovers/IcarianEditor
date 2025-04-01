// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glad/glad.h>
#include <imgui.h>
#include <string_view>

class Texture;

#define TexToImHandle(texture) (ImTextureID)(uintptr_t)(texture)->GetHandle()

namespace FlareImGui
{
    constexpr ImGuiKey ImGuiKeyTable[] = 
    {
        ImGuiKey_Space,
        ImGuiKey_Apostrophe,
        ImGuiKey_Comma,
        ImGuiKey_Minus,
        ImGuiKey_Equal,
        ImGuiKey_Period,
        ImGuiKey_Slash,
        ImGuiKey_Backslash,
        ImGuiKey_LeftBracket,
        ImGuiKey_RightBracket,
        ImGuiKey_Semicolon,
        ImGuiKey_GraveAccent,
    
        ImGuiKey_A, 
        ImGuiKey_B, 
        ImGuiKey_C, 
        ImGuiKey_D, 
        ImGuiKey_E, 
        ImGuiKey_F, 
        ImGuiKey_G, 
        ImGuiKey_H, 
        ImGuiKey_I, 
        ImGuiKey_J,
        ImGuiKey_K, 
        ImGuiKey_L, 
        ImGuiKey_M, 
        ImGuiKey_N, 
        ImGuiKey_O, 
        ImGuiKey_P, 
        ImGuiKey_Q, 
        ImGuiKey_R, 
        ImGuiKey_S, 
        ImGuiKey_T,
        ImGuiKey_U, 
        ImGuiKey_V, 
        ImGuiKey_W, 
        ImGuiKey_X, 
        ImGuiKey_Y, 
        ImGuiKey_Z,
    
        ImGuiKey_0, 
        ImGuiKey_1, 
        ImGuiKey_2, 
        ImGuiKey_3, 
        ImGuiKey_4, 
        ImGuiKey_5, 
        ImGuiKey_6, 
        ImGuiKey_7, 
        ImGuiKey_8, 
        ImGuiKey_9,
    
        ImGuiKey_KeypadDecimal,
        ImGuiKey_KeypadDivide,
        ImGuiKey_KeypadMultiply,
        ImGuiKey_KeypadSubtract,
        ImGuiKey_KeypadAdd,
        ImGuiKey_KeypadEqual,
        ImGuiKey_KeypadEnter,
    
        ImGuiKey_Keypad0, 
        ImGuiKey_Keypad1, 
        ImGuiKey_Keypad2, 
        ImGuiKey_Keypad3, 
        ImGuiKey_Keypad4,
        ImGuiKey_Keypad5, 
        ImGuiKey_Keypad6, 
        ImGuiKey_Keypad7, 
        ImGuiKey_Keypad8, 
        ImGuiKey_Keypad9,
    
        ImGuiKey_Escape,
        ImGuiKey_Enter,
        ImGuiKey_Tab,
        ImGuiKey_Backspace,
        ImGuiKey_Insert,
        ImGuiKey_Delete,
        ImGuiKey_Home,
        ImGuiKey_End,
        ImGuiKey_PageUp,
        ImGuiKey_PageDown,
        ImGuiKey_LeftArrow,
        ImGuiKey_RightArrow,
        ImGuiKey_UpArrow,
        ImGuiKey_DownArrow,
        ImGuiKey_CapsLock,
        ImGuiKey_NumLock,
        ImGuiKey_ScrollLock,
        ImGuiKey_PrintScreen,
        ImGuiKey_Pause,
    
        ImGuiKey_LeftShift,
        ImGuiKey_LeftCtrl,
        ImGuiKey_LeftAlt,
        ImGuiKey_LeftSuper,
    
        ImGuiKey_RightShift,
        ImGuiKey_RightCtrl,
        ImGuiKey_RightAlt,
        ImGuiKey_RightSuper,
    
        ImGuiKey_F1, 
        ImGuiKey_F2, 
        ImGuiKey_F3, 
        ImGuiKey_F4, 
        ImGuiKey_F5, 
        ImGuiKey_F6,
        ImGuiKey_F7, 
        ImGuiKey_F8, 
        ImGuiKey_F9, 
        ImGuiKey_F10,
        ImGuiKey_F11, 
        ImGuiKey_F12,
        ImGuiKey_None, // 13
        ImGuiKey_None, // 14
        ImGuiKey_None, // 15
        ImGuiKey_None, // 16
        ImGuiKey_None, // 17
        ImGuiKey_None, // 18
        ImGuiKey_None, // 19
        ImGuiKey_None, // 20
        ImGuiKey_None, // 21
        ImGuiKey_None, // 22
        ImGuiKey_None, // 23
        ImGuiKey_None, // 24
        ImGuiKey_None, // 25
    
        ImGuiKey_Menu
    };

    void Label(const std::string_view& a_label);

    bool DragVec2(const std::string_view& a_label, float a_val[2], float a_speed = 1.0f, float a_min = 0.0f, float a_max = 0.0f, const char* a_format = "%.4f", ImGuiSliderFlags a_flags = 0);
    bool DragVec3(const std::string_view& a_label, float a_val[3], float a_speed = 1.0f, float a_min = 0.0f, float a_max = 0.0f, const char* a_format = "%.4f", ImGuiSliderFlags a_flags = 0);
    bool DragVec4(const std::string_view& a_label, float a_val[4], float a_speed = 1.0f, float a_min = 0.0f, float a_max = 0.0f, const char* a_format = "%.4f", ImGuiSliderFlags a_flags = 0);

    bool Image(const char* a_path, const ImVec2& a_size);
    bool Image(const std::string_view& a_path, const glm::vec2& a_size);

    bool ImageButton(const char* a_label, GLuint a_texture, const glm::vec2& a_size, bool a_background = true);
    bool ImageButton(const char* a_label, Texture* a_texture, const glm::vec2& a_size, bool a_background = true);
    bool ImageButton(const char* a_label, const char* a_path, const ImVec2& a_size, bool a_background = true);
    bool ImageButton(const std::string_view& a_label, const std::string_view& a_path, const glm::vec2& a_size, bool a_background = true);

    bool ImageSwitchButton(const char* a_label, const char* a_pathEnabled, const char* a_pathDisabled, bool* a_state, const ImVec2& a_size);
    bool ImageSwitchButton(const std::string_view& a_label, const std::string_view& a_pathEnabled, const std::string_view& a_pathDisabled, bool* a_state, const glm::vec2& a_size);
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