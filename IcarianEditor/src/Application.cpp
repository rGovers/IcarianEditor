#include "Application.h"

#include "Flare/IcarianAssert.h"
#include "Logger.h"

static void ErrorCallback(int a_error, const char* a_description)
{
    Logger::Error(a_description);
}

Application::Application(uint32_t a_width, uint32_t a_height, const std::string_view& a_title)
{
    m_width = a_width;
    m_height = a_height;

    ICARIAN_ASSERT_R(glfwInit());

    glfwSetErrorCallback(ErrorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow((int)m_width, (int)m_height, a_title.data(), NULL, NULL);

    if (!m_window)
    {
        glfwTerminate();
        
        ICARIAN_ASSERT(0);
    }
    glfwMakeContextCurrent(m_window);

    ICARIAN_ASSERT_R(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    glfwSwapInterval(1);
}
Application::~Application()
{
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

glm::vec2 Application::GetCursorPos() const
{
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);

    return glm::vec2(x, y);
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