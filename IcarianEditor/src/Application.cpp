#include "Application.h"

#include "Flare/IcarianAssert.h"
#include "Logger.h"

static void ErrorCallback(int a_error, const char* a_description)
{
    Logger::Error(a_description);
}

Application::Application(uint32_t a_width, uint32_t a_height, const std::string_view& a_title)
{
    ICARIAN_ASSERT_R(glfwInit());

    glfwSetErrorCallback(ErrorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    m_window = glfwCreateWindow((int)a_width, (int)a_height, a_title.data(), NULL, NULL);

    glfwSetWindowUserPointer(m_window, this);

    if (!m_window)
    {
        glfwTerminate();
        
        ICARIAN_ASSERT(0);
    }
    glfwMakeContextCurrent(m_window);

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

void Application::SetCursorState(FlareBase::e_CursorState a_state)
{
    switch (a_state) 
    {
    case FlareBase::CursorState_Normal:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        break;
    }
    case FlareBase::CursorState_Hidden:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        break;
    }
    case FlareBase::CursorState_Locked:
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
    return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED) != GLFW_FALSE;
}
void Application::Maximize(bool a_state)
{
    if (a_state)
    {
        glfwMaximizeWindow(m_window);
    }
    else
    {
        glfwRestoreWindow(m_window);
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