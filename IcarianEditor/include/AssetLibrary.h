// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <filesystem>
#include <vector>

class Project;
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
    F(Shader) \
    F(UI) \
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

struct FileAlias
{
    std::filesystem::path SourceFile;
    std::filesystem::path AliasFile;
};

struct Asset
{
    static constexpr uint32_t ForceWriteBit = 0;

    std::filesystem::file_time_type ModifiedTime;
    std::filesystem::path Path;
    e_AssetType AssetType;
    uint32_t Size;
    uint8_t* Data;
    uint8_t Flags;
};

class AssetLibrary
{
private:
    static constexpr uint32_t ForceSerializeBit = 0;

    std::vector<Asset> m_assets;
    uint8_t            m_flags;

protected:

public:
    AssetLibrary();
    ~AssetLibrary();

    void CreateDef(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);

    void WriteDef(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);
    void WriteScene(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);

    bool ShouldRefresh(const std::filesystem::path& a_workingDir) const;
    bool ShouldSerialize();

    void Refresh(const std::filesystem::path& a_workingDir);
    void BuildDirectory(const std::filesystem::path& a_path, const Project* a_project) const;

    std::vector<std::filesystem::path> GetAssetPathWithExtension(const std::string_view& a_ext);

    e_AssetType GetAssetType(const std::filesystem::path& a_path);
    e_AssetType GetAssetType(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path);

    void WriteAsset(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);
    void GetAsset(const std::filesystem::path& a_path, uint32_t* a_size, const uint8_t** a_data, e_AssetType* a_type = nullptr);
    void GetAsset(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path, uint32_t* a_size, const uint8_t** a_data, e_AssetType* a_type = nullptr);

    void Serialize(const Project* a_project);
};

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