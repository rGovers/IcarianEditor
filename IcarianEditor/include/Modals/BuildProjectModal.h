#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <list>
#include <vector>

class AppMain;
class AssetLibrary;
class FileDialogBlock;
class Project;

class BuildProjectModal : public Modal
{
private:
    AppMain*                         m_app;
    AssetLibrary*                    m_library;
    Project*                         m_project;

    FileDialogBlock*                 m_fileDialogBlock;

    uint32_t                         m_selectedExport;
    std::vector<std::string>         m_exportOptions;

    void OptionsDisplay();

protected:

public:
    BuildProjectModal(AppMain* a_app, AssetLibrary* a_library, Project* a_project, const std::vector<std::string>& a_exportOptions);
    virtual ~BuildProjectModal();

    virtual bool Update();
};