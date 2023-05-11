#include "FileDialog.h"

#include <filesystem>
#include <imgui.h>

static bool IDirectoryExplorer(const std::list<std::filesystem::path>& a_dirs, std::filesystem::path* a_path)
{
    if (a_path->has_parent_path())
    {
        if (ImGui::Selectable(".."))
        {
            *a_path = a_path->parent_path();

            return false;
        }

        ImGui::NextColumn();
    }

    for (const std::filesystem::path& dir : a_dirs)
    {
        // C string seems to be a bit wonky so using another var seems to fix crash
        const std::string name = dir.filename().string();
        if (ImGui::Selectable(name.c_str()))
        {
            *a_path /= dir.filename();

            return false;
        }

        ImGui::NextColumn();
    }

    return true;
}

bool FileDialog::GenerateDirs(std::list<std::filesystem::path>* a_dirs, const std::filesystem::path& a_path)
{
    if (std::filesystem::exists(a_path))
    {   
        auto fsIter = std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied);

        for (const auto& iter : fsIter)
        {
            if (iter.is_directory())
            {
                a_dirs->emplace_back(iter.path());
            }
        }

        a_dirs->sort();

        return true;
    }

    return false;
}
bool FileDialog::GenerateFileDirs(std::list<std::filesystem::path>* a_dirs, std::list<std::filesystem::path>* a_files, const std::filesystem::path& a_path)
{
    if (std::filesystem::exists(a_path))
    {   
        auto fsIter = std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied);

        for (const auto& iter : fsIter)
        {
            if (iter.is_directory())
            {
                a_dirs->emplace_back(iter.path());
            }
            else if (iter.is_regular_file())
            {
                a_files->emplace_back(iter.path());
            }
        }

        a_dirs->sort();
        a_files->sort();

        return true;
    }

    return false;
}

bool FileDialog::DirectoryExplorer(const std::list<std::filesystem::path>& a_dirs, std::filesystem::path* a_path)
{
    bool ret = true;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 region = ImGui::GetContentRegionAvail();

    if (ImGui::BeginChild("Dir", { region.x - style.FramePadding.x, region.y - ImGui::GetFrameHeightWithSpacing() * 2}))
    {
        if (std::filesystem::exists(*a_path))
        {
            ImGui::Columns(3);
            ImGui::BeginGroup();

            ret = IDirectoryExplorer(a_dirs, a_path);

            ImGui::EndGroup();
            ImGui::Columns();
        }

        ImGui::EndChild();
    }

    return ret;
}
bool FileDialog::FileExplorer(const std::list<std::filesystem::path>& a_dirs, const std::list<std::filesystem::path>& a_files, std::filesystem::path* a_path, std::string* a_name)
{
    bool ret = true;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 region = ImGui::GetContentRegionAvail();

    if (ImGui::BeginChild("Dir", { region.x - style.FramePadding.x, region.y - ImGui::GetFrameHeightWithSpacing() * 2}))
    {
        if (std::filesystem::exists(*a_path))
        {
            ImGui::Columns(3);
            ImGui::BeginGroup();

            ret = IDirectoryExplorer(a_dirs, a_path);

            ImGui::EndGroup();
            ImGui::Columns();

            ImGui::Separator();

            ImGui::Columns(3);
            ImGui::BeginGroup();

            for (const std::filesystem::path& path : a_files)
            {
                const std::string name = path.filename().string();
                if (ImGui::Selectable(name.c_str()))
                {
                    *a_name = name;
                }

                ImGui::NextColumn();
            }

            ImGui::EndGroup();
            ImGui::Columns();
        }

        ImGui::EndChild();
    }

    return ret;
}