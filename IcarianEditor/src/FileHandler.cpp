#include "FileHandler.h"

#include <stb_image.h>

#include "AssetLibrary.h"
#include "Datastore.h"
#include "Flare/IcarianDefer.h"
#include "Texture.h"
#include "Workspace.h"

FileHandler* FileHandler::Instance = nullptr;

FileHandler::FileHandler(AssetLibrary* a_assets, Workspace* a_workspace)
{
    m_assets = a_assets;

    m_extTex.emplace(".cs", Datastore::GetTexture("Textures/FileIcons/FileIcon_CSharpScript.png"));
    m_extTex.emplace(".def", Datastore::GetTexture("Textures/FileIcons/FileIcon_Def.png"));
    m_extTex.emplace(".scrb", Datastore::GetTexture("Textures/FileIcons/FileIcon_Scribe.png"));
    m_extTex.emplace(".frag", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".fpix", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".ffrag", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLFrag.png"));
    m_extTex.emplace(".vert", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLVert.png"));
    m_extTex.emplace(".fvert", Datastore::GetTexture("Textures/FileIcons/FileIcon_GLVert.png"));
    m_extTex.emplace(".ui", Datastore::GetTexture("Textures/FileIcons/FileIcon_Canvas.png"));
    m_extTex.emplace(".ttf", Datastore::GetTexture("Textures/FileIcons/FileIcon_Font.png"));

    m_extOpenCallback.emplace(".def", FileCallback(std::bind(&Workspace::OpenDef, a_workspace, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    m_extOpenCallback.emplace(".iscene", FileCallback(std::bind(&Workspace::SetScene, a_workspace, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    m_extDragCallback.emplace(".def", FileCallback(std::bind(&Workspace::PushDef, a_workspace, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}
FileHandler::~FileHandler()
{
    for (auto iter = m_textureTex.begin(); iter != m_textureTex.end(); ++iter)
    {
        if (iter->second != nullptr)
        {
            delete iter->second;
        }
    }
}

void FileHandler::Init(AssetLibrary* a_assets, Workspace* a_workspace)
{
    if (Instance == nullptr)
    {
        Instance = new FileHandler(a_assets, a_workspace);
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

    if (ext == ".png")
    {
        const std::string name = a_path.filename().string();

        auto iter = Instance->m_textureTex.find(name);
        if (iter != Instance->m_textureTex.end())
        {
            if (iter->second != nullptr)
            {
                *a_texture = iter->second;
            }
        }
        else
        {
            uint32_t size; 
            const char* data;
            Instance->m_assets->GetAsset(a_path, &size, &data);

            if (size > 0 && data != nullptr)
            {
                int width;
                int height;
                int channels;
                stbi_uc* pixels = stbi_load_from_memory((stbi_uc*)data, size, &width, &height, &channels, STBI_rgb_alpha);

                if (pixels != NULL)
                {
                    IDEFER(stbi_image_free(pixels));

                    Texture* texture = new Texture(width, height, pixels);
                    *a_texture = texture;
                    Instance->m_textureTex.emplace(name, texture);
                }
                else 
                {
                    Instance->m_textureTex.emplace(name, nullptr);
                }
            }
        }
    }

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