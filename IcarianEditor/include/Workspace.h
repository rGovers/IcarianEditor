#pragma once

#include <filesystem>
#include <functional>

class EditorWindow;
class RuntimeManager;

#include "Gizmos.h"

class Workspace
{
private:
    RuntimeManager*            m_runtime;

    std::filesystem::path      m_currentScene;

    std::filesystem::path      m_selectionPath;

    e_ManipulationMode         m_manipulationMode;

    std::vector<EditorWindow*> m_editorWindows;

protected:

public:
    Workspace(RuntimeManager* a_runtime);
    ~Workspace();

    inline std::filesystem::path GetCurrentScene() const
    {
        return m_currentScene;
    }
    inline void SetCurrentScene(const std::filesystem::path& a_path)
    {
        m_currentScene = a_path;
    }

    inline e_ManipulationMode GetManipulationMode() const
    {
        return m_manipulationMode;
    }
    inline void SetManipulationMode(e_ManipulationMode a_mode) 
    {
        m_manipulationMode = a_mode;
    }

    inline RuntimeManager* GetRuntime() const
    {
        return m_runtime;
    }

    void AddEditorWindow(EditorWindow* a_window);
    void RemoveEditorWindow(EditorWindow* a_window);

    void SetScene(const std::filesystem::path& a_path, uint32_t a_size, const char* a_data);

    void OpenDef(const std::filesystem::path& a_path, uint32_t a_size, const char* a_data);
    void PushDef(const std::filesystem::path& a_path, uint32_t a_size, const char* a_data);
};