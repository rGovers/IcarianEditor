#pragma once

#include "Modals/Modal.h"

class Project;

class ProjectConfigModal : public Modal
{
private:
    Project* m_project;

protected:

public:
    ProjectConfigModal(Project* a_project);
    virtual ~ProjectConfigModal();

    virtual bool Update();
};
