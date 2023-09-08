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

    style.FrameRounding = 4.0f;
    style.WindowRounding = 6.0f;
    style.ChildRounding = 8.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_ChildBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
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
    m_windows.emplace_back(new GameWindow(m_process));
    m_windows.emplace_back(new AssetBrowserWindow(this, m_project, m_assets));
    m_windows.emplace_back(new HierarchyWindow(m_runtime));
    m_windows.emplace_back(new PropertiesWindow(m_runtime));

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

void AppMain::Update(double a_delta, double a_time)
{    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    GLFWwindow* window = GetWindow();

    const ImGuiIO& io = ImGui::GetIO();

    std::string title = "IcarianEditor";

    // ImGui::ShowStyleEditor();

    const int focusState = glfwGetWindowAttrib(window, GLFW_FOCUSED);
    if (!m_focused && focusState)
    {
        m_refresh = true;
    }

    m_focused = (bool)focusState;

    const bool validProject = m_project->IsValidProject();

    if (validProject)
    {
        title += " [" + std::string(m_project->GetName()) + "]";

        if (m_project->ShouldRefresh())
        {
            m_refresh = true;

            m_project->SetRefresh(false);
        }
    }

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    }

    m_process->Update();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            const bool validProject = m_project->IsValidProject();

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

            ImGui::EndMenu();
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
            if (ImGui::BeginMenu("Add"))
            {
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
                    m_windows.emplace_back(new GameWindow(m_process));
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

                ImGui::Separator();

                if (ImGui::MenuItem("Profiler"))
                {
                    m_windows.emplace_back(new ProfilerWindow());
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();

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
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, (GLsizei)GetWidth(), (GLsizei)GetHeight());

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

    const int fps = (int)(1.0 / a_delta);

    title += " EditorFPS: " + std::to_string(fps);

    if (m_process->IsRunning())
    {
        const int engineFPS = (int)m_process->GetFPS();
        const int engineUPS = (int)m_process->GetUPS();

        title += " EngineUPS: " + std::to_string(engineUPS);
        title += " EngineFPS: " + std::to_string(engineFPS);
    }

    m_titleSet -= a_delta;

    if (m_titleSet <= 0.0)
    {
        SetTitle(title);

        m_titleSet += 0.1;
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