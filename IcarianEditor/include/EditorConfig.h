#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>

class RuntimeManager;

enum e_CodeEditor : uint32_t
{
    CodeEditor_Default,
    CodeEditor_VisualStudio,
    CodeEditor_VisualStudioCode,
    CodeEditor_End
};

class EditorConfig
{
private:
    static constexpr char ConfigFile[] = "editorConfig.xml";

    bool         m_useDegrees = false;

    glm::vec4    m_backgroundColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);

    e_CodeEditor m_codeEditor = CodeEditor_Default;

    EditorConfig();

protected:

public:
    ~EditorConfig();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static bool GetUseDegrees();
    static void SetUseDegrees(bool useDegrees);

    static glm::vec4 GetBackgroundColor();
    static void SetBackgroundColor(const glm::vec4& backgroundColor);

    static e_CodeEditor GetCodeEditor();
    static void SetCodeEditor(e_CodeEditor codeEditor);

    static void Deserialize();
    static void Serialize();
};