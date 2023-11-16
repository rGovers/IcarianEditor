#include "Modals/OpenProjectModal.h"

#include <imgui.h>

#include "AppMain.h"
#include "FileDialogBlock.h"
#include "Modals/ErrorModal.h"

constexpr static const char* const Filters[] = { ".icproj", "*" };
constexpr uint32_t FilterCount = sizeof(Filters) / sizeof(*Filters);

OpenProjectModal::OpenProjectModal(AppMain* a_app, Callback a_callback) : Modal("Open Project", glm::vec2(640, 480))
{
    m_app = a_app;

    m_callback = a_callback;

    m_fileDialogBlock = new FileDialogBlock(glm::vec2(-1, -1), false, FilterCount, Filters);
}
OpenProjectModal::~OpenProjectModal()
{
    delete m_fileDialogBlock;
}

bool OpenProjectModal::Update()
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