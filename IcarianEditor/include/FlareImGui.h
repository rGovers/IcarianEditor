#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <imgui.h>
#include <string_view>

class Texture;

namespace FlareImGui
{
    void Label(const std::string_view& a_label);

    bool DragVec2(const std::string_view& a_label, float a_val[2], float a_speed = 1.0f, float a_min = 0.0f, float a_max = 0.0f, const char* a_format = "%.3f", ImGuiSliderFlags a_flags = 0);
    bool DragVec3(const std::string_view& a_label, float a_val[3], float a_speed = 1.0f, float a_min = 0.0f, float a_max = 0.0f, const char* a_format = "%.3f", ImGuiSliderFlags a_flags = 0);
    bool DragVec4(const std::string_view& a_label, float a_val[4], float a_speed = 1.0f, float a_min = 0.0f, float a_max = 0.0f, const char* a_format = "%.3f", ImGuiSliderFlags a_flags = 0);

    bool Image(const char* a_path, const ImVec2& a_size);
    bool Image(const std::string_view& a_path, const glm::vec2& a_size);

    bool ImageButton(Texture* a_texture, const glm::vec2& a_size, bool a_background = true);
    bool ImageButton(const char* a_label, const char* a_path, const ImVec2& a_size, bool a_background = true);
    bool ImageButton(const std::string_view& a_label, const std::string_view& a_path, const glm::vec2& a_size, bool a_background = true);

    bool ImageSwitchButton(const char* a_label, const char* a_pathEnabled, const char* a_pathDisabled, bool* a_state, const ImVec2& a_size);
    bool ImageSwitchButton(const std::string_view& a_label, const std::string_view& a_pathEnabled, const std::string_view& a_pathDisabled, bool* a_state, const glm::vec2& a_size);
}