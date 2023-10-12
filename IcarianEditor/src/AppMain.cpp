#include "AppMain.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <string>
#include <sstream>

#include "AssetLibrary.h"
#include "Datastore.h"
#include "EditorConfig.h"
#include "FileHandler.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "FlareImGui.h"
#include "Gizmos.h"
#include "GUI.h"
#include "Modals/CreateProjectModal.h"
#include "Modals/EditorConfigModal.h"
#include "Modals/RuntimeModal.h"
#include "ProcessManager.h"
#include "ProfilerData.h"
#include "Project.h"
#include "RenderCommand.h"
#include "Runtime/RuntimeManager.h"
#include "Runtime/RuntimeStorage.h"
#include "Windows/AssetBrowserWindow.h"
#include "Windows/ConsoleWindow.h"
#include "Windows/ControlWindow.h"
#include "Windows/EditorWindow.h"
#include "Windows/GameWindow.h"
#include "Windows/HierarchyWindow.h"
#include "Windows/ProfilerWindow.h"
#include "Windows/PropertiesWindow.h"
#include "Windows/SceneDefsWindow.h"
#include "Workspace.h"

#include "Modals/ErrorModal.h"

static AppMain* Instance = nullptr;

static void GLAPIENTRY MessageCallback
( 
    GLenum a_source,
    GLenum a_type,
    GLuint a_id,
    GLenum a_severity,
    GLsizei a_length,
    const GLchar* a_message,
    const void* a_userParam 
)
{
    std::stringstream stream;

    stream << "GL CALLBACK: ";

    switch (a_severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    {
        stream << "HIGH SEVERITY ";

        break;
    }
    case GL_DEBUG_SEVERITY_MEDIUM:
    {
        stream << "MEDIUM SEVERITY ";

        break;
    }
    case GL_DEBUG_SEVERITY_LOW:
    {
        stream << "LOW SEVERITY ";

        break;
    }
    default:
    {
        stream << std::hex << a_severity << " ";

        break;
    }
    }

    stream << a_message;

    switch (a_type)
    {
    case GL_DEBUG_TYPE_ERROR:
    {
        Logger::Error(stream.str());

        break;
    }
    case GL_DEBUG_TYPE_PERFORMANCE:
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    {
        Logger::Warning(stream.str());

        break;
    }
    default:
    {
        break;
    }
    }
}

static void SetImguiStyle()
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    style.FrameRounding = 2.0f;
    style.WindowRounding = 2.0f;
    style.ChildRounding = 2.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    // ImVec4* colors = style.Colors;
    // colors[ImGuiCol_ChildBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.90f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.78f, 0.53f, 0.17f, 0.69f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.86f, 0.42f, 0.18f, 0.88f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.78f, 0.53f, 0.17f, 0.69f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.86f, 0.42f, 0.18f, 0.88f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.78f, 0.53f, 0.17f, 0.59f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.86f, 0.42f, 0.18f, 0.78f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.12f, 0.12f, 0.14f, 0.78f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.86f, 0.42f, 0.18f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.78f, 0.53f, 0.17f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.86f, 0.42f, 0.18f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.82f, 0.33f, 0.18f, 0.94f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.78f, 0.53f, 0.17f, 0.86f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.86f, 0.42f, 0.18f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.78f, 0.53f, 0.17f, 0.39f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.82f, 0.33f, 0.18f, 0.39f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.78f, 0.53f, 0.17f, 0.59f);
}

RUNTIME_FUNCTION(void, Modal, PushModal,
{
    char* title = mono_string_to_utf8(a_title);
    IDEFER(mono_free(title));

    Instance->DispatchRuntimeModal(title, a_size, a_index);
}, MonoString* a_title, glm::vec2 a_size, uint32_t a_index)
RUNTIME_FUNCTION(void, Modal, PushModalState,
{
    Instance->SetRuntimeModalState(a_index, a_state);
}, uint32_t a_index, uint32_t a_state)

