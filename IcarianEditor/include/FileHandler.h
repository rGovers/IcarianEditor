#pragma once

#include <filesystem>
#include <functional>
#include <unordered_map>

class AssetLibrary;
class Texture;
class Workspace;

class FileHandler
{
public:
    using FileCallback = std::function<void(const std::filesystem::path&, uint32_t, const char*)>;

private:
    static FileHandler* Instance;

    AssetLibrary*                                 m_assets;       

    std::unordered_map<std::string, Texture*>     m_extTex;
    std::unordered_map<std::string, Texture*>     m_textureTex;
    std::unordered_map<std::string, FileCallback> m_extOpenCallback;
    std::unordered_map<std::string, FileCallback> m_extDragCallback;

    FileHandler(AssetLibrary* a_assets, Workspace* a_workspace);
protected:

public:
    ~FileHandler();

    static void Init(AssetLibrary* a_assets, Workspace* a_workspace);
    static void Destroy();

    static void GetFileData(const std::filesystem::path& a_path, FileCallback** a_openCallback, FileCallback** a_dragCallback, Texture** a_texture);
};