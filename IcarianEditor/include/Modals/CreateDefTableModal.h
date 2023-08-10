#pragma once

#include "Modals/Modal.h"

#include <filesystem>

class AppMain;
class Project;

class CreateDefTableModal : public Modal
{
private:
    static constexpr uint32_t BufferSize = 2048;

    AppMain*              m_app;
    Project*              m_project;

    std::filesystem::path m_path;

    char                  m_name[BufferSize];

protected:

public:
    CreateDefTableModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path);
    virtual ~CreateDefTableModal();

    virtual bool Update();
};