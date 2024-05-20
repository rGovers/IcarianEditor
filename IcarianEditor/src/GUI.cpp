#include "GUI.h"

#include <glm/glm.hpp>
#include <string>

#include "AppMain.h"
#include "Core/IcarianDefer.h"
#include "Datastore.h"
#include "FlareImGui.h"
#include "Modals/GetAssetModal.h"
#include "Runtime/RuntimeManager.h"
#include "Texture.h"

static GUI* Instance = nullptr;

static constexpr uint32_t BufferSize = 4096;

struct IDStack
{
    IDStack(const std::string_view& a_id)
    {
        Instance->PushID(a_id);

        ImGui::PushID(a_id.data());
    }
    ~IDStack()
    {
        Instance->PopID();

        ImGui::PopID();
    }
};

#define STACK_ID(str) const IDStack idStackTVal = IDStack(str)
#define STACK_G_ID(str) STACK_ID(Instance->GetID() + (str))

RUNTIME_FUNCTION(uint32_t, GUI, GetButton, 
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    STACK_G_ID(str);
    return (uint32_t)ImGui::Button(str);
}, MonoString* a_str)

RUNTIME_FUNCTION(uint32_t, GUI, GetCheckbox, 
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;

    bool b = (bool)*a_value;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    if (ImGui::Checkbox(("##V_" + str).c_str(), &b))
    {
        *a_value = (uint32_t)b;

        return 1;
    }

    return 0;
}, MonoString* a_str, uint32_t* a_value)

// Work around for MSVC formating macro arguments
static MonoString* M_GUI_GetDef(MonoString* a_str, MonoString* a_preview, uint32_t* a_dispatchModal)
{
    *a_dispatchModal = 0;

    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));

    char* preview = mono_string_to_utf8(a_preview);
    IDEFER(mono_free(preview));

    const std::string str = mStr;

    STACK_G_ID(str);

    FlareImGui::Label(str);

    if (ImGui::Button(preview, { Instance->GetFieldWidth(), 0 }))
    {
        *a_dispatchModal = 1;
    }

    if (ImGui::BeginDragDropTarget())
    {
        IDEFER(ImGui::EndDragDropTarget());

        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DefPath");
        if (payload != nullptr)
        {
            return mono_string_from_utf32((mono_unichar4*)payload->Data);
        }
    }

    return NULL;
}
RUNTIME_FUNCTION(MonoString*, GUI, GetDef,
{
    return M_GUI_GetDef(a_str, a_preview, a_dispatchModal);
}, MonoString* a_str, MonoString* a_preview, uint32_t* a_dispatchModal)

RUNTIME_FUNCTION(uint32_t, GUI, GetInt,
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;
    
    STACK_G_ID(str);
    FlareImGui::Label(str);

    return (uint32_t)ImGui::InputInt(("##V_" + str).c_str(), (int*)a_value);
}, MonoString* a_str, int32_t* a_value)
RUNTIME_FUNCTION(uint32_t, GUI, GetUInt, 
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    return (uint32_t)ImGui::InputInt(("##V_" + str).c_str(), (int*)a_value);
}, MonoString* a_str, uint32_t* a_value)

RUNTIME_FUNCTION(uint32_t, GUI, GetBitField, 
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;

    STACK_G_ID(str);
    ImGui::Text("%s", str.c_str());

    ImGuiStyle& style = ImGui::GetStyle();

    bool ret = false;
    for (uint32_t i = 0; i < a_bitCount; ++i)
    {
        STACK_G_ID(str + "[" + std::to_string(i) + "]");

        if (i % 8 != 0)
        {
            ImGui::SameLine();
        }

        const bool selected = (*a_value & (1 << i)) != 0;
        if (selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
        }
        else 
        {
            ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_Button]);
        }
        IDEFER(ImGui::PopStyleColor());

        if (ImGui::Button(("##V_" + str + "[" + std::to_string(i) + "]").c_str(), { 16.0f, 16.0f }))
        {
            if (selected)
            {
                *a_value &= ~(1 << i);
            }
            else
            {
                *a_value |= (1 << i);
            }

            ret = true;
        }
    }

    return (uint32_t)ret;
}, MonoString* a_str, uint32_t* a_value, uint32_t a_bitCount)

RUNTIME_FUNCTION(uint32_t, GUI, GetFloat, 
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    return (uint32_t)ImGui::DragFloat(("##V_" + str).c_str(), a_value);
}, MonoString* a_str, float* a_value)

