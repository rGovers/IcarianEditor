#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <list>
#include <vector>

class AppMain;
class AssetLibrary;
class Project;

class BuildProjectModal : public Modal
{
private:
    static constexpr uint32_t BufferSize = 4096;

    AppMain*                         m_app;
    AssetLibrary*                    m_library;
    Project*                         m_project;

    std::filesystem::path            m_path;
    std::string                      m_name;

    std::list<std::filesystem::path> m_dirs;

    uint32_t                         m_selectedExport;
    std::vector<std::string>         m_exportOptions;

    void OptionsDisplay();

protected:

public:
    BuildProjectModal(AppMain* a_app, AssetLibrary* a_library, Project* a_project, const std::vector<std::string>& a_exportOptions);
    virtual ~BuildProjectModal();

    virtual bool Update();
};