AppMain::AppMain() : Application(1280, 720, "IcarianEditor")
{
    Instance = this;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
#endif

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    SetImguiStyle();

    ICARIAN_ASSERT_R(ImGui_ImplGlfw_InitForOpenGL(GetWindow(), true));
    ICARIAN_ASSERT_R(ImGui_ImplOpenGL3_Init("#version 130"));
    
    Datastore::Init();
    ProfilerData::Init();

    m_process = new ProcessManager();
    m_runtime = new RuntimeManager();

    EditorConfig::Init(m_runtime);

    m_assets = new AssetLibrary(m_runtime);
    m_rStorage = new RuntimeStorage(m_runtime, m_assets);

    m_workspace = new Workspace(m_runtime);

    m_project = new Project(this, m_assets, m_workspace);

    RenderCommand::Init(m_runtime, m_rStorage);
    Gizmos::Init(m_runtime);
    GUI::Init(m_runtime);

    FileHandler::Init(m_assets, m_workspace);
    
    m_windows.emplace_back(new ConsoleWindow());
    m_windows.emplace_back(new ControlWindow(this, m_process, m_runtime, m_workspace, m_project));
    m_windows.emplace_back(new EditorWindow(m_runtime, m_workspace));
    m_windows.emplace_back(new GameWindow(this, m_process));
    m_windows.emplace_back(new AssetBrowserWindow(this, m_project, m_assets));
    m_windows.emplace_back(new HierarchyWindow(m_runtime));
    m_windows.emplace_back(new PropertiesWindow(m_runtime));
    m_windows.emplace_back(new SceneDefsWindow(m_runtime));

    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    m_titleSet = 0.0;
    m_refresh = false;

    BIND_FUNCTION(m_runtime, IcarianEditor.Modals, Modal, PushModal);
    BIND_FUNCTION(m_runtime, IcarianEditor.Modals, Modal, PushModalState);
}
AppMain::~AppMain()
{
    glDeleteVertexArrays(1, &m_vao);

    delete m_project;

    delete m_assets;

    if (m_process->IsRunning())
    {
        m_process->Stop();
    }

    for (Window* wind : m_windows)
    {
        delete wind;
    }

    for (Modal* modal : m_modals)
    {
        delete modal;
    }

    delete m_process;
    delete m_runtime;
    delete m_rStorage;

    ProfilerData::Destroy();

    FileHandler::Destroy();
    Datastore::Destroy();

    RenderCommand::Destroy();
    Gizmos::Destroy();
    GUI::Destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    EditorConfig::Destroy();
}

static bool InBounds(const glm::vec2& a_point, const glm::vec2& a_min, const glm::vec2& a_max)
{
    return a_point.x > a_min.x && a_point.x < a_max.x && a_point.y > a_min.y && a_point.y < a_max.y;
}

