#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <functional>
#include <list>

class AppMain;

class OpenProjectModal : public Modal
{
public:
    using Callback = std::function<void(const std::filesystem::path&, const std::string_view&)>;

private:
    static constexpr uint32_t BufferSize = 4096;

    AppMain*                         m_app;
             
    std::filesystem::path            m_path;
    std::string                      m_name;

    std::list<std::filesystem::path> m_dirs;
    std::list<std::filesystem::path> m_files;

    Callback                         m_callback;

protected:

public:
    OpenProjectModal(AppMain* a_app, Callback a_callback);
    virtual ~OpenProjectModal();

    virtual bool Update();
};