RUNTIME_FUNCTION(uint32_t, GUI, GetVec2, 
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    return (uint32_t)FlareImGui::DragVec2(("##V_" + str).c_str(), (float*)a_value);
}, MonoString* a_str, glm::vec2* a_value)
RUNTIME_FUNCTION(uint32_t, GUI, GetVec3, 
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    return (uint32_t)FlareImGui::DragVec3(("##V_" + str).c_str(), (float*)a_value);
}, MonoString* a_str, glm::vec3* a_value)
RUNTIME_FUNCTION(uint32_t, GUI, GetVec4,
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    return (uint32_t)FlareImGui::DragVec4(("##V_" + str).c_str(), (float*)a_value);
}, MonoString* a_str, glm::vec4* a_value)

RUNTIME_FUNCTION(uint32_t, GUI, GetColor,
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    const std::string str = mStr;
    
    STACK_G_ID(str);
    FlareImGui::Label(str);

    return (uint32_t)ImGui::ColorEdit4(("##V_" + str).c_str(), (float*)a_value);
}, MonoString* a_str, glm::vec4* a_value)

// Workaround for MSVC
static MonoString* M_GUI_GetString(MonoString* a_str, MonoString* a_value)
{
    char buffer[BufferSize];

    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    const std::string str = mStr;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    strncpy(buffer, value, BufferSize - 1);
    if (ImGui::InputText(("##V_" + str).c_str(), buffer, BufferSize))
    {
        return mono_string_new(mono_domain_get(), buffer);
    }

    return NULL;
}
RUNTIME_FUNCTION(MonoString*, GUI, GetString,
{
    return M_GUI_GetString(a_str, a_value);
}, MonoString* a_str, MonoString* a_value)

RUNTIME_FUNCTION(MonoString*, GUI, GetPathString, 
{
    char buffer[BufferSize];

    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    const std::string str = mStr;

    STACK_G_ID(str);
    FlareImGui::Label(str);

    ImGui::BeginGroup();
    IDEFER(ImGui::EndGroup());

    if (FlareImGui::ImageButton(("##F_" + str).c_str(), "Textures/WindowIcons/WindowIcon_AssetBrowser.png", { 16.0f, 16.0f }, false))
    {
        const uint32_t extensionCount = (uint32_t)mono_array_length(a_extensions);

        char** extensions = new char*[extensionCount];
        memset(extensions, 0, extensionCount * sizeof(char*));
        IDEFER(
        {
            for (uint32_t i = 0; i < extensionCount; ++i)
            {
                if (extensions[i] != NULL)
                {
                    mono_free(extensions[i]);
                }
            }

            delete[] extensions;
        });

        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            MonoString* str = mono_array_get(a_extensions, MonoString*, i);
            if (str != NULL)
            {
                extensions[i] = mono_string_to_utf8(str);
            }
        }

        Instance->OpenAssetPathModal(extensions, extensionCount);
    }

    ImGui::SameLine();

    strncpy(buffer, value, BufferSize - 1);
    if (ImGui::InputText(("##V_" + str).c_str(), buffer, BufferSize))
    {
        return mono_string_new(mono_domain_get(), buffer);
    }

    const std::string pStr = Instance->GetPathString();
    if (!pStr.empty())
    {
        return mono_string_new(mono_domain_get(), pStr.c_str());
    }

    return NULL;
}, MonoString* a_str, MonoString* a_value, MonoArray* a_extensions)

