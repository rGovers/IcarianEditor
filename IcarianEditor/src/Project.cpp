#include "Project.h"

#include <filesystem>
#include <fstream>
#include <tinyxml2.h>

#include "AppMain.h"
#include "AssetLibrary.h"
#include "Logger.h"
#include "Modals/BuildProjectModal.h"
#include "Modals/CreateProjectModal.h"
#include "Modals/ErrorModal.h"
#include "Modals/OpenProjectModal.h"
#include "TemplateBuilder.h"
#include "Templates.h"

static void GenerateDirs(const std::filesystem::path& a_path)
{
    const std::filesystem::path projectDir = a_path / "Project";
    if (!std::filesystem::exists(projectDir))
    {
        std::filesystem::create_directories(projectDir);
    }

    const std::filesystem::path sceneDir = projectDir / "Scenes";
    if (!std::filesystem::exists(sceneDir))
    {
        std::filesystem::create_directories(sceneDir);
    }

    const std::filesystem::path cacheDir = a_path / ".cache";
    if (!std::filesystem::exists(cacheDir))
    {
        std::filesystem::create_directories(cacheDir);
    }
}

Project::Project(AppMain* a_app, AssetLibrary* a_assetLibrary, Workspace* a_workspace)
{
    m_app = a_app;
    m_assetLibrary = a_assetLibrary;
    m_workspace = a_workspace;

    m_shouldRefresh = false;
}
Project::~Project()
{

}

void Project::NewCallback(const std::filesystem::path& a_path, const std::string_view& a_name)
{
    m_path = a_path;
    m_name = std::string(a_name);

    GenerateDirs(m_path);
    
    std::filesystem::path path = m_path / m_name;
    if (!path.has_extension())
    {
        path += ".icproj";
    }

    tinyxml2::XMLDocument doc;
    doc.InsertEndChild(doc.NewDeclaration());
    tinyxml2::XMLElement* rootElement = doc.NewElement("Project");
    doc.InsertEndChild(rootElement);
    
    tinyxml2::XMLElement* versionElement = doc.NewElement("Version");
    rootElement->InsertEndChild(versionElement);
    
    tinyxml2::XMLElement* versionMajorElement = doc.NewElement("Major");
    versionElement->InsertEndChild(versionMajorElement);
    versionMajorElement->SetText(ICARIANEDITOR_VERSION_MAJOR);

    tinyxml2::XMLElement* versionMinorElement = doc.NewElement("Minor");
    versionElement->InsertEndChild(versionMinorElement);
    versionMinorElement->SetText(ICARIANEDITOR_VERSION_MINOR);

    doc.SaveFile(path.string().c_str());

    m_shouldRefresh = true;

    const std::string assemblyControlName = std::string(a_name) + "AssemblyControl";

    std::ofstream assemblyControlStream = std::ofstream(m_path / "Project" / (assemblyControlName + ".cs"));
    if (assemblyControlStream.good() && assemblyControlStream.is_open())
    {
        assemblyControlStream << TemplateBuilder::GenerateFromTemplate(AssemblyControlTemplate, a_name, assemblyControlName);
        assemblyControlStream.close();
    }
    else
    {
        Logger::Error("Unable to create Assembly Control");
    }

    std::ofstream aboutStream = std::ofstream(m_path / "Project" / "about.xml");
    if (aboutStream.good() && aboutStream.is_open())
    {
        aboutStream << TemplateBuilder::GenerateFromTemplate(AboutTemplate, a_name, "about");
        aboutStream.close();
    }
    else
    {
        Logger::Error("Unable to create About");
    }
}
void Project::OpenCallback(const std::filesystem::path& a_path, const std::string_view& a_name)
{
    m_path = a_path;

    // Want to strip out extension if it exists
    const std::size_t extPos = a_name.find_last_of(".");
    if (extPos == std::string_view::npos)
    {
        m_name = std::string(a_name);
    }
    else
    {
        m_name = std::string(a_name, 0, extPos);
    }

    GenerateDirs(m_path);

    m_shouldRefresh = true;
}

void Project::New()
{
    Logger::Message("New Project");

    m_app->PushModal(new CreateProjectModal(m_app, std::bind(&Project::NewCallback, this, std::placeholders::_1, std::placeholders::_2)));
}
void Project::Open()
{
    Logger::Message("Open Project");

    m_app->PushModal(new OpenProjectModal(m_app, std::bind(&Project::OpenCallback, this, std::placeholders::_1, std::placeholders::_2)));
}
void Project::Save() const
{
    if (IsValidProject())
    {
        Logger::Message("Save Project");

        m_assetLibrary->Serialize(m_path);
    }
    else
    {
        m_app->PushModal(new ErrorModal("Saving Invalid Project"));
    }
}
void Project::Build()
{
    if (IsValidProject())
    {
        Logger::Message("Build Project");

        std::vector<std::string> buildSystems;

        for (const auto& iter : std::filesystem::directory_iterator("BuildFiles", std::filesystem::directory_options::skip_permission_denied))
        {
            if (iter.is_directory())
            {
                buildSystems.emplace_back(iter.path().filename().string());
            }
        }

        if (buildSystems.empty())
        {
            m_app->PushModal(new ErrorModal("No Build Systems Found"));
            
            return;
        }

        m_app->PushModal(new BuildProjectModal(m_app, m_assetLibrary, this, buildSystems));
    }
    else
    {
        m_app->PushModal(new ErrorModal("Building Invalid Project"));
    }
}