// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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