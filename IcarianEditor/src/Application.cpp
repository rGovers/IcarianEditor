#include "Application.h"

#include <stb_image.h>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Logger.h"

static void ErrorCallback(int a_error, const char* a_description)
{
    Logger::Error(a_description);
}

// I dont not know if I need to throw GLFW or Windows under the bus for this one and have had enough of the WIN32 api to go digging
Application::Application(uint32_t a_width, uint32_t a_height, const std::string_view& a_title)
{
    // TODO: Do some glfw hacks relating to the titlebar
    m_maximized = false;

#ifndef WIN32
    // Wayland is weird so prefer run under XWayland for the editor for the time being
    // Will change when Wayland becomes more mature cough GNOME cough
    // Have had no issues under KDE
    // Major thing is Wayland up until recently had a fixed size input buffer and would cause crashes
    // Still dont have a way to set/get window position I prefer it have it and ignore it on select desktops then not at all
    // Wayland contributors still seem more interested in gaslighting and bikeshedding then fixing issues still so not even gonna bother
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    ICARIAN_ASSERT_R(glfwInit());

    glfwSetErrorCallback(ErrorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    m_window = glfwCreateWindow((int)a_width, (int)a_height, a_title.data(), NULL, NULL);
    if (m_window == NULL)
    {
        glfwTerminate();
        
        ICARIAN_ASSERT(0);
    }
    
    glfwSetWindowUserPointer(m_window, this);

    glfwMakeContextCurrent(m_window);

#ifdef WIN32
    // Center window on screen on Windows
    // For some reason Windows does not center the window by default
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    // Monitor can be null even if there is a monitor
    if (monitor != NULL)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (mode != NULL)
        {
            const int xPos = (mode->width - a_width) / 2;
            const int yPos = (mode->height - a_height) / 2;

            glfwSetWindowPos(m_window, xPos, yPos);
        }
    }
#endif

    GLFWimage icon;
    icon.pixels = stbi_load("Textures/Icons/Logo_White.png", &icon.width, &icon.height, NULL, 4);

    if (icon.pixels != nullptr)
    {
        IDEFER(stbi_image_free(icon.pixels));

        glfwSetWindowIcon(m_window, 1, &icon);
    }
    
    m_cursors[Cursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    m_cursors[Cursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    m_cursors[Cursor_HResize] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    m_cursors[Cursor_VResize] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    m_cursors[Cursor_Move] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);

    ICARIAN_ASSERT_R(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    glfwSwapInterval(1);
}
Application::~Application()
{
    for (uint32_t i = 0; i < Cursor_Last; ++i)
    {
        glfwDestroyCursor(m_cursors[i]);
    }

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void Application::SetCursorState(e_CursorState a_state)
{
    switch (a_state) 
    {
    case CursorState_Normal:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        break;
    }
    case CursorState_Hidden:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        break;
    }
    case CursorState_Locked:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        break;
    }
    }   
}
void Application::SetCursor(e_Cursors a_cursor)
{
    glfwSetCursor(m_window, m_cursors[a_cursor]);
}

bool Application::IsFocused() const
{
    return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != GLFW_FALSE;
}

bool Application::IsMaximized() const
{
    // Maximized state does not seem to be set correctly on Windows
    // Therefore we need to keep track of it ourselves
    return m_maximized;
}
void Application::Maximize(bool a_state)
{
    if (m_maximized == a_state)
    {
        return;
    }

    m_maximized = a_state;

    if (m_maximized)
    {
        glfwGetWindowPos(m_window, &m_xPosState, &m_yPosState);

#ifdef WIN32
        // Dodgy hack to get maximized window to not be off screen on Windows
        // Need to be done before maximizing window otherwise crashes
        GLFWmonitor* monitor = glfwGetWindowMonitor(m_window);

        // Window can be on no monitor apparently so check for that
        if (monitor != NULL)
        {
            int xPos;
            int yPos;
            glfwGetMonitorPos(monitor, &xPos, &yPos);

            glfwSetWindowPos(m_window, xPos, yPos);
        }
        else
        {
            // Fallback to 0, 0
            glfwSetWindowPos(m_window, 0, 0);
        }
#endif

        glfwMaximizeWindow(m_window);
    }
    else
    {
        glfwRestoreWindow(m_window);

#ifdef WIN32
        // Restore window position
        // Needs to be done after restoring window otherwise crashes or gets stuck in current position
        glfwSetWindowPos(m_window, m_xPosState, m_yPosState);
#endif
    }
}

void Application::Minimize() const
{
    glfwIconifyWindow(m_window);
}

void Application::SetWindowSize(const glm::vec2& a_size)
{
    int width = (int)a_size.x;
    int height = (int)a_size.y;

    if (width < 1)
    {
        width = 1;
    }
    if (height < 1)
    {
        height = 1;
    }

    glfwSetWindowSize(m_window, width, height);
}
glm::vec2 Application::GetWindowSize() const
{
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);

    return glm::vec2((float)width, (float)height);
}

glm::vec2 Application::GetCursorPos() const
{
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);

    return glm::vec2((float)x, (float)y);
}

float Application::GetDPI() const
{
    int x, y;
    int width, height;

    glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &x, &y);
    glfwGetFramebufferSize(m_window, &width, &height);

    return (float)x / (float)width;
}

glm::vec2 Application::GetMousePos() const
{
    const glm::vec2 windowPos = GetWindowPos();
    const glm::vec2 cursorPos = GetCursorPos();

    return windowPos + cursorPos;
}

glm::vec2 Application::GetWindowPos() const
{
    int x, y;
    glfwGetWindowPos(m_window, &x, &y);

    return glm::vec2((float)x, (float)y);
}
void Application::SetWindowPos(const glm::vec2& a_pos)
{
    // Safeguard against window getting stuck on Windows
    // Want to keep behavior consistent across platforms
    if (m_maximized)
    {
        return;
    }

    glfwSetWindowPos(m_window, (int)a_pos.x, (int)a_pos.y);
}

void Application::Run()
{
    const double startTime = glfwGetTime();
    double prevTime = startTime;

    while (!glfwWindowShouldClose(m_window))
    {
        // ImGui resets context so need to set
        glfwMakeContextCurrent(m_window);

        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        const double time = glfwGetTime();

        Update(time - prevTime, time - startTime);

        glfwSwapBuffers(m_window);

        prevTime = time;
    }
}

void Application::SetTitle(const std::string_view& a_title) const
{
    glfwSetWindowTitle(m_window, a_title.data());
}

void Application::Close() const
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}