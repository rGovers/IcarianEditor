#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <string_view>

#include "Flare/InputBindings.h"

enum e_Cursors
{
    Cursor_Arrow,
    Cursor_Hand,
    Cursor_HResize,
    Cursor_VResize,
    Cursor_Move,
    Cursor_Last
};

class Application
{
private:
    bool        m_maximized;

    int         m_xPosState;
    int         m_yPosState;

    GLFWwindow* m_window;

    GLFWcursor* m_cursors[Cursor_Last];

protected:

public:
    Application(uint32_t a_width, uint32_t a_height, const std::string_view& a_title);
    virtual ~Application();

    void Run();

    bool IsFocused() const;

    bool IsMaximized() const;
    void Maximize(bool a_state);

    void Minimize() const;

    glm::vec2 GetWindowSize() const;
    void SetWindowSize(const glm::vec2& a_size);

    void SetCursorState(FlareBase::e_CursorState a_state);
    void SetCursor(e_Cursors a_cursor);

    glm::vec2 GetCursorPos() const;
    glm::vec2 GetMousePos() const;

    float GetDPI() const;

    glm::vec2 GetWindowPos() const;
    void SetWindowPos(const glm::vec2& a_pos);

    virtual void Update(double a_delta, double a_time) = 0;

    inline GLFWwindow* GetWindow() const
    {
        return m_window;
    }

    void SetTitle(const std::string_view& a_title) const;

    void Close() const;
};