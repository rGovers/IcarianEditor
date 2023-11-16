#include "Modals/CreateProjectModal.h"

#include <imgui.h>
#include <string.h>

#include "AppMain.h"
#include "FileDialog.h"
#include "FileDialogBlock.h"
#include "IO.h"
#include "Modals/ErrorModal.h"

CreateProjectModal::CreateProjectModal(AppMain* a_app, Callback a_callback) : Modal("Create Project", glm::vec2(640, 480))
{
    m_app = a_app;

    m_callback = a_callback;

    m_fileDialogBlock = new FileDialogBlock(glm::vec2(-1, -1), true);
}
CreateProjectModal::~CreateProjectModal()
{
    delete m_fileDialogBlock;
}

bool CreateProjectModal::Update()
{
    std::filesystem::path path;
    std::string name;

    const e_FileDialogStatus status = m_fileDialogBlock->ShowFileDialog(&path, &name);

    switch (status)
    {
    case FileDialogStatus_Ok:
    {
        if (!std::filesystem::exists(path))
        {
            m_app->PushModal(new ErrorModal("Directory does not exist"));

            return true;
        }

        if (name.empty())
        {
            m_app->PushModal(new ErrorModal("Invalid Name"));

            return true;
        }

        m_callback(path, name);

        return false;
    }
    case FileDialogStatus_Error:
    {
        m_app->PushModal(new ErrorModal("File dialog error"));

        break;
    }
    case FileDialogStatus_Cancel:
    {
        return false;
    }
    default:
    {
        break;
    }
    }

    return true;
}