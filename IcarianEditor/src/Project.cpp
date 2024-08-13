// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Project.h"

#include <filesystem>
#include <fstream>
#include <tinyxml2.h>

#include "AppMain.h"
#include "AssetLibrary.h"
#include "Core/IcarianDefer.h"
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

    m_projectFlags = 0;
}
Project::~Project()
{

}

void Project::SaveProjectFile() const
{
    const std::filesystem::path projectFilePath = GetProjectFilePath();
    const std::string projectFilePathStr = projectFilePath.string();

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

    tinyxml2::XMLElement* versionPatchElement = doc.NewElement("Patch");
    versionElement->InsertEndChild(versionPatchElement);
    versionPatchElement->SetText(ICARIANEDITOR_VERSION_PATCH);

    tinyxml2::XMLElement* flags = doc.NewElement("Flags");
    rootElement->InsertEndChild(flags);

    tinyxml2::XMLElement* convertKtx = doc.NewElement("ConvertKTX");
    flags->InsertEndChild(convertKtx);
    convertKtx->SetText(ConvertKTX());

    doc.SaveFile(projectFilePathStr.c_str());
}

void Project::ReloadProjectFile()
{
    const std::filesystem::path projectFilePath = GetProjectFilePath();
    const std::string projectFilePathStr = projectFilePath.string();

    tinyxml2::XMLDocument doc;
    doc.LoadFile(projectFilePathStr.c_str());

    const tinyxml2::XMLElement* rootElement = doc.FirstChildElement("Project");
    for (const tinyxml2::XMLElement* element = rootElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
    {
        const char* name = element->Name();

        if (strcmp(name, "Flags") == 0)
        {
            for (const tinyxml2::XMLElement* flagsElement = element->FirstChildElement(); flagsElement != nullptr; flagsElement = flagsElement->NextSiblingElement())
            {
                const char* name = flagsElement->Name();

                if (strcmp(name, "ConvertKTX") == 0)
                {
                    SetConvertKTX(flagsElement->BoolText());
                }
            }   
        }
    }
}

void Project::NewCallback(const std::filesystem::path& a_path, const std::string_view& a_name)
{
    m_name = std::string(a_name);
    m_path = a_path / m_name;

    GenerateDirs(m_path);
    
    SaveProjectFile();

    IDEFER(m_shouldRefresh = true);

    const std::string assemblyControlName = m_name + "AssemblyControl";

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

    IDEFER(m_shouldRefresh = true);

    ReloadProjectFile();

    GenerateDirs(m_path);
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

        m_assetLibrary->Serialize(this);

        SaveProjectFile();
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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.