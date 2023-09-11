#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <string_view>

#include "Flare/InputBindings.h"

class Application
{
private:
    GLFWwindow* m_window;
            
    uint32_t    m_width;
    uint32_t    m_height;

protected:

public:
    Application(uint32_t a_width, uint32_t a_height, const std::string_view& a_title);
    virtual ~Application();

    void Run();

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
    {
        return m_height;
    }

    void SetCursorState(FlareBase::e_CursorState a_state);

    glm::vec2 GetCursorPos() const;

    virtual void Update(double a_delta, double a_time) = 0;

    inline GLFWwindow* GetWindow() const
    {
        return m_window;
    }

    void SetTitle(const std::string_view& a_title) const;

    void Close() const;
};