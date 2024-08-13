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