void AppMain::Update(double a_delta, double a_time)
{    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    GLFWwindow* window = GetWindow();

    const ImGuiIO& io = ImGui::GetIO();
    const ImGuiStyle& style = ImGui::GetStyle();

    e_Cursors cursor = Cursor_Arrow;
    const glm::vec2 cursorPos = GetCursorPos();

    const uint32_t width = GetWidth();
    const uint32_t height = GetHeight();

    const float dpi = GetDPI();
    const float resizeSize = ResizeThreshold / dpi;

    const bool leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);

    // ImGui::ShowStyleEditor();

    const int focusState = glfwGetWindowAttrib(window, GLFW_FOCUSED);
    if (!m_focused && focusState)
    {
        m_refresh = true;
    }

    m_focused = (bool)focusState;

    const bool validProject = m_project->IsValidProject();

    if (validProject && m_project->ShouldRefresh())
    {
        m_refresh = true;

        m_project->SetRefresh(false);
    }

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    }

    m_process->Update();

    const int fps = (int)(1.0 / a_delta);

    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 10.0f));
        IDEFER(ImGui::PopStyleVar());
        const bool open = ImGui::BeginMainMenuBar();
        IDEFER(ImGui::EndMainMenuBar());
        if (open)
        {
            // TODO: Insert icon here when we have one
            FlareImGui::Image("Textures/Icons/Window_Close.png", { 32.0f, 32.0f });

            if (ImGui::BeginMenu("File"))
            {
                IDEFER(ImGui::EndMenu());

                if (ImGui::MenuItem("New Project"))
                {
                    m_project->New();
                }

                if (ImGui::MenuItem("Open Project", "Ctrl+O"))
                {
                    m_project->Open();
                }

                if (ImGui::MenuItem("Save Project", "Ctrl+S", nullptr, validProject))
                {
                    m_project->Save();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Build Project", nullptr, nullptr, validProject))
                {
                    m_project->Build();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                {
                    Close();
                }
            }

            if (ImGui::BeginMenu("Edit"))
            {
                IDEFER(ImGui::EndMenu());

                if (ImGui::MenuItem("Config"))
                {
                    m_modals.emplace_back(new EditorConfigModal());
                }
            }

            if (ImGui::BeginMenu("Windows"))
            {
                IDEFER(ImGui::EndMenu());

                if (ImGui::BeginMenu("Add"))
                {
                    IDEFER(ImGui::EndMenu());

                    if (ImGui::MenuItem("Control"))
                    {
                        m_windows.emplace_back(new ControlWindow(this, m_process, m_runtime, m_workspace, m_project));
                    }

                    if (ImGui::MenuItem("Editor"))
                    {
                        m_windows.emplace_back(new EditorWindow(m_runtime, m_workspace));
                    }

                    if (ImGui::MenuItem("Game"))
                    {
                        m_windows.emplace_back(new GameWindow(this, m_process));
                    }

                    if (ImGui::MenuItem("Asset Browser"))
                    {
                        m_windows.emplace_back(new AssetBrowserWindow(this, m_project, m_assets));
                    }                

                    if (ImGui::MenuItem("Console"))
                    {
                        m_windows.emplace_back(new ConsoleWindow());
                    }

                    if (ImGui::MenuItem("Properties"))
                    {
                        m_windows.emplace_back(new PropertiesWindow(m_runtime));
                    }

                    if (ImGui::MenuItem("Hierarchy"))
                    {
                        m_windows.emplace_back(new HierarchyWindow(m_runtime));
                    }

                    if (ImGui::MenuItem("Scene Definitions"))
                    {
                        m_windows.emplace_back(new SceneDefsWindow(m_runtime));
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Profiler"))
                    {
                        m_windows.emplace_back(new ProfilerWindow());
                    }
                }
            }

            float offset = 0.0f;

            const float width = ImGui::GetWindowWidth();

            offset += 24;
            ImGui::SetCursorPosX(width - offset);

            if (FlareImGui::ImageButton("X", "Textures/Icons/Window_Close.png", { 16.0f, 16.0f }, false))
            {
                Close();
            }

            offset += 24;
            ImGui::SetCursorPosX(width - offset);

            bool maximized = IsMaximized();

            if (FlareImGui::ImageSwitchButton("O", "Textures/Icons/Window_Restore.png", "Textures/Icons/Window_Maximize.png", &maximized, { 16.0f, 16.0f }))
            {
                Maximize(maximized);
            }

            offset += 24;
            ImGui::SetCursorPosX(width - offset);

            if (FlareImGui::ImageButton("_", "Textures/Icons/Window_Minimize.png", { 16.0f, 16.0f }, false))
            {
                Minimize();
            }

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImU32 headerColor = ImGui::GetColorU32(ImGuiCol_Header);

            if (!m_fpsText.empty())
            {
                const ImVec2 size = ImGui::CalcTextSize(m_fpsText.c_str());

                offset += size.x + 20.0f;

                const float xPos = width - offset;

                ImGui::SetCursorPosX(xPos);

                drawList->AddRectFilled(ImVec2(xPos - style.ItemSpacing.x, 0.0f), ImVec2(xPos + size.x + style.ItemSpacing.x, MenuBarSize), headerColor, 5.0f);

                ImGui::Text("%s", m_fpsText.c_str());
            }

            if (!m_engineFpsText.empty())
            {
                const ImVec2 size = ImGui::CalcTextSize(m_engineFpsText.c_str());

                offset += size.x + 20.0f;

                const float xPos = width - offset;

                ImGui::SetCursorPosX(xPos);

                drawList->AddRectFilled(ImVec2(xPos - style.ItemSpacing.x, 0.0f), ImVec2(xPos + size.x + style.ItemSpacing.x, MenuBarSize), headerColor, 5.0f);

                ImGui::Text("%s", m_engineFpsText.c_str());
            }

            if (!m_engineUpsText.empty())
            {
                const ImVec2 size = ImGui::CalcTextSize(m_engineUpsText.c_str());

                offset += size.x + 20.0f;

                const float xPos = width - offset;

                ImGui::SetCursorPosX(xPos);

                drawList->AddRectFilled(ImVec2(xPos - style.ItemSpacing.x, 0.0f), ImVec2(xPos + size.x + style.ItemSpacing.x, MenuBarSize), headerColor, 5.0f);

                ImGui::Text("%s", m_engineUpsText.c_str());
            }

            if (validProject)
            {
                const std::string name = m_project->GetName();

                const ImVec2 size = ImGui::CalcTextSize(name.c_str());

                offset += size.x + 20.0f;

                const float xPos = width - offset;

                ImGui::SetCursorPosX(xPos);

                drawList->AddRectFilled(ImVec2(xPos - style.ItemSpacing.x, 0.0f), ImVec2(xPos + size.x + style.ItemSpacing.x, MenuBarSize), headerColor, 5.0f);

                ImGui::Text("%s", name.c_str());
            }
        }
    }

    if (leftDown && !m_windowActions)
    {
        if (InBounds(cursorPos, glm::vec2(0.0f, -resizeSize), glm::vec2(width, resizeSize)))
        {
            m_windowActions |= 0b1 << TopResizeBit;
        }
        else if (InBounds(cursorPos, glm::vec2(0.0f, height - resizeSize), glm::vec2(width, height + resizeSize)))
        {
            m_windowActions |= 0b1 << BottomResizeBit;
        }
        else if (InBounds(cursorPos, glm::vec2(-resizeSize, 0.0f), glm::vec2(resizeSize, height)))
        {
            m_windowActions |= 0b1 << LeftResizeBit;
        }
        else if (InBounds(cursorPos, glm::vec2(width - resizeSize, 0.0f), glm::vec2(width + resizeSize, height)))
        {
            m_windowActions |= 0b1 << RightResizeBit;
        }
        else if (InBounds(cursorPos, glm::vec2(0.0f), glm::vec2(width, MenuBarSize)))
        {
            if (a_time - m_lastClick < DoubleClickThreshold)
            {
                Maximize(!IsMaximized());
            }
            else
            {
                m_windowActions |= 0b1 << MoveBit;
            }

            m_lastClick = a_time;
        }

        if (m_windowActions)
        {
            m_startWindowPos = GetWindowPos();
            m_startWindowSize = glm::vec2(width, height);
            m_startMousePos = GetMousePos();
        }
    }

    if (validProject)
    {
        for (auto iter = m_windows.begin(); iter != m_windows.end(); ++iter)
        {
            if (!(*iter)->Display(a_delta))
            {
                delete *iter;
                iter = m_windows.erase(iter);

                if (iter == m_windows.end())
                {
                    break;
                }
            }
        }
    }

    switch (m_windowActions) 
    {
    case 0b1 << MoveBit:
    {
        const glm::vec2 mousePos = GetMousePos();
        const glm::vec2 windowPos = GetWindowPos();

        const glm::vec2 wDelta = m_startMousePos - m_startWindowPos;
        const glm::vec2 cDelta = mousePos - m_startMousePos;

        SetWindowPos(m_startMousePos + cDelta - wDelta);

        break;
    }
    case 0b1 << TopResizeBit:
    {
        const glm::vec2 mousePos = GetMousePos();

        const glm::vec2 cDelta = mousePos - m_startMousePos;
        
        const float newHeight = m_startWindowSize.y - cDelta.y;
        const float newYPos = m_startWindowPos.y + cDelta.y;
        
        SetWindowSize((uint32_t)m_startWindowSize.x, (uint32_t)newHeight);
        SetWindowPos(glm::vec2(m_startWindowPos.x, newYPos));

        break;
    }
    case 0b1 << BottomResizeBit:
    {
        const glm::vec2 mousePos = GetMousePos();

        const glm::vec2 cDelta = mousePos - m_startMousePos;

        const float newHeight = m_startWindowSize.y + cDelta.y;

        SetWindowSize((uint32_t)m_startWindowSize.x, (uint32_t)newHeight);

        break;
    }
    case 0b1 << LeftResizeBit:
    {
        const glm::vec2 mousePos = GetMousePos();

        const glm::vec2 cDelta = mousePos - m_startMousePos;

        const float newWidth = m_startWindowSize.x - cDelta.x;
        const float newXPos = m_startWindowPos.x + cDelta.x;

        SetWindowSize((uint32_t)newWidth, (uint32_t)m_startWindowSize.y);
        SetWindowPos(glm::vec2(newXPos, m_startWindowPos.y));

        break;
    }
    case 0b1 << RightResizeBit:
    {
        const glm::vec2 mousePos = GetMousePos();

        const glm::vec2 cDelta = mousePos - m_startMousePos;

        const float newWidth = m_startWindowSize.x + cDelta.x;

        SetWindowSize((uint32_t)newWidth, (uint32_t)m_startWindowSize.y);

        break;
    }
    }

    if (m_windowActions && !leftDown)
    {
        m_windowActions = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    if (!m_modals.empty())
    {
        const uint32_t index = (uint32_t)m_modals.size() - 1;

        Modal* modal = m_modals[index];

        if (!modal->Display())
        {
            IDEFER(delete modal);

            m_modals.erase(m_modals.begin() + index);
        }
    }

    m_runtime->Update(a_delta);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
    {
        if (glfwGetKey(window, GLFW_KEY_S))
        {
            if (!(m_inputByte & (0b1 << SaveBit)))
            {
                m_inputByte |= 0b1 << SaveBit;
                m_project->Save();
            }
        }
        else
        {
            m_inputByte &= ~(0b1 << SaveBit);
        }

        if (glfwGetKey(window, GLFW_KEY_O))
        {
            if (!(m_inputByte & (0b1 << LoadBit)))
            {
                m_inputByte |= 0b1 << LoadBit;
                m_project->Open();
            }
        }
        else
        {
            m_inputByte &= ~(0b1 << LoadBit);
        }
    }

    if (m_refresh && validProject)
    {
        Logger::Message("Refreshing Project");
        m_refresh = false;

        const std::filesystem::path path = m_project->GetPath(); 
        const std::string pathStr = path.string();
        const std::string projectName = m_project->GetName();

        if (m_runtime->Build(pathStr, projectName))
        {
            m_runtime->Start(pathStr, projectName);
        }

        m_rStorage->Clear();

        m_assets->Refresh(path);
        m_assets->BuildDirectory(path / ".cache");

        for (Window* wind : m_windows)
        {
            wind->Refresh();
        }
    }

    m_titleSet += a_delta;

    if (m_titleSet >= 0.5f)
    {
        m_titleSet -= 0.5f;

        const double fps = 1.0 / a_delta;

        m_fpsText = "FPS " + std::to_string((uint32_t)fps);

        if (m_process->IsRunning())
        {
            const int ups = m_process->GetUPS();
            const int fps = m_process->GetFPS();

            m_engineFpsText = "Engine FPS " + std::to_string(fps);
            m_engineUpsText = "Engine UPS " + std::to_string(ups);
        }
        else
        {
            m_engineFpsText.clear();
            m_engineUpsText.clear();
        }
    }

    if (InBounds(cursorPos, glm::vec2(-resizeSize, 0.0f), glm::vec2(resizeSize, height)) || 
        InBounds(cursorPos, glm::vec2(width - resizeSize, 0.0f), glm::vec2(width + resizeSize, height)))
    {
        cursor = Cursor_HResize;
    }
    else if (InBounds(cursorPos, glm::vec2(0.0f, -resizeSize), glm::vec2(width, resizeSize)) ||
             InBounds(cursorPos, glm::vec2(0.0f, height - resizeSize), glm::vec2(width, height + resizeSize)))
    {
        cursor = Cursor_VResize;
    }
    
    if (m_windowActions & 0b1 << MoveBit)
    {
        cursor = Cursor_Move;
    }

    if (cursor != Cursor_Arrow)
    {
        SetCursor(cursor);
    }
}

bool AppMain::GetRuntimeModalState(uint32_t a_index)
{
    if (a_index >= m_runtimeModalState.size())
    {
        return false;
    }

    return m_runtimeModalState[a_index];
}
void AppMain::SetRuntimeModalState(uint32_t a_index, bool a_state)
{
    if (a_index >= m_runtimeModalState.size())
    {
        m_runtimeModalState.resize(a_index + 1);
    }

    m_runtimeModalState[a_index] = a_state;
}

void AppMain::PushModal(Modal* a_modal)
{
    m_modals.emplace_back(a_modal);
}

void AppMain::DispatchRuntimeModal(const std::string_view& a_title, const glm::vec2& a_size, uint32_t a_index)
{
    SetRuntimeModalState(a_index, true);

    m_modals.emplace_back(new RuntimeModal(this, m_runtime, a_index, a_title, a_size));
}