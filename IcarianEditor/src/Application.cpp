#include "Application.h"

#include <assert.h>

#include "Logger.h"

static void ErrorCallback(int a_error, const char* a_description)
{
    Logger::Error(a_description);
}

Application::Application(uint32_t a_width, uint32_t a_height, const std::string_view& a_title)
{
    m_width = a_width;
    m_height = a_height;

    if (!glfwInit())
    {
        assert(0);
    }

    glfwSetErrorCallback(ErrorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow((int)m_width, (int)m_height, a_title.data(), NULL, NULL);

    if (!m_window)
    {
        glfwTerminate();
        
        assert(0);
    }
    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        assert(0);
    }

    glfwSwapInterval(1);
}
Application::~Application()
{
    glfwDestroyWindow(m_window);

    glfwTerminate();
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