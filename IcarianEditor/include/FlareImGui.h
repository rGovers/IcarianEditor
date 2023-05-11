#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <imgui.h>
#include <string_view>

class Texture;

namespace FlareImGui
{
    bool ImageButton(Texture* a_texture, const glm::vec2& a_size, bool a_background = true);
    bool ImageButton(const char* a_label, const char* a_path, const ImVec2& a_size, bool a_background = true);
    bool ImageButton(const std::string_view& a_label, const std::string_view& a_path, const glm::vec2& a_size, bool a_background = true);

    bool ImageSwitchButton(const char* a_label, const char* a_pathEnabled, const char* a_pathDisabled, bool* a_state, const ImVec2& a_size);
    bool ImageSwitchButton(const std::string_view& a_label, const std::string_view& a_pathEnabled, const std::string_view& a_pathDisabled, bool* a_state, const glm::vec2& a_size);
}