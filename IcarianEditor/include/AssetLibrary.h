#pragma once

#include <filesystem>
#include <vector>

class RuntimeManager;

#define ASSETTYPE_TABLE(F) \
    F(About) \
    F(Assembly) \
    F(Def) \
    F(Texture) \
    F(Model) \
    F(Scene) \
    F(Scribe) \
    F(Script) \
    F(Other)

#define ASSETTYPE_ENUM_DEFINITION(name) AssetType_##name,
#define ASSETTYPE_STRING_DEFINITION(name) #name,

enum e_AssetType
{
    AssetType_Null = -1,
    
    ASSETTYPE_TABLE(ASSETTYPE_ENUM_DEFINITION)
};

constexpr static const char* AssetTypeStrings[] = 
{
    ASSETTYPE_TABLE(ASSETTYPE_STRING_DEFINITION)
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

    e_AssetType GetAssetType(const std::filesystem::path& a_path);
    e_AssetType GetAssetType(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path);

    void GetAsset(const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data, e_AssetType* a_type = nullptr);
    void GetAsset(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data, e_AssetType* a_type = nullptr);

    void Serialize(const std::filesystem::path& a_workingDir);
};