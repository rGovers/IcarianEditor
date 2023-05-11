#include "FlareImGui.h"

#include "Datastore.h"
#include "Texture.h"

#include <imgui_internal.h>

namespace FlareImGui
{
    bool ImageButton(Texture* a_texture, const glm::vec2& a_size, bool a_background)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        const ImVec4 color = style.Colors[ImGuiCol_Button];
        const ImVec4 aColor = style.Colors[ImGuiCol_ButtonActive];

        if (!a_background)
        {
            style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        }

        bool ret = false;

        if (a_texture != nullptr)
        {
            ret = ImGui::ImageButton((ImTextureID)a_texture->GetHandle(), { a_size.x, a_size.y });
        }

        style.Colors[ImGuiCol_Button] = color;
        style.Colors[ImGuiCol_ButtonActive] = aColor;

        return ret;
    }
    bool ImageButton(const char* a_label, const char* a_path, const ImVec2& a_size, bool a_background)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        const ImVec4 color = style.Colors[ImGuiCol_Button];
        const ImVec4 aColor = style.Colors[ImGuiCol_ButtonActive];

        if (!a_background)
        {
            style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        }

        bool ret;

        const Texture* tex = Datastore::GetTexture(a_path);
        if (tex != nullptr)
        {
            const ImGuiID id = ImGui::GetID(a_label);

            ImGui::PushID(id);
            ret = ImGui::ImageButton((ImTextureID)tex->GetHandle(), a_size);
            ImGui::PopID();
        }
        else
        {
            ret = ImGui::Button(a_label, a_size);
        }

        style.Colors[ImGuiCol_Button] = color;
        style.Colors[ImGuiCol_ButtonActive] = aColor;

        return ret;
    }
    bool ImageButton(const std::string_view& a_label, const std::string_view& a_path, const glm::vec2& a_size, bool a_background)
    {
        return ImageButton(a_label.data(), a_path.data(), { a_size.x, a_size.y }, a_background);
    }

    bool ImageSwitchButton(const char* a_label, const char* a_pathEnabled, const char* a_pathDisabled, bool* a_state, const ImVec2& a_size)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        const ImVec4 color = style.Colors[ImGuiCol_Button];
        const ImVec4 aColor = style.Colors[ImGuiCol_ButtonActive];

        style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        const char* path = a_pathDisabled;
        if (*a_state)
        {
            path = a_pathEnabled;
        }

        bool ret;

        const Texture* tex = Datastore::GetTexture(path);
        if (tex != nullptr)
        {
            const ImGuiID id = ImGui::GetID(a_label);

            ImGui::PushID(id);
            ret = ImGui::ImageButton((ImTextureID)tex->GetHandle(), a_size);
            ImGui::PopID();

            if (ret)
            {
                *a_state = !*a_state;
            }
        }
        else
        {
            ret = ImGui::Checkbox(a_label, a_state);
        }

        style.Colors[ImGuiCol_Button] = color;
        style.Colors[ImGuiCol_ButtonActive] = aColor;

        return ret;
    }
    bool ImageSwitchButton(const std::string_view& a_label, const std::string_view& a_pathEnabled, const std::string_view& a_pathDisabled, bool* a_state, const glm::vec2& a_size)
    {
        return ImageSwitchButton(a_label.data(), a_pathEnabled.data(), a_pathDisabled.data(), a_state, { a_size.x, a_size.y });
    }
}