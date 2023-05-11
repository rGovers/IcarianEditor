#include "Workspace.h"

#include <imgui.h>

#include "Runtime/RuntimeManager.h"
#include "Windows/EditorWindow.h"

static Workspace* Instance = nullptr;

#define WORKSPACE_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define WORKSPACE_BINDING_FUNCTION_TABLE(F) \
    F(MonoString*, IcarianEditor, Workspace, GetCurrentScene, {  return mono_string_new_wrapper(Instance->GetCurrentScene().string().c_str()); }) \
    F(void, IcarianEditor, Workspace, SetCurrentScene, { char* str = mono_string_to_utf8(a_path); Instance->SetCurrentScene(str); mono_free(str); }, MonoString* a_path) \
    \
    F(uint32_t, IcarianEditor, Workspace, GetManipulationMode, { return (uint32_t)Instance->GetManipulationMode(); })

WORKSPACE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

Workspace::Workspace(RuntimeManager* a_runtime)
{
    m_runtime = a_runtime;

    m_manipulationMode = ManipulationMode_Translate;

    WORKSPACE_BINDING_FUNCTION_TABLE(WORKSPACE_RUNTIME_ATTACH);

    Instance = this;
}
Workspace::~Workspace()
{
    
}

void Workspace::AddEditorWindow(EditorWindow* a_window)
{
    m_editorWindows.emplace_back(a_window);
}
void Workspace::RemoveEditorWindow(EditorWindow* a_window)
{
    for (auto iter = m_editorWindows.begin(); iter != m_editorWindows.end(); ++iter)
    {
        if (*iter == a_window)
        {
            m_editorWindows.erase(iter);

            return;
        }
    }
}

void Workspace::SetScene(const std::filesystem::path& a_path, uint32_t a_size, const char* a_data)
{
    m_currentScene = a_path;

    for (EditorWindow* wind : m_editorWindows)
    {
        wind->Refresh();
    }
}

void Workspace::OpenDef(const std::filesystem::path& a_path, uint32_t a_size, const char* a_data)
{
    MonoDomain* editorDomain = m_runtime->GetEditorDomain();

    MonoString* pathString = mono_string_from_utf32((mono_unichar4*)a_path.u32string().c_str());

    void* args[] =
    {
        pathString
    };

    m_runtime->ExecFunction("IcarianEditor", "AssetProperties", ":PushDef(string)", args);
}
void Workspace::PushDef(const std::filesystem::path& a_path, uint32_t a_size, const char* a_data)
{
    const std::u32string str = a_path.u32string();

    ImGui::Text(a_path.string().c_str());
    ImGui::SetDragDropPayload("DefPath", str.c_str(), str.size() * sizeof(char32_t));
}