// MSVC workaround
static uint32_t M_GUI_GetStringList(MonoString* a_str, MonoArray* a_list, int32_t* a_selected)
{
    char* mStr = mono_string_to_utf8(a_str);
    IDEFER(mono_free(mStr));

    const std::string str = mStr;

    const uintptr_t size = mono_array_length(a_list);
    if (*a_selected >= size)
    {
        *a_selected = 0;
    }

    char* selectedStr = mono_string_to_utf8(mono_array_get(a_list, MonoString*, *a_selected));
    IDEFER(mono_free(selectedStr));

    STACK_G_ID(str);
    FlareImGui::Label(str);

    if (ImGui::BeginCombo(("##V_" + str).c_str(), selectedStr))
    {
        IDEFER(ImGui::EndCombo());
        
        static char Buffer[4096] = { 0 };
        ImGui::InputText(("Search##VS_" + str).c_str(), Buffer, sizeof(Buffer) - 1);

        for (uint32_t i = 0; i < size; ++i)
        {
            char* selectableStr = mono_string_to_utf8(mono_array_get(a_list, MonoString*, i));
            IDEFER(mono_free(selectableStr));

            if (Buffer[0] != 0 && strstr(selectableStr, Buffer) == NULL)
            {
                continue;
            }

            const bool selected = i == *a_selected;

            STACK_G_ID(std::string(str) + "[" + std::to_string(i) + "]");
            if (ImGui::Selectable(selectableStr, selected))
            {
                *a_selected = i;

                return 1;
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
    }

    return 0;
}
RUNTIME_FUNCTION(uint32_t, GUI, GetStringList,
{
    return M_GUI_GetStringList(a_str, a_list, a_selected);
}, MonoString* a_str, MonoArray* a_list, int32_t* a_selected)

// MSVC workaround
static uint32_t M_GUI_ResetButton(MonoString* a_str)
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    IDEFER(ImGui::SameLine());

    STACK_G_ID(str);

    return (uint32_t)FlareImGui::ImageButton(str, "Textures/Icons/Icon_Reset.png", glm::vec2(16.0f));
}
RUNTIME_FUNCTION(uint32_t, GUI, ResetButton,
{
    return M_GUI_ResetButton(a_str);
}, MonoString* a_str)

RUNTIME_FUNCTION(void, GUI, NIndent,
{
    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
})
RUNTIME_FUNCTION(void, GUI, Indent,
{
    ImGui::Indent();
})
RUNTIME_FUNCTION(void, GUI, Unindent,
{
    ImGui::Unindent();
})

// MSVC workaround
static void M_GUI_Tooltip(MonoString* a_title, MonoString* a_str)
{
    char* title = mono_string_to_utf8(a_title);
    IDEFER(mono_free(title));
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        IDEFER(ImGui::EndTooltip());

        if (title != nullptr)
        {
            ImGui::Text("%s", title);

            if (str != nullptr)
            {
                ImGui::Separator();
            }
        }

        if (str != nullptr)
        {
            ImGui::Text("%s", str);
        }
    }
}
RUNTIME_FUNCTION(void, GUI, Tooltip,
{
    M_GUI_Tooltip(a_title, a_str);
}, MonoString* a_title, MonoString* a_str)

RUNTIME_FUNCTION(uint32_t, GUI, ShowStructView,
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    STACK_G_ID(str);
    return (uint32_t)ImGui::CollapsingHeader(str);
}, MonoString* a_str)
RUNTIME_FUNCTION(uint32_t, GUI, ShowArrayView,
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    {
        STACK_G_ID(std::string(str) + "_Add");
        *a_addValue = (uint32_t)ImGui::Button("+");
    }
    
    ImGui::SameLine();
    
    STACK_G_ID(str);
    return (uint32_t)ImGui::CollapsingHeader(str);
}, MonoString* a_str, uint32_t* a_addValue)

RUNTIME_FUNCTION(uint32_t, GUI, ShowTexture, 
{
    char* path = mono_string_to_utf8(a_path);
    IDEFER(mono_free(path));

    const Texture* texture = Datastore::GetTexture(path);
    if (texture != nullptr)
    {
        ImGui::Image(TexToImHandle(texture), ImVec2(a_size.x, a_size.y));

        return 1;
    }

    return 0;
}, MonoString* a_path, glm::vec2 a_size)

RUNTIME_FUNCTION(void, GUI, PushID, 
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    Instance->PushID(str);
}, MonoString* a_str)
RUNTIME_FUNCTION(void, GUI, PopID,
{
    Instance->PopID();
})
RUNTIME_FUNCTION(MonoString*, GUI, GetCurrentID,
{
    return mono_string_new(mono_domain_get(), Instance->GetID().c_str());
})

RUNTIME_FUNCTION(void, GUI, Label, 
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    STACK_G_ID(str);
    ImGui::Text("%s", str);
}, MonoString* a_str)
RUNTIME_FUNCTION(uint32_t, GUI, GetSelectable,
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    STACK_G_ID(str);
    return (uint32_t)ImGui::Selectable(str);
}, MonoString* a_str)

RUNTIME_FUNCTION(uint32_t, GUI, GetContextPopup, 
{
    return (uint32_t)ImGui::BeginPopupContextItem();
})
RUNTIME_FUNCTION(uint32_t, GUI, GetContextPopupWindow, 
{
    return (uint32_t)ImGui::BeginPopupContextWindow();
})
RUNTIME_FUNCTION(void, GUI, EndPopup,
{
    ImGui::EndPopup();
})

RUNTIME_FUNCTION(uint32_t, GUI, GetMenu, 
{
    char* str = mono_string_to_utf8(a_label);
    IDEFER(mono_free(str));

    return (uint32_t)ImGui::BeginMenu(str);
}, MonoString* a_label)
RUNTIME_FUNCTION(void, GUI, EndMenu,
{
    ImGui::EndMenu();
})

RUNTIME_FUNCTION(uint32_t, GUI, GetMenuItem, 
{
    char* str = mono_string_to_utf8(a_label);
    IDEFER(mono_free(str));

    return (uint32_t)ImGui::MenuItem(str);
}, MonoString* a_label)

RUNTIME_FUNCTION(uint32_t, GUI, NodeI,
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    STACK_G_ID(str);
    return (uint32_t)ImGui::TreeNode(str);
}, MonoString* a_str)
RUNTIME_FUNCTION(void, GUI, PopNode,
{
    ImGui::TreePop();
})

RUNTIME_FUNCTION(void, GUI, SameLine, 
{
    ImGui::SameLine();
})
RUNTIME_FUNCTION(void, GUI, Separator, 
{
    ImGui::Separator();
})

RUNTIME_FUNCTION(uint32_t, GUI, GetBeginChild, 
{
    char* str = mono_string_to_utf8(a_str);
    IDEFER(mono_free(str));

    return (uint32_t)ImGui::BeginChild(str, { a_size.x, a_size.y });
}, MonoString* a_str, glm::vec2 a_size)
RUNTIME_FUNCTION(void, GUI, EndChild, 
{
    ImGui::EndChild();
})

RUNTIME_FUNCTION(uint32_t, GUI, GetShiftModifier, 
{
    return (uint32_t)(ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift));
})
RUNTIME_FUNCTION(uint32_t, GUI, GetCtrlModifier,
{
    return (uint32_t)(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl));
})

GUI::GUI(AppMain* a_app, RuntimeManager* a_runtime, AssetLibrary* a_assets)
{
    m_app = a_app;
    m_assets = a_assets;
    m_runtime = a_runtime;
}
GUI::~GUI()
{
    
}

void GUI::Init(AppMain* a_app, RuntimeManager* a_runtime, AssetLibrary* a_assets)
{
    if (Instance == nullptr)
    {
        Instance = new GUI(a_app, a_runtime, a_assets);
        
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetButton);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetCheckbox);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetDef);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetInt);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetUInt);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetBitField);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetFloat);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetVec2);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetVec3);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetVec4);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetColor);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetString);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetPathString);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetStringList);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, ResetButton);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, NIndent);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Indent);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Unindent);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, ShowStructView);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, ShowArrayView);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, ShowTexture);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Tooltip);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, PushID);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, PopID);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetCurrentID);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Label);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetSelectable);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetContextPopup);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetContextPopupWindow);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, EndPopup);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetMenu);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, EndMenu);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetMenuItem);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, NodeI);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, PopNode);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, SameLine);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Separator);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetShiftModifier);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetCtrlModifier);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetBeginChild);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, EndChild);
    }
}
void GUI::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

class PathAssetModalData : public GetAssetModalData
{
private:
    GUI*        m_gui;
    std::string m_id;

protected:

public:
    PathAssetModalData(GUI* a_gui, const std::string_view& a_id)
    {
        m_gui = a_gui;
        m_id = a_id;
    }

    virtual void Confirm(const std::filesystem::path& a_path)
    {
        const PathString str =
        {
            .IDStr = m_id,
            .Path = a_path.string()
        };

        m_gui->PushPathString(str);
    }
};

void GUI::OpenAssetPathModal(char* const* a_extensions, uint32_t a_extensionCount)
{
    PathAssetModalData* data = new PathAssetModalData(this, GetID());

    m_app->PushModal(new GetAssetModal(a_extensions, a_extensionCount, m_assets, data));
}

std::string GUI::GetPathString()
{
    const std::string id = GetID();

    for (auto iter = m_pathStrings.begin(); iter != m_pathStrings.end(); ++iter)
    {
        if (iter->IDStr == id)
        {
            const std::string path = iter->Path;

            m_pathStrings.erase(iter);

            return path;
        }   
    }

    return std::string();
}

void GUI::SetWidth(float a_width)
{
    Instance->m_width = a_width;
}

std::string GUI::GetID() const
{
    std::string str;

    for (const std::string& s : m_id)
    {
        str += s + " ";
    }

    return str;
}