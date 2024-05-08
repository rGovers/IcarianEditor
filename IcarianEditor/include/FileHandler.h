#pragma once

#include <filesystem>
#include <functional>
#include <glad/glad.h>
#include <unordered_map>

class AssetLibrary;
class RuntimeManager;
class RuntimeStorage;
class Texture;
class Workspace;

#include "EditorFileHandlerInteropStructures.h"

class FileHandler
{
public:
    using FileCallback = std::function<void(const std::filesystem::path&, const std::filesystem::path&, uint32_t, const char*)>;

private:
    FileTextureHandle                             m_runtimeTexHandle;

    AssetLibrary*                                 m_assets;
    RuntimeManager*                               m_runtime;
    RuntimeStorage*                               m_storage;

    std::unordered_map<std::string, Texture*>     m_extTex;
    std::unordered_map<std::string, Texture*>     m_textureTex;
    std::unordered_map<std::string, FileCallback> m_extOpenCallback;
    std::unordered_map<std::string, FileCallback> m_extDragCallback;

    FileHandler(AssetLibrary* a_assets, RuntimeManager* a_runtime, RuntimeStorage* a_storage, Workspace* a_workspace);
    
protected:

public:
    ~FileHandler();

    static void SetFileHandle(const FileTextureHandle& a_handle);

    static void Init(AssetLibrary* a_assets, RuntimeManager* a_runtime, RuntimeStorage* a_storage, Workspace* a_workspace);
    static void Destroy();

    static void GetFileData(const std::filesystem::path& a_path, FileCallback** a_openCallback, FileCallback** a_dragCallback, GLuint* a_texture);
};