#include "EditorConfig.h"

#include <filesystem>
#include <tinyxml2.h>

#include "Runtime/RuntimeManager.h"

static EditorConfig* Instance = nullptr;

#define EDITORCONFIG_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) BIND_FUNCTION(a_runtime, namespace, klass, name)

#define EDITORCONFIG_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEditor, EditorConfig, GetUseDegrees, { return (uint32_t)EditorConfig::GetUseDegrees(); }) \

EDITORCONFIG_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

EditorConfig::EditorConfig()
{
    
}
EditorConfig::~EditorConfig()
{
    Serialize();
}

void EditorConfig::Deserialize()
{
    if (std::filesystem::exists(ConfigFile))
    {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(ConfigFile) == tinyxml2::XML_SUCCESS)
        {
            tinyxml2::XMLElement* root = doc.FirstChildElement("Config");
            if (root != nullptr)
            {
                for (const tinyxml2::XMLElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
                {
                    const char* name = element->Name();

                    if (strcmp(name, "UseDegrees") == 0)
                    {
                        Instance->m_useDegrees = element->BoolText();
                    }
                    else if (strcmp(name, "BackgroundColor") == 0)
                    {
                        for (const tinyxml2::XMLElement* colorElement = element->FirstChildElement(); colorElement != nullptr; colorElement = colorElement->NextSiblingElement())
                        {
                            const char* colorName = colorElement->Name();

                            if (strcmp(colorName, "R") == 0)
                            {
                                Instance->m_backgroundColor.r = colorElement->FloatText();
                            }
                            else if (strcmp(colorName, "G") == 0)
                            {
                                Instance->m_backgroundColor.g = colorElement->FloatText();
                            }
                            else if (strcmp(colorName, "B") == 0)
                            {
                                Instance->m_backgroundColor.b = colorElement->FloatText();
                            }
                            else if (strcmp(colorName, "A") == 0)
                            {
                                Instance->m_backgroundColor.a = colorElement->FloatText();
                            }
                        }
                    }
                    else if (strcmp(name, "CodeEditor") == 0)
                    {
                        const char* codeEditor = element->GetText();
                        if (strcmp(codeEditor, "Default") == 0)
                        {
                            Instance->m_codeEditor = CodeEditor_Default;
                        }
                        else if (strcmp(codeEditor, "VisualStudioCode") == 0)
                        {
                            Instance->m_codeEditor = CodeEditor_VisualStudioCode;
                        }
                        else if (strcmp(codeEditor, "VisualStudio") == 0)
                        {
                            Instance->m_codeEditor = CodeEditor_VisualStudio;
                        }
                    }
                }
            }
        }
    }
}
void EditorConfig::Serialize()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);

    tinyxml2::XMLElement* root = doc.NewElement("Config");
    doc.InsertEndChild(root);

    tinyxml2::XMLElement* useDegrees = doc.NewElement("UseDegrees");
    useDegrees->SetText(Instance->m_useDegrees);
    root->InsertEndChild(useDegrees);

    tinyxml2::XMLElement* backgroundColor = doc.NewElement("BackgroundColor");
    root->InsertEndChild(backgroundColor);
    {
        tinyxml2::XMLElement* r = doc.NewElement("R");
        r->SetText(Instance->m_backgroundColor.r);
        backgroundColor->InsertEndChild(r);

        tinyxml2::XMLElement* g = doc.NewElement("G");
        g->SetText(Instance->m_backgroundColor.g);
        backgroundColor->InsertEndChild(g);

        tinyxml2::XMLElement* b = doc.NewElement("B");
        b->SetText(Instance->m_backgroundColor.b);
        backgroundColor->InsertEndChild(b);

        tinyxml2::XMLElement* a = doc.NewElement("A");
        a->SetText(Instance->m_backgroundColor.a);
        backgroundColor->InsertEndChild(a);
    }

    tinyxml2::XMLElement* codeEditor = doc.NewElement("CodeEditor");
    switch (Instance->m_codeEditor)
    {
    case CodeEditor_VisualStudio:
    {
        codeEditor->SetText("VisualStudio");

        break;
    }
    case CodeEditor_VisualStudioCode:
    {
        codeEditor->SetText("VisualStudioCode");

        break;
    }
    default:
    {
        codeEditor->SetText("Default");

        break;
    }
    }

    doc.SaveFile(ConfigFile);
}

void EditorConfig::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        Instance = new EditorConfig();
        Deserialize();

        EDITORCONFIG_BINDING_FUNCTION_TABLE(EDITORCONFIG_RUNTIME_ATTACH);
    }
}
void EditorConfig::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

bool EditorConfig::GetUseDegrees()
{
    return Instance->m_useDegrees;
}
void EditorConfig::SetUseDegrees(bool useDegrees)
{
    Instance->m_useDegrees = useDegrees;
}

glm::vec4 EditorConfig::GetBackgroundColor()
{
    return Instance->m_backgroundColor;
}
void EditorConfig::SetBackgroundColor(const glm::vec4& backgroundColor)
{
    Instance->m_backgroundColor = backgroundColor;
}

e_CodeEditor EditorConfig::GetCodeEditor()
{
    return Instance->m_codeEditor;
}
void EditorConfig::SetCodeEditor(e_CodeEditor codeEditor)
{
    Instance->m_codeEditor = codeEditor;
}