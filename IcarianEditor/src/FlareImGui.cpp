#include "FlareImGui.h"

#include "Core/IcarianDefer.h"
#include "Datastore.h"
#include "Texture.h"

#include <imgui_internal.h>

namespace FlareImGui
{
    static constexpr glm::vec4 Vec2Colors[] = { { 1.0f, 0.0f, 0.0f, 0.5f }, { 0.0, 1.0f, 0.0f, 0.5f } };
    static constexpr glm::vec4 Vec3Colors[] = { { 1.0f, 0.0f, 0.0f, 0.5f }, { 0.0, 1.0f, 0.0f, 0.5f }, { 0.0, 0.0f, 1.0f, 0.5f } };
    static constexpr glm::vec4 Vec4Colors[] = { { 1.0f, 0.0f, 0.0f, 0.5f }, { 0.0, 1.0f, 0.0f, 0.5f }, { 0.0, 0.0f, 1.0f, 0.5f }, { 1.0f, 1.0f, 0.0f, 0.5f } };

    static constexpr const char* Vec2Names[] = { "X", "Y" };
    static constexpr const char* Vec3Names[] = { "X", "Y", "Z" };
    static constexpr const char* Vec4Names[] = { "X", "Y", "Z", "W" };

    static bool DragVecScalar(const std::string_view& a_label, ImGuiDataType a_dataType, void* a_data, const char* const* a_strs, const glm::vec4* a_colors, uint32_t a_comps, float a_speed, float a_min, float a_max, const char* a_format, ImGuiSliderFlags a_flags)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            return false;
        }

        bool valueChanged = false;
        
        ImDrawList* drawList = window->DrawList;
        ImGuiStyle& style = ImGui::GetStyle();

        const size_t typeSize = ImGui::DataTypeGetInfo(a_dataType)->Size;

        {
            ImGui::BeginGroup();
            IDEFER(ImGui::EndGroup());

            ImGui::PushID(a_label.data());
            IDEFER(ImGui::PopID());

            ImGui::PushMultiItemsWidths(a_comps * 2, ImGui::CalcItemWidth());
            for (uint32_t i = 0; i < a_comps; ++i)
            {
                ImGui::PushID(i);
                IDEFER(ImGui::PopID());

                if (i > 0)
                {
                    ImGui::SameLine(0, style.ItemInnerSpacing.x * 2);
                }
                
                const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(a_colors[i].x, a_colors[i].y, a_colors[i].z, a_colors[i].w));
                
                const ImVec2 textSize = ImGui::CalcTextSize(a_strs[i]);
                const ImVec2 pos = ImGui::GetCursorScreenPos();
                const float offset = window->DC.CurrLineTextBaseOffset;
                constexpr float Padding = 4.0f;

                drawList->AddRectFilled(ImVec2(pos.x - Padding, pos.y + offset - Padding), ImVec2(pos.x + textSize.x + Padding, pos.y + textSize.y + offset + Padding), color, 4.0f);

                ImGui::Text("%s", a_strs[i]);
                ImGui::PopItemWidth();

                ImGui::SameLine(0, style.ItemInnerSpacing.x * 2);

                valueChanged |= ImGui::DragScalar("", a_dataType, (char*)a_data + i * typeSize, a_speed, &a_min, &a_max, a_format, a_flags);

                ImGui::PopItemWidth();
            }

            const char* labelEnd = ImGui::FindRenderedTextEnd(a_label.data());
            if (a_label.data() != labelEnd)
            {
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(labelEnd);
            }
        }

        return valueChanged;
    }

    bool DragVec2(const std::string_view& a_label, float a_val[2], float a_speed, float a_min, float a_max, const char* a_format, ImGuiSliderFlags a_flags)
    {
        return DragVecScalar(a_label, ImGuiDataType_Float, a_val, Vec2Names, Vec2Colors, 2, a_speed, a_min, a_max, a_format, a_flags);
    }
    bool DragVec3(const std::string_view& a_label, float a_val[3], float a_speed, float a_min, float a_max, const char* a_format, ImGuiSliderFlags a_flags)
    {
        return DragVecScalar(a_label, ImGuiDataType_Float, a_val, Vec3Names, Vec3Colors, 3, a_speed, a_min, a_max, a_format, a_flags);
    }
    bool DragVec4(const std::string_view& a_label, float a_val[4], float a_speed, float a_min, float a_max, const char* a_format, ImGuiSliderFlags a_flags)
    {
        return DragVecScalar(a_label, ImGuiDataType_Float, a_val, Vec4Names, Vec4Colors, 4, a_speed, a_min, a_max, a_format, a_flags);
    }

    void Label(const std::string_view& a_label)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const float width = ImGui::GetContentRegionAvail().x;
        const float itemWidth = width * 0.7f;

        const ImVec2 textSize = ImGui::CalcTextSize(a_label.data());
        const ImVec2 pos = ImGui::GetCursorScreenPos();

        ImGui::AlignTextToFramePadding();

        const float offset = window->DC.CurrLineTextBaseOffset;

        const ImRect textRect = ImRect(ImVec2(pos.x, pos.y + offset), ImVec2(pos.x + width - itemWidth, pos.y + textSize.y + offset));

        ImGui::ItemSize(textRect);
        const ImGuiID id = window->GetID(a_label.data());
        if (ImGui::ItemAdd(textRect, id))
        {
            ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), textRect.Min, textRect.Max, textRect.Max.x, textRect.Max.x, a_label.data(), nullptr, &textSize);
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
    }

    bool Image(const char* a_path, const ImVec2& a_size)
    {
        const Texture* tex = Datastore::GetTexture(a_path);
        if (tex != nullptr)
        {
            ImGui::Image(TexToImHandle(tex), a_size);

            return true;
        }

        return false;
    }
    bool Image(const std::string_view& a_path, const glm::vec2& a_size)
    {
        return Image(a_path.data(), { a_size.x, a_size.y });
    }

    bool ImageButton(const char* a_label, GLuint a_texture, const glm::vec2& a_size, bool a_background)
    {
        if (!a_background)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        }
        IDEFER(
        if (!a_background)
        {   
            ImGui::PopStyleColor(2);
        });

        bool ret = false;

        if (a_texture != -1)
        {
            ret = ImGui::ImageButton(a_label, (ImTextureID)(uintptr_t)a_texture, { a_size.x, a_size.y });
        }

        return ret;
    }
    bool ImageButton(const char* a_label, Texture* a_texture, const glm::vec2& a_size, bool a_background)
    {
        if (a_texture == nullptr)
        {
            return ImageButton(a_label, -1, a_size, a_background);
        }

        return ImageButton(a_label, a_texture->GetHandle(), a_size, a_background);
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
            ret = ImGui::ImageButton(a_label, TexToImHandle(tex), a_size);
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
            ret = ImGui::ImageButton(TexToImHandle(tex), a_size);
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