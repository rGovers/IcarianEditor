#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <functional>
#include <list>

class AppMain;

class CreateProjectModal : public Modal
{
public:
    using Callback = std::function<void(const std::filesystem::path&, const std::string_view&)>;

private:
    static constexpr uint32_t BufferSize = 4096;

    AppMain*                         m_app;
             
    std::filesystem::path            m_path;
    std::string                      m_name;

    std::list<std::filesystem::path> m_dirs;

    Callback                         m_callback;

protected:

public:
    CreateProjectModal(AppMain* a_app, Callback a_callback);
    virtual ~CreateProjectModal();

    virtual bool Update();
};