#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <functional>

class AppMain;
class FileDialogBlock;

class OpenProjectModal : public Modal
{
public:
    using Callback = std::function<void(const std::filesystem::path&, const std::string_view&)>;

private:
    AppMain*         m_app;
             
    FileDialogBlock* m_fileDialogBlock;

    Callback         m_callback;

protected:

public:
    OpenProjectModal(AppMain* a_app, Callback a_callback);
    virtual ~OpenProjectModal();

    virtual bool Update();
};