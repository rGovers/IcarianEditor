#pragma once

#include "Modals/Modal.h"
#include "Project.h"

#include <filesystem>

class AppMain;
class Project;

class RenamePathModal : public Modal
{
private:
    // Generally safe with 2048 characters because Windows still has issues with
    // paths longer than 260 characters despite adding support for longer paths in
    // Windows 10. 
    // Would have to do Windows specific code to use the full 32,767 characters.
    // Because the setting is controller by the user it is not worth the effort.
    // Just picked a number that seemed reasonable for UNIX systems as it just works with them.
    static constexpr uint32_t BufferSize = 2048;

    AppMain*              m_app;
    Project*              m_project;

    std::filesystem::path m_path;

    char                  m_name[BufferSize];

protected:

public:
    RenamePathModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path);
    virtual ~RenamePathModal();

    virtual bool Update();
};