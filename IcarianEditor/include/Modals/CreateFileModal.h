#pragma once

#include "Modals/Modal.h"

#include <filesystem>

class AppMain;
class Project;

class CreateFileModal : public Modal
{
private:
    static constexpr uint32_t BufferSize = 2048;

    AppMain*              m_app;
    Project*              m_project;

    char*                 m_data;
    uint32_t              m_size;

    std::filesystem::path m_path;
    std::string           m_extension;   

    char                  m_name[BufferSize];

protected:

public:
    CreateFileModal(AppMain* a_app, Project* a_project, const std::filesystem::path& a_path, const char* a_data, uint32_t a_size, const std::string_view& a_name, const std::string_view& a_extension);
    virtual ~CreateFileModal();

    virtual bool Update();
};