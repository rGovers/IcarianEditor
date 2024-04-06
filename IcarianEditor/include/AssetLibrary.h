#pragma once

#include <filesystem>
#include <vector>

class RuntimeManager;

enum e_AssetType
{
    AssetType_Null = -1,
    AssetType_About,
    AssetType_Assembly,
    AssetType_Def,
    AssetType_Model,
    AssetType_Scene,
    AssetType_Scribe,
    AssetType_Script,
    AssetType_Other
};

constexpr static const char* AssetTypeStrings[] = 
{
    "About",
    "Assembly",
    "Def",
    "Model",
    "Scene",
    "Scribe",
    "Script",
    "Other"
};

struct Asset
{
    std::filesystem::file_time_type ModifiedTime;
    std::filesystem::path Path;
    e_AssetType AssetType;
    uint32_t Size;
    char* Data;
};

class AssetLibrary
{
private:
    RuntimeManager*    m_runtime;

    std::vector<Asset> m_assets;

protected:

public:
    AssetLibrary(RuntimeManager* a_runtime);
    ~AssetLibrary();

    void WriteDef(const std::filesystem::path& a_path, uint32_t a_size, char* a_data);
    void WriteScene(const std::filesystem::path& a_path, uint32_t a_size, char* a_data);

    bool ShouldRefresh(const std::filesystem::path& a_workingDir) const;

    void Refresh(const std::filesystem::path& a_workingDir);
    void BuildDirectory(const std::filesystem::path& a_path) const;

    void GetAsset(const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data, e_AssetType* a_type = nullptr);
    void GetAsset(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data, e_AssetType* a_type = nullptr);

    void Serialize(const std::filesystem::path& a_workingDir);
};