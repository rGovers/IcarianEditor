#include "GUI.h"

#include <glm/glm.hpp>
#include <string>

#include "FlareImGui.h"
#include "Logger.h"
#include "Runtime/RuntimeManager.h"

static GUI* Instance = nullptr;

static constexpr uint32_t BufferSize = 4096;

struct IDStack
{
    IDStack(const std::string_view& a_id)
    {
        ImGui::PushID(a_id.data());
    }
    ~IDStack()
    {
        ImGui::PopID();
    }
};

struct WidthStack
{
    WidthStack(float a_width)
    {
        ImGui::PushItemWidth(a_width);
    }
    ~WidthStack()
    {
        ImGui::PopItemWidth();
    }
};

#define STACK_ID(str) const IDStack idStackTVal = IDStack(str)
#define STACK_G_ID(str) STACK_ID(Instance->GetID() + (str))

#define STACK_LABELWIDTH const WidthStack widthStackTVal = WidthStack(Instance->GetTextWidth())
#define STACK_FIELDWIDTH const WidthStack widthStackTVal = WidthStack(Instance->GetFieldWidth())

static void FieldLabel(const std::string_view& a_str)
{
    // TODO: Fix issue with text ignoring size
    STACK_LABELWIDTH;

    const float curY = ImGui::GetCursorPosY();

    ImGui::Text(a_str.data());

    ImGui::SetCursorPos({ Instance->GetWidth() - Instance->GetFieldWidth() - ImGui::GetStyle().ItemSpacing.x, curY });
}

#define LABEL_VAL(str) FieldLabel(str); STACK_FIELDWIDTH

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetCheckbox), MonoString* a_str, uint32_t* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    bool b = (bool)*a_value;

    STACK_G_ID(str);
    LABEL_VAL(str);

    bool ret = false;
    if (ImGui::Checkbox(("##V_" + str).c_str(), &b))
    {
        ret = true;
        *a_value = (uint32_t)b;
    }

    return ret;
}
FLARE_MONO_EXPORT(MonoString*, RUNTIME_FUNCTION_NAME(GUI, GetDef), MonoString* a_str, MonoString* a_preview, MonoString* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    char* preview = mono_string_to_utf8(a_preview);

    MonoString* outBuff = nullptr;

    STACK_G_ID(str);
    
    FieldLabel(str);

    if (ImGui::Button(preview, { Instance->GetFieldWidth(), 0 }))
    {
        Logger::Error("Not implemented use drag and drop");
    }
    
    if (ImGui::BeginDragDropTarget())
    {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DefPath");
        if (payload != nullptr)
        {
            outBuff = mono_string_from_utf32((mono_unichar4*)payload->Data);
        } 

        ImGui::EndDragDropTarget();
    }

    mono_free(preview);

    return outBuff;
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetInt), MonoString* a_str, int32_t* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);
    
    STACK_G_ID(str);
    LABEL_VAL(str);

    return (uint32_t)ImGui::InputInt(("##V_" + str).c_str(), (int*)a_value);
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetUInt), MonoString* a_str, uint32_t* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    STACK_G_ID(str);
    
    FieldLabel(str);

    STACK_FIELDWIDTH;

    return (uint32_t)ImGui::InputInt(("##V_" + str).c_str(), (int*)a_value);
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetFloat), MonoString* a_str, float* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    STACK_G_ID(str);
    LABEL_VAL(str);

    return (uint32_t)ImGui::DragFloat(("##V_" + str).c_str(), a_value);
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetVec2), MonoString* a_str, glm::vec2* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    STACK_G_ID(str);

    LABEL_VAL(str);

    return (uint32_t)ImGui::DragFloat2(("##V_" + str).c_str(), (float*)a_value);
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetVec3), MonoString* a_str, glm::vec3* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    STACK_G_ID(str);
    LABEL_VAL(str);

    return (uint32_t)ImGui::DragFloat3(("##V_" + str).c_str(), (float*)a_value);
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetVec4), MonoString* a_str, glm::vec4* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    STACK_G_ID(str);
    LABEL_VAL(str);

    return (uint32_t)ImGui::DragFloat4(("##V_" + str).c_str(), (float*)a_value);
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetColor), MonoString* a_str, glm::vec4* a_value)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);
    
    STACK_G_ID(str);
    LABEL_VAL(str);

    return (uint32_t)ImGui::ColorEdit4(("##V_" + str).c_str(), (float*)a_value);
}

FLARE_MONO_EXPORT(MonoString*, RUNTIME_FUNCTION_NAME(GUI, GetString), MonoString* a_str, MonoString* a_value)
{
    char buffer[BufferSize];

    char* mStr = mono_string_to_utf8(a_str);
    char* value = mono_string_to_utf8(a_value);
    
    const std::string str = mStr;

    mono_free(mStr);

    MonoString* outBuff = nullptr;

    STACK_G_ID(str);
    LABEL_VAL(str);

    strncpy(buffer, value, BufferSize - 1);
    if (ImGui::InputText(("##V_" + str).c_str(), buffer, BufferSize))
    {
        const RuntimeManager* runtime = Instance->GetRuntime();

        outBuff = mono_string_new(runtime->GetEditorDomain(), buffer);
    }

    mono_free(value);

    return outBuff;
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetStringList), MonoString* a_str, MonoArray* a_list, int32_t* a_selected)
{
    char* mStr = mono_string_to_utf8(a_str);
    const std::string str = mStr;
    mono_free(mStr);

    const uintptr_t size = mono_array_length(a_list);
    if (*a_selected >= size)
    {
        *a_selected = 0;
    }

    char* selectedStr = mono_string_to_utf8(mono_array_get(a_list, MonoString*, *a_selected));

    bool ret = false;

    STACK_G_ID(str);
    LABEL_VAL(str);

    if (ImGui::BeginCombo(("##V_" + str).c_str(), selectedStr))
    {
        for (int32_t i = 0; i < size; ++i)
        {
            const bool selected = i == *a_selected;

            char* selectableStr = mono_string_to_utf8(mono_array_get(a_list, MonoString*, i));

            STACK_G_ID(std::string(str) + "[" + std::to_string(i) + "]");
            if (ImGui::Selectable(selectableStr, selected))
            {
                *a_selected = i;

                ret = true;
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }

            mono_free(selectableStr);
        }

        ImGui::EndCombo();
    }

    mono_free(selectedStr);

    return ret;
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, ResetButton), MonoString* a_str)
{
    char* str = mono_string_to_utf8(a_str);

    STACK_G_ID(str);
    bool ret = FlareImGui::ImageButton(str, "Textures/Icons/Icon_Reset.png", glm::vec2(16.0f));

    ImGui::SameLine();

    mono_free(str);

    return (uint32_t)ret;
}

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, NIndent))
{
    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, Indent))
{
    ImGui::Indent();
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, Unindent))
{
    ImGui::Unindent();
}

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, Tooltip), MonoString* a_title, MonoString* a_str)
{
    char* title = mono_string_to_utf8(a_title);
    char* str = mono_string_to_utf8(a_str);

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();

        if (title != nullptr)
        {
            ImGui::Text(title);

            if (str != nullptr)
            {
                ImGui::Separator();
            }
        }

        if (str != nullptr)
        {
            ImGui::Text(str);
        }

        ImGui::EndTooltip();
    }
    
    mono_free(title);
    mono_free(str);
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, ShowStructView), MonoString* a_str)
{
    bool ret = false;

    char* str = mono_string_to_utf8(a_str);

    STACK_G_ID(str);
    if (ImGui::CollapsingHeader(str))
    {
        ret = true;
    }

    mono_free(str);

    return (uint32_t)ret;
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, ShowArrayView), MonoString* a_str, uint32_t* a_addValue)
{
    bool ret = false;

    char* str = mono_string_to_utf8(a_str);

    {
        STACK_G_ID(std::string(str) + "_Add");
        *a_addValue = (uint32_t)ImGui::Button("+");
    }
    

    ImGui::SameLine();
    
    STACK_G_ID(str);
    if (ImGui::CollapsingHeader(str))
    {
        ret = true;
    }   

    mono_free(str);

    return (uint32_t)ret;
}

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, PushID), MonoString* a_str)
{
    char* str = mono_string_to_utf8(a_str);

    Instance->PushID(str);

    mono_free(str);
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, PopID))
{
    Instance->PopID();
}

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, Label), MonoString* a_str)
{
    char* str = mono_string_to_utf8(a_str);

    ImGui::Text(str);

    mono_free(str);
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetSelectable), MonoString* a_str)
{
    char* str = mono_string_to_utf8(a_str);

    STACK_G_ID(str);
    const bool ret = ImGui::Selectable(str);

    mono_free(str);

    return (uint32_t)ret;
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, NodeI), MonoString* a_str)
{
    char* str = mono_string_to_utf8(a_str);
    
    STACK_G_ID(str);
    const bool ret = ImGui::TreeNode(str);

    mono_free(str);

    return (uint32_t)ret;
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, PopNode))
{
    ImGui::TreePop();
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(GUI, SameLine))
{
    ImGui::SameLine();
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetShiftModifier))
{
    return (uint32_t)(ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift));
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(GUI, GetCtrlModifier))
{
    return (uint32_t)(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl));
}

GUI::GUI(RuntimeManager* a_runtime)
{
    m_runtime = a_runtime;
}
GUI::~GUI()
{
    
}

void GUI::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        Instance = new GUI(a_runtime);
        
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetCheckbox);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetDef);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetInt);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetUInt);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetFloat);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetVec2);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetVec3);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetVec4);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetColor);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetString);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetStringList);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, ResetButton);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, NIndent);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Indent);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Unindent);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, ShowStructView);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, ShowArrayView);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Tooltip);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, PushID);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, PopID);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, Label);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetSelectable);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, NodeI);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, PopNode);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, SameLine);

        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetShiftModifier);
        BIND_FUNCTION(a_runtime, IcarianEditor, GUI, GetCtrlModifier);
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

void GUI::SetWidth(float a_width)
{
    Instance->m_width = a_width;
}

std::string GUI::GetID() const
{
    std::string str;

    for (const std::string& s : m_id)
    {
        str += s;
    }

    return str;
}