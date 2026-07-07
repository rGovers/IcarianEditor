// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "FileHandler.h"

#include <ktx.h>
#include <stb_image.h>

#include "AssetLibrary.h"
#include "Datastore.h"
#include "EditorConfig.h"
#include "IO.h"
#include "Runtime/RuntimeManager.h"
#include "Texture.h"
#include "Workspace.h"

FileHandler* Instance = nullptr;

static void OpenCSScript(const std::filesystem::path& a_path, const std::filesystem::path& a_relativePath, uint32_t a_size, const uint8_t* a_data)
{
    const e_CodeEditor codeEditor = EditorConfig::GetCodeEditor();
    switch (codeEditor)
    {
    case CodeEditor_VisualStudio:
    {
        IO::StartOpenFile("devenv", a_path);

        break;
    }
    case CodeEditor_VisualStudioCode:
    {
        IO::OpenFile("code", a_path);

        break;
    }
    case CodeEditor_VSCodium:
    {
        IO::OpenFile("codium", a_path);

        break;
    }
    case CodeEditor_Kate:
    {
        IO::OpenFile("kate", a_path);

        break;
    }
    default:
    {
        IO::OpenFile(a_path);

        break;
    }
    }
}

static void OpenShader(const std::filesystem::path& a_path, const std::filesystem::path& a_relativePath, uint32_t a_size, const uint8_t* a_data)
{
    const e_CodeEditor codeEditor = EditorConfig::GetCodeEditor();
    switch (codeEditor)
    {
    case CodeEditor_VisualStudioCode:
    {
        IO::OpenFile("code", a_path);

        break;
    }
    case CodeEditor_VSCodium:
    {
        IO::OpenFile("codium", a_path);

        break;
    }
    case CodeEditor_Kate:
    {
        IO::OpenFile("kate", a_path);

        break;
    }
    default:
    {
        IO::OpenFile(a_path);

        break;
    }
    }
}

static void OpenDef(const std::filesystem::path& a_path, const std::filesystem::path& a_relativePath, uint32_t a_size, const uint8_t* a_data)
{
    const e_DefEditor defEditor = EditorConfig::GetDefEditor();

    switch (defEditor)
    {
    case DefEditor_VisualStudioCode:
    {
        IO::OpenFile("code", a_path);

        break;
    }
    case DefEditor_VSCodium:
    {
        IO::OpenFile("codium", a_path);

        break;
    }
    case DefEditor_Kate:
    {
        IO::OpenFile("kate", a_path);

        break;
    }
    default:
    {
        Workspace::OpenDef(a_relativePath);

        break;
    }
    }
}
static void SetScene(const std::filesystem::path& a_path, const std::filesystem::path& a_relativePath, uint32_t a_size, const uint8_t* a_data)
{
    Workspace::SetCurrentScene(a_relativePath);
}

static void PushDef(const std::filesystem::path& a_path, const std::filesystem::path& a_relativePath, uint32_t a_size, const uint8_t* a_data)
{
    const std::string str = a_relativePath.generic_string();
    ImGui::SetDragDropPayload("DefPath", str.c_str(), str.size(), ImGuiCond_Once);
}

FileHandler::FileHandler()
{
    m_extTex.emplace(".cs", Datastore::GetTexture("Textures/FileIcons/FileIcon_CSharpScript.png"));
    m_extTex.emplace(".def", Datastore::GetTexture("Textures/FileIcons/FileIcon_Def.png"));
    m_extTex.emplace(".scrb", Datastore::GetTexture("Textures/FileIcons/FileIcon_Scribe.png"));
    m_extTex.emplace(".fpix", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".ffrag", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".fvert", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLVert.png"));
    m_extTex.emplace(".ui", Datastore::GetTexture("Textures/FileIcons/FileIcon_Canvas.png"));
    m_extTex.emplace(".ttf", Datastore::GetTexture("Textures/FileIcons/FileIcon_Font.png"));
    m_extTex.emplace(".wav", Datastore::GetTexture("Textures/FileIcons/FileIcon_Sound.png"));
    m_extTex.emplace(".ogg", Datastore::GetTexture("Textures/FileIcons/FileIcon_Sound.png"));

    m_extOpenCallback.emplace(".def", FileCallback(std::bind(OpenDef, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    m_extOpenCallback.emplace(".iscene", FileCallback(std::bind(SetScene, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    m_extOpenCallback.emplace(".cs", FileCallback(std::bind(OpenCSScript, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    m_extOpenCallback.emplace(".fvert", FileCallback(std::bind(OpenShader, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    m_extOpenCallback.emplace(".fmesh", FileCallback(std::bind(OpenShader, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    m_extOpenCallback.emplace(".ftask", FileCallback(std::bind(OpenShader, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    m_extOpenCallback.emplace(".fpix", FileCallback(std::bind(OpenShader, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    m_extOpenCallback.emplace(".ffrag", FileCallback(std::bind(OpenShader, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

    m_extDragCallback.emplace(".def", FileCallback(std::bind(PushDef, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
}
FileHandler::~FileHandler()
{

}

void FileHandler::Init()
{
    if (Instance == nullptr)
    {
        Instance = new FileHandler();
    }
}
void FileHandler::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void FileHandler::GetFileData(const std::filesystem::path& a_path, FileCallback** a_openCallback, FileCallback** a_dragCallback, GLuint* a_texture)
{
    const Texture* unknownTex = Datastore::GetTexture("Textures/FileIcons/FileIcon_Unknown.png");

    *a_texture = unknownTex->GetHandle();
    *a_openCallback = nullptr;
    *a_dragCallback = nullptr;

    if (!a_path.has_extension())
    {
        return;
    }

    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.generic_string();

    const e_AssetType type = AssetLibrary::GetAssetType(a_path);
    switch (type)
    {
    case AssetType_Texture:
    {
        // TODO: Reimplement me!~

        break;
    }
    default:
    {
        break;
    }
    }

    const auto tIter = Instance->m_extTex.find(extStr);
    if (tIter != Instance->m_extTex.end())
    {
        if (tIter->second != nullptr)
        {
            const Texture* tex = tIter->second;

            *a_texture = tex->GetHandle();
        }
    }

    const auto oIter = Instance->m_extOpenCallback.find(extStr);
    if (oIter != Instance->m_extOpenCallback.end())
    {
        *a_openCallback = &oIter->second;
    }

    const auto dIter = Instance->m_extDragCallback.find(extStr);
    if (dIter != Instance->m_extDragCallback.end())
    {
        *a_dragCallback = &dIter->second;
    }
}

// MIT License
// 
// Copyright (c) 2026 River Govers
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
