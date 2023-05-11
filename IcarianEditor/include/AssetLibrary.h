#pragma once

#include <filesystem>
#include <list>
#include <string>

class RuntimeManager;

enum e_AssetType
{
    AssetType_Null = -1,
    AssetType_About,
    AssetType_Def,
    AssetType_Model,
    AssetType_Scene,
    AssetType_Scribe,
    AssetType_Script,
    AssetType_Other
};

struct Asset
{
    std::filesystem::path Path;
    e_AssetType AssetType;
    uint32_t Size;
    char* Data;
};

class AssetLibrary
{
private:
    RuntimeManager*  m_runtime;

    std::list<Asset> m_assets;

    void TraverseTree(const std::filesystem::path& a_path, const std::filesystem::path& a_workingDir);

protected:

public:
    AssetLibrary(RuntimeManager* a_runtime);
    ~AssetLibrary();

    static std::filesystem::path GetRelativePath(const std::filesystem::path& a_relative, const std::filesystem::path& a_path);

    void WriteDef(const std::filesystem::path& a_path, uint32_t a_size, char* a_data);
    void WriteScene(const std::filesystem::path& a_path, uint32_t a_size, char* a_data);

    void Refresh(const std::filesystem::path& a_workingDir);
    void BuildDirectory(const std::filesystem::path& a_path) const;

    void GetAsset(const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data);
    void GetAsset(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data);

    void Serialize(const std::filesystem::path& a_workingDir) const;
};