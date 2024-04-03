#include "LoadingTasks/GenerateConfigLoadingTask.h"

#include <tinyxml2.h>

GenerateConfigLoadingTask::GenerateConfigLoadingTask(const std::filesystem::path& a_path, const std::string_view& a_name, const std::string_view& a_renderBackend)
{
    m_path = a_path;
    m_name = a_name;
    m_renderBackend = a_renderBackend;
}
GenerateConfigLoadingTask::~GenerateConfigLoadingTask()
{

}

void GenerateConfigLoadingTask::Run()
{
    tinyxml2::XMLDocument doc;

    doc.InsertEndChild(doc.NewDeclaration(nullptr));

    tinyxml2::XMLElement* root = doc.NewElement("Config");
    doc.InsertEndChild(root);

    tinyxml2::XMLElement* applicationName = doc.NewElement("ApplicationName");
    root->InsertEndChild(applicationName);
    applicationName->SetText(m_name.c_str());

    tinyxml2::XMLElement* renderEngine = doc.NewElement("RenderEngine");
    root->InsertEndChild(renderEngine);
    renderEngine->SetText(m_renderBackend.c_str());

    doc.SaveFile((m_path / "config.xml").string().c_str());
}