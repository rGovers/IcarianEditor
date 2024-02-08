#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <functional>
#include <list>

class AppMain;
class FileDialogBlock;

class CreateProjectModal : public Modal
{
public:
    using Callback = std::function<void(const std::filesystem::path&, const std::string_view&)>;

private:
    AppMain*         m_app;
             
    FileDialogBlock* m_fileDialogBlock;

    Callback         m_callback;

protected:

public:
    CreateProjectModal(AppMain* a_app, Callback a_callback);
    virtual ~CreateProjectModal();

    virtual bool Update();
};