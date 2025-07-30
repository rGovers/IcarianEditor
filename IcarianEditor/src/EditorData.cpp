// Icarian Editor - Editor for the Icarian Game Engine
//
// License at end of file.

#include "EditorData.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <tinyxml2.h>

#include "Core/StringUtils.h"
#include "IO.h"

static EditorData* Instance = nullptr;

EditorData::EditorData()
{
    m_lastProjectCount = 0;

    const std::filesystem::path appPath = IO::GetUserAppdataPath();
    const std::filesystem::path dataPath = appPath / "EditorData.xml";

    if (!std::filesystem::exists(dataPath))
    {
        return;
    }

    const std::string dataPathStr = dataPath.generic_string();

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(dataPathStr.c_str()) != tinyxml2::XML_SUCCESS)
    {
        return;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("Data");
    if (root == nullptr)
    {
        return;
    }

    for (const tinyxml2::XMLElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
    {
        const char* name = element->Name();

        switch (StringHash(name))
        {
        case StringHash("LastProjectPath"):
        {
            // This may change between versions so just do a sanity check
            if (m_lastProjectCount >= MaxProjectCount)
            {
                break;
            }

            const char* text = element->GetText();

            // No point adding the project to the list if it does not exist anymore
            if (std::filesystem::exists(text))
            {
                m_lastProjectPath[m_lastProjectCount++] = text;
            }

            break;
        }
        }
    }
}
EditorData::~EditorData()
{
    Serialize();
}

void EditorData::Init()
{
    if (Instance == nullptr)
    {
        Instance = new EditorData();
    }
}
void EditorData::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void EditorData::AddLastProjectPath(const std::filesystem::path& a_path)
{
    const std::string str = a_path.generic_string();

    for (uint32_t i = 0; i < Instance->m_lastProjectCount; ++i)
    {
        if (Instance->m_lastProjectPath[i] == str)
        {
            for (uint32_t j = i; j > 0; --j)
            {
                Instance->m_lastProjectPath[j] = Instance->m_lastProjectPath[j - 1];
            }

            Instance->m_lastProjectPath[0] = str;

            return;
        }
    }

    for (uint32_t i = Instance->m_lastProjectCount; i > 0; --i)
    {
        Instance->m_lastProjectPath[i] = Instance->m_lastProjectPath[i - 1];
    }

    Instance->m_lastProjectPath[0] = str;
    Instance->m_lastProjectCount = glm::min(Instance->m_lastProjectCount + 1, MaxProjectCount);
}

uint32_t EditorData::GetLastProjectCount()
{
    return Instance->m_lastProjectCount;
}
const std::string* EditorData::GetLastProjectPaths()
{
    return Instance->m_lastProjectPath;
}

void EditorData::Serialize()
{
    const std::filesystem::path appPath = IO::GetUserAppdataPath();
    if (!std::filesystem::exists(appPath))
    {
        std::filesystem::create_directories(appPath);
    }

    const std::filesystem::path dataPath = appPath / "EditorData.xml";

    const std::string dataPathStr = dataPath.generic_string();

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);

    tinyxml2::XMLElement* root = doc.NewElement("Data");
    doc.InsertEndChild(root);

    for (uint32_t i = 0; i < Instance->m_lastProjectCount; ++i)
    {
        const std::string& path = Instance->m_lastProjectPath[i];

        // Do not bother saving if it no longer exists
        if (!std::filesystem::exists(path))
        {
            continue;
        }

        tinyxml2::XMLElement* pathElement = doc.NewElement("LastProjectPath");
        pathElement->SetText(path.c_str());
        root->InsertEndChild(pathElement);
    }

    doc.SaveFile(dataPathStr.c_str());
}

// MIT License
//
// Copyright (c) 2025 River Govers
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
