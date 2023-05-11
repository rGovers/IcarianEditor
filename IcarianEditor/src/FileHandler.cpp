#include "FileHandler.h"

#include "Datastore.h"
#include "Texture.h"
#include "Workspace.h"

FileHandler* FileHandler::Instance = nullptr;

FileHandler::FileHandler(Workspace* a_workspace)
{
    m_extTex.emplace(".cs", Datastore::GetTexture("Textures/FileIcons/FileIcon_CSharpScript.png"));
    m_extTex.emplace(".def", Datastore::GetTexture("Textures/FileIcons/FileIcon_Def.png"));
    m_extTex.emplace(".scrb", Datastore::GetTexture("Textures/FileIcons/FileIcon_Scribe.png"));
    m_extTex.emplace(".frag", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".fpix", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".ffrag", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".vert", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLVert.png"));
    m_extTex.emplace(".fvert", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLVert.png"));

    m_extOpenCallback.emplace(".def", FileCallback(std::bind(&Workspace::OpenDef, a_workspace, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    m_extOpenCallback.emplace(".iscene", FileCallback(std::bind(&Workspace::SetScene, a_workspace, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    m_extDragCallback.emplace(".def", FileCallback(std::bind(&Workspace::PushDef, a_workspace, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}
FileHandler::~FileHandler()
{

}

void FileHandler::Init(Workspace* a_workspace)
{
    if (Instance == nullptr)
    {
        Instance = new FileHandler(a_workspace);
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

void FileHandler::GetFileData(const std::filesystem::path& a_path, FileCallback** a_openCallback, FileCallback** a_dragCallback, Texture** a_texture)
{
    *a_texture = Datastore::GetTexture("Textures/FileIcons/FileIcon_Unknown.png");
    *a_openCallback = nullptr;
    *a_dragCallback = nullptr;

    if (!a_path.has_extension())
    {
        return;
    }

    const std::string ext = a_path.extension().string();

    const auto tIter = Instance->m_extTex.find(ext);
    if (tIter != Instance->m_extTex.end())
    {
        *a_texture = tIter->second;
    }

    const auto oIter = Instance->m_extOpenCallback.find(ext);
    if (oIter != Instance->m_extOpenCallback.end())
    {
        *a_openCallback = &oIter->second;
    }

    const auto dIter = Instance->m_extDragCallback.find(ext);
    if (dIter != Instance->m_extDragCallback.end())
    {
        *a_dragCallback = &dIter->second;
    }
}