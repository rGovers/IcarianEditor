#include "Windows/GameWindow.h"

#include <imgui.h>

#include "Flare/InputBindings.h"
#include "ProcessManager.h"

static constexpr ImGuiKey GameKeyTable[] = 
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

GameWindow::GameWindow(ProcessManager* a_processManager) : Window("Game")
{
    m_processManager = a_processManager;
}
GameWindow::~GameWindow()
{

}

void GameWindow::Update(double a_delta)
{
    const ImVec2 vMinIm = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMaxIm = ImGui::GetWindowContentRegionMax();
    const ImVec2 sizeIm = { vMaxIm.x - vMinIm.x, vMaxIm.y - vMinIm.y };

    m_processManager->SetSize((uint32_t)sizeIm.x, (uint32_t)sizeIm.y);

    if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered())
    {
        const ImVec2 mousePosIm = ImGui::GetMousePos();
        const ImVec2 winPosIm = ImGui::GetWindowPos();
        const ImGuiStyle& style = ImGui::GetStyle();

        const float titleBarSize = ImGui::GetFontSize() + style.FramePadding.y * 2;

        const glm::vec2 cPos = glm::vec2(mousePosIm.x - winPosIm.x, mousePosIm.y - (winPosIm.y + titleBarSize));

        m_processManager->PushCursorPos(cPos);

        unsigned char mouseState = 0;
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            mouseState |= 0b1 << FlareBase::MouseButton_Left;
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            mouseState |= 0b1 << FlareBase::MouseButton_Middle;
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            mouseState |= 0b1 << FlareBase::MouseButton_Right;
        }

        m_processManager->PushMouseState(mouseState);

        FlareBase::KeyboardState state;
        for (unsigned int i = 0; i < FlareBase::KeyCode_Last; ++i)
        {
            const ImGuiKey key = GameKeyTable[i];
            if (key != ImGuiKey_None)
            {
                if (ImGui::IsKeyDown(key))
                {
                    state.SetKey((FlareBase::e_KeyCode)i, true);
                }
            }
        }
        
        m_processManager->PushKeyboardState(state);
    }

    ImGui::Image((ImTextureID)m_processManager->GetImage(), sizeIm);
}