#include "Windows/TimelineWindow.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <imgui.h>

#include "Core/IcarianDefer.h"
#include "Runtime/RuntimeManager.h"

TimelineWindow::TimelineWindow(RuntimeManager* a_runtime) : Window("Timeline")
{
    m_runtime = a_runtime;
}
TimelineWindow::~TimelineWindow()
{

}

void TimelineWindow::Update(double a_delta)
{
    const ImVec2 windowPos = ImGui::GetCursorScreenPos();
    const ImGuiStyle& style = ImGui::GetStyle();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImU32 buttonColorU32 = ImGui::GetColorU32(style.Colors[ImGuiCol_Button]);
    const ImU32 buttonHovColorU32 = ImGui::GetColorU32(style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 childBgColorU32 = ImGui::GetColorU32(style.Colors[ImGuiCol_ChildBg]);
    const ImU32 windowBgColorU32 = ImGui::GetColorU32(style.Colors[ImGuiCol_WindowBg]);
    constexpr ImU32 WhiteU32 = IM_COL32(255, 255, 255, 255);
    constexpr ImU32 BlackU32 = IM_COL32(0, 0, 0, 255);

    ImGui::SetCursorPosX(TimelineOffset);

    const int itemCount = 10;
    const int frameCount = 100;

    const float height = (itemCount + 1) * (16.0f + style.ItemSpacing.y);
    const float width = frameCount * 20.0f;

    if (ImGui::InvisibleButton("##Time", ImVec2(width, 16.0f)))
    {
        printf("neow\n");
    }

    const glm::vec2 scroll = glm::vec2(ImGui::GetScrollX(), ImGui::GetScrollY());

    const glm::vec2 windowOffset = glm::vec2(windowPos.x - scroll.x, windowPos.y - scroll.y);

    drawList->AddRectFilled(ImVec2(windowOffset.x + TimelineOffset - style.FramePadding.x, windowOffset.y + style.FramePadding.y), ImVec2(windowOffset.x + TimelineOffset + width + style.FramePadding.x, windowOffset.y + height), BlackU32, 4.0f);

    const int intervals = frameCount / 30;

    for (int i = 0; i < itemCount; ++i)
    {
        const ImVec2 buttonPos = ImGui::GetCursorScreenPos();
        if (ImGui::InvisibleButton("nyah", ImVec2(TimelineOffset - 20.0f, 16.0f)))
        {
            printf("nyah\n");
        }

        if (ImGui::IsItemHovered())
        {
            drawList->AddRectFilled(buttonPos, ImVec2(buttonPos.x + TimelineOffset - 20.0f, buttonPos.y + 16.0f), buttonHovColorU32);
        }

        drawList->AddText(ImVec2(buttonPos.x + 4.0f, buttonPos.y + 1.0f), WhiteU32, "test");
        drawList->AddRectFilled(ImVec2(windowOffset.x + TimelineOffset, buttonPos.y), ImVec2(windowOffset.x + TimelineOffset + width, buttonPos.y + 16.0f), childBgColorU32, 2.0f);

        for (int j = 0; j < frameCount; ++j)
        {
            const float x = TimelineOffset + (j * 20.0f);

            ImGui::SameLine();

            ImGui::SetCursorPosX(x + 10.0f);

            if (ImGui::InvisibleButton("##Frame", ImVec2(20.0f, 16.0f)))
            {
                printf("frame\n");
            }

            if (ImGui::IsItemHovered())
            {
                drawList->AddRectFilled(ImVec2(windowOffset.x + x, buttonPos.y), ImVec2(windowOffset.x + x + 20.0f, buttonPos.y + 16.0f), buttonHovColorU32);
            }

            drawList->AddCircleFilled(ImVec2(windowOffset.x + x + 10.0f, buttonPos.y + 8.0f), 4.0f, WhiteU32);
            drawList->AddCircle(ImVec2(windowOffset.x + x + 10.0f, buttonPos.y + 8.0f), 4.0f, buttonHovColorU32, 0, 2.0f);

            if (j % 30 == 0)
            {
                drawList->AddLine(ImVec2(windowOffset.x + x, buttonPos.y), ImVec2(windowOffset.x + x, buttonPos.y + 16.0f), WhiteU32, 2.0f);
            }
        }
    }

    for (uint32_t i = 0; i < frameCount; ++i)
    {
        const float x = TimelineOffset + (i * 20.0f);

        drawList->AddLine(ImVec2(windowOffset.x + x, windowOffset.y + style.FramePadding.y), ImVec2(windowOffset.x + x, windowOffset.y + height), windowBgColorU32);
    }

    for (uint32_t i = 0; i <= intervals; ++i)
    {
        const float x = TimelineOffset + (i * 30 * 20);

        const std::string text = std::to_string(i) + "s";

        drawList->AddText(ImVec2(windowOffset.x + x + 4.0f, windowOffset.y + style.FramePadding.y), WhiteU32, text.c_str());
    }
}