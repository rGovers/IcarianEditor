#include "Modals/GetAssetModal.h"

#include <imgui.h>

#include "AssetLibrary.h"
#include "Core/IcarianDefer.h"

GetAssetModal::GetAssetModal(char* const* a_extensions, uint32_t a_extensionCount, AssetLibrary* a_assetLibrary, GetAssetModalData* a_data) : Modal("Get Asset")
{
    m_data = a_data;

    m_curIndex = 0;

    for (uint32_t i = 0; i < a_extensionCount; ++i)
    {
        std::vector<std::filesystem::path> paths = a_assetLibrary->GetAssetPathWithExtension(a_extensions[i]);

        m_paths.insert(m_paths.end(), paths.begin(), paths.end());
    }
}
GetAssetModal::~GetAssetModal()
{
    delete m_data;
}

bool GetAssetModal::Update()
{
    const uint32_t count = (uint32_t)m_paths.size();
    if (count > 0)
    {
        const std::string curVal = m_paths[m_curIndex].string();

        if (ImGui::BeginCombo("Asset", curVal.c_str()))
        {
            IDEFER(ImGui::EndCombo());

            static char Buffer[4096] = { 0 };
            ImGui::InputText("##Search", Buffer, sizeof(Buffer) - 1);

            for (uint32_t i = 0; i < count; ++i)
            {
                const std::string str = m_paths[i].string();
                if (Buffer[0] != 0 && strstr(str.c_str(), Buffer) == NULL)
                {
                    continue;
                }

                const bool selected = i == m_curIndex;

                if (ImGui::Selectable(str.c_str(), selected))
                {
                    m_curIndex = i;
                }

                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }

        if (ImGui::Button("Confirm"))
        {
            m_data->Confirm(m_paths[m_curIndex]);

            return false;
        }

        ImGui::SameLine();
    }

    if (ImGui::Button("Cancel"))
    {
        m_data->Cancel();

        return false;
    }

    return true;   
}