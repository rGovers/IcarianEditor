// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <filesystem>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "Core/SharedMemoryBuffer.h"

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

struct AssetCommand
{
    std::mutex Lock;
    uint32_t ID;
    IcarianCore::SharedMemoryBuffer* CommandBuffer;
    IcarianCore::SharedMemoryBuffer* DataBuffer;
};

// TODO: Can probably clean up this class a bit been just hacking stuff on as needed and patching can probably tear out and clean up
// TODO: I can probably break this out to a seperate process from the editor with a watchdog that saves to a backup archive in the event of an unexpected close
class AssetLibrary
{
private:
#ifndef WIN32
    static constexpr char CommandBufferName[] = "IcarianEditorAssetCommand";
    static constexpr char DataBufferName[] = "IcarianEditorAssetData";

    static constexpr uint32_t SharedBufferSize = 10 << 10;
#endif

    static constexpr uint32_t ForceSerializeBit = 0;

    // TODO: Probably will need multiple down the line but works for now
    std::shared_mutex                m_lock;
    std::mutex                       m_commandBufferLock;
    std::thread                      m_thread;

    AssetCommand**                   m_commandBuffers;
    uint32_t                         m_commandBufferCount;
    uint32_t                         m_commandID;

    std::vector<Asset>               m_assets;
    uint8_t                          m_flags;

    volatile bool                    m_shutdown;
    volatile bool                    m_join;

    AssetLibrary();

    static void RunBuffer();

protected:

public:
    ~AssetLibrary();

    static void Init();
    static void Destroy();

    static uint32_t CreateAssetCommandBuffer();
    static void DestroyAssetCommandBuffer(uint32_t a_id);

    static void CreateDef(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);

    static void WriteDef(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);
    static void WriteScene(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);

    static bool ShouldRefresh(const std::filesystem::path& a_workingDir);
    static bool ShouldSerialize();

    static void Refresh(const std::filesystem::path& a_workingDir);
    static void BuildDirectory(const std::filesystem::path& a_path, const Project* a_project);

    static std::vector<std::filesystem::path> GetAssetPathWithExtension(const std::string_view& a_ext);

    static e_AssetType GetAssetType(const std::filesystem::path& a_path);

    static void WriteAsset(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data);
    static void GetAsset(const std::filesystem::path& a_path, uint32_t* a_size, const uint8_t** a_data, e_AssetType* a_type = nullptr);

    static void Serialize(const Project* a_project);
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
