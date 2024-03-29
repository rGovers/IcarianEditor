#include "AssetLibrary.h"

#include <cstddef>
#include <cstdint>
#include <fstream>

#include "EditorConfig.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "IO.h"
#include "Logger.h"
#include "Runtime/RuntimeManager.h"
#include "mono/metadata/appdomain.h"
#include "mono/metadata/object-forward.h"
#include "mono/metadata/object.h"

#include "EditorDefLibraryInterop.h"
#include "EditorSceneInterop.h"

static AssetLibrary* Instance = nullptr;

#define ASSETLIBRARY_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) BIND_FUNCTION(a_runtime, namespace, klass, name);

EDITORDEFLIBRARY_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
EDITORSCENE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

AssetLibrary::AssetLibrary(RuntimeManager* a_runtime)
{
    m_runtime = a_runtime;
    
    EDITORDEFLIBRARY_EXPORT_TABLE(ASSETLIBRARY_RUNTIME_ATTACH);
    EDITORSCENE_EXPORT_TABLE(ASSETLIBRARY_RUNTIME_ATTACH);

    Instance = this;
}
AssetLibrary::~AssetLibrary()
{
    for (const Asset& asset : m_assets)
    {
        if (asset.Data != nullptr)
        {
            delete[] asset.Data;
        }
    }
}

static void TraverseTree(std::vector<Asset>* a_assets, const std::filesystem::path& a_path, const std::filesystem::path& a_workingDir)
{
    for (const auto& iter : std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied))
    {
        const std::filesystem::path path = iter.path();

        if (iter.is_regular_file())
        {
            Asset asset;

            asset.Path = IO::GetRelativePath(a_workingDir, path);
            asset.ModifiedTime = std::filesystem::last_write_time(path);

            const std::filesystem::path ext = asset.Path.extension();
            const std::filesystem::path name = asset.Path.filename();

            if (ext == ".cs")
            {
                asset.AssetType = AssetType_Script;
            }
            else if (ext == ".def")
            {
                asset.AssetType = AssetType_Def;
            }
            else if (ext == ".scrb")
            {
                asset.AssetType = AssetType_Scribe;
            }
            else if (ext == ".iscene")
            {
                asset.AssetType = AssetType_Scene;
            }
            else if (ext == ".dae" || ext == ".fbx" || ext == ".obj")
            {
                asset.AssetType = AssetType_Model;
            }
            else if (name == "about.xml")
            {
                asset.AssetType = AssetType_About;
            }
            else
            {
                asset.AssetType = AssetType_Other;
            }

            asset.Data = nullptr;
            asset.Size = 0;

            a_assets->emplace_back(asset);
        }
        else if (iter.is_directory())
        {
            TraverseTree(a_assets, path, a_workingDir);
        }
    }
}

static void ReadAssets(std::vector<Asset>* a_assets, const std::filesystem::path& a_workingDir)
{
    for (Asset& asset : *a_assets)
    {
        if (asset.Data != nullptr)
        {
            delete[] asset.Data;
            asset.Data = nullptr;
        }

        asset.Size = 0;

        const std::filesystem::path p = a_workingDir / asset.Path;

        std::ifstream file = std::ifstream(p, std::ios::binary);
        if (file.good() && file.is_open())
        {
            file.ignore(std::numeric_limits<std::streamsize>::max());
            asset.Size = (uint32_t)file.gcount();
            file.clear();
            file.seekg(0, std::ios::beg);

            asset.Data = new char[asset.Size];
            file.read(asset.Data, (std::streamsize)asset.Size);
        }
    }
}

void AssetLibrary::WriteDef(const std::filesystem::path& a_path, uint32_t a_size, char* a_data)
{
    for (Asset& a : m_assets)
    {
        if (a.AssetType != AssetType_Def)
        {
            continue;
        }

        if (a.Path == a_path)
        {
            if (a.Data != nullptr)
            {
                delete[] a.Data;
            }

            a.Data = a_data;
            a.Size = a_size;

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "Def not found");
}

void AssetLibrary::WriteScene(const std::filesystem::path& a_path, uint32_t a_size, char* a_data)
{
    for (Asset& a : m_assets)
    {
        if (a.AssetType != AssetType_Scene)
        {
            continue;
        }

        if (a.Path == a_path)
        {
            if (a.Data != nullptr)
            {
                delete[] a.Data;
            }

            a.Data = a_data;
            a.Size = a_size;

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "Scene not found");
}

bool AssetLibrary::ShouldRefresh(const std::filesystem::path& a_workingDir) const
{
    std::vector<Asset> assets;

    const std::filesystem::path p = a_workingDir / "Project";
    TraverseTree(&assets, p, p);

    for (const Asset& internalAsset : assets)
    {
        for (const Asset& externalAsset : m_assets)
        {
            if (internalAsset.Path == externalAsset.Path)
            {
                if (externalAsset.ModifiedTime > internalAsset.ModifiedTime)
                {
                    return true;
                }

                goto Next;
            }
        }

        // Not found, new asset.
        return true;
Next:;
    }

    return false;
}

void AssetLibrary::Refresh(const std::filesystem::path& a_workingDir)
{
    for (const Asset& asset : m_assets)
    {
        if (asset.Data != nullptr)
        {
            delete[] asset.Data;
        }
    }

    m_assets.clear();

    const std::filesystem::path p = a_workingDir / "Project";

    TraverseTree(&m_assets, p, p);
    ReadAssets(&m_assets, p);

    if (!m_runtime->IsBuilt() || !m_runtime->IsRunning())
    {
        return;
    }

    std::vector<const char*> defAssets;
    std::vector<uint32_t> defSizes;
    std::vector<std::filesystem::path> defPaths;

    std::vector<const char*> sceneAssets;
    std::vector<uint32_t> sceneSizes;
    std::vector<std::filesystem::path> scenePaths;

    for (const Asset& asset : m_assets)
    {
        switch (asset.AssetType)
        {
        case AssetType_Def:
        {
            defAssets.emplace_back(asset.Data);
            defSizes.emplace_back((uint32_t)asset.Size);
            defPaths.emplace_back(asset.Path);

            break;
        }
        case AssetType_Scene:
        {
            sceneAssets.emplace_back(asset.Data);
            sceneSizes.emplace_back((uint32_t)asset.Size);
            scenePaths.emplace_back(asset.Path);

            break;
        }
        default:
        {
            continue;
        }
        }
    }

    MonoDomain* editorDomain = m_runtime->GetEditorDomain();

    MonoClass* stringClass = mono_get_string_class();
    MonoClass* byteClass = mono_get_byte_class();
    MonoClass* arrayClass = mono_get_array_class();

    const uint32_t defSize = (uint32_t)defAssets.size();

    MonoArray* defDataArray = mono_array_new(editorDomain, arrayClass, (uintptr_t)defSize);
    MonoArray* defPathArray = mono_array_new(editorDomain, stringClass, (uintptr_t)defSize);
    for (uint32_t i = 0; i < defSize; ++i)
    {
        MonoArray* data = mono_array_new(editorDomain, byteClass, defSizes[i]);
        for (uint32_t j = 0; j < defSizes[i]; ++j)
        {
            mono_array_set(data, mono_byte, j, (mono_byte)defAssets[i][j]);
        }

        mono_array_set(defDataArray, MonoArray*, i, data);
        mono_array_set(defPathArray, MonoString*, i, mono_string_from_utf32((mono_unichar4*)defPaths[i].u32string().c_str()));
    }

    void* defArgs[] = 
    {
        defDataArray,
        defPathArray
    };

    m_runtime->ExecFunction("IcarianEditor", "EditorDefLibrary", ":Load(byte[][],string[])", defArgs);

    const uint32_t sceneSize = (uint32_t)sceneAssets.size();

    MonoArray* sceneDataArray = mono_array_new(editorDomain, arrayClass, (uintptr_t)sceneSize);
    MonoArray* scenePathArray = mono_array_new(editorDomain, stringClass, (uintptr_t)sceneSize);

    for (uint32_t i = 0; i < sceneSize; ++i)
    {
        MonoArray* data = mono_array_new(editorDomain, byteClass, sceneSizes[i]);
        for (uint32_t j = 0; j < sceneSizes[i]; ++j)
        {
            mono_array_set(data, mono_byte, j, (mono_byte)sceneAssets[i][j]);
        }

        mono_array_set(sceneDataArray, MonoArray*, i, data);
        mono_array_set(scenePathArray, MonoString*, i, mono_string_from_utf32((mono_unichar4*)scenePaths[i].u32string().c_str()));
    }

    void* sceneArgs[] =
    {
        sceneDataArray,
        scenePathArray
    };

    m_runtime->ExecFunction("IcarianEditor", "EditorScene", ":LoadScenes(byte[][],string[])", sceneArgs);
}
void AssetLibrary::BuildDirectory(const std::filesystem::path& a_path) const
{
    for (const Asset& asset : m_assets)
    {
        switch (asset.AssetType)
        {
        case AssetType_About:
        {
            const std::filesystem::path p = a_path / "Core" / "about.xml";

            std::filesystem::create_directories(p.parent_path());

            std::ofstream file = std::ofstream(p, std::ios_base::binary);
            if (file.good() && file.is_open())
            {
                file.write(asset.Data, asset.Size);

                file.close();
            }
            else
            {
                Logger::Warning("Failed writing about: " + p.string());
            }

            break;
        }
        case AssetType_Def:
        {
            std::filesystem::path p = a_path / "Core" / "Defs" / asset.Path;
            
            if (asset.Path.begin()->string() == "Defs")
            {
                p = a_path / "Core" / asset.Path;
            }

            std::filesystem::create_directories(p.parent_path());

            std::ofstream file = std::ofstream(p, std::ios_base::binary);
            if (file.good() && file.is_open())
            {
                file.write(asset.Data, asset.Size);

                file.close();
            }
            else
            {
                Logger::Warning("Failed writing build def: " + p.string());
            }

            break;
        }
        case AssetType_Scene:
        {
            std::filesystem::path p = a_path / "Core" / "Scenes" / asset.Path.filename();

            std::filesystem::create_directories(p.parent_path());

            std::ofstream file = std::ofstream(p, std::ios_base::binary);
            if (file.good() && file.is_open())
            {
                file.write(asset.Data, asset.Size);

                file.close();
            }
            else
            {
                Logger::Warning("Failed to write scene: " + p.string());
            }

            break;
        }
        case AssetType_Script:
        {
            continue;
        }
        case AssetType_Scribe:
        {
            std::filesystem::path p = a_path / "Core" / "Scribe" / asset.Path;

            if (asset.Path.begin()->string() == "Scribe")
            {
                p = a_path / "Core" / asset.Path;
            }

            std::filesystem::create_directories(p.parent_path());

            std::ofstream file = std::ofstream(p, std::ios_base::binary);
            if (file.good() && file.is_open())
            {
                file.write(asset.Data, asset.Size);

                file.close();
            }
            else
            {
                Logger::Warning("Failed writing scribe file: " + p.string());
            }

            break;
        }
        case AssetType_Model:
        case AssetType_Other:
        {
            const std::filesystem::path p = a_path / "Core" / "Assets" / asset.Path;

            std::filesystem::create_directories(p.parent_path());
            
            std::ofstream file = std::ofstream(p, std::ios_base::binary);
            if (file.good() && file.is_open())
            {
                file.write(asset.Data, asset.Size);

                file.close();
            }
            else
            {
                Logger::Warning("Failed writing asset: " + p.string());
            }

            break;
        }
        default:
        {
            Logger::Warning("Invalid Asset: " + asset.Path.string());
        }
        }
    }
}

void AssetLibrary::GetAsset(const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data)
{
    *a_size = 0;
    *a_data = nullptr;

    for (const Asset& asset : m_assets)
    {
        if (a_path == asset.Path)
        {
            *a_size = asset.Size;
            *a_data = asset.Data;

            return;
        }
    }
}
void AssetLibrary::GetAsset(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path, uint32_t* a_size, const char** a_data)
{
    const std::filesystem::path rPath = IO::GetRelativePath(a_workingDir, a_path);

    GetAsset(rPath, a_size, a_data);
}

void AssetLibrary::Serialize(const std::filesystem::path& a_workingDir)
{
    std::vector<Asset> defs;

    for (const Asset& asset : m_assets)
    {
        if (asset.AssetType == AssetType_Def)
        {
            defs.emplace_back(asset);
        }
    }

    const std::filesystem::path pPath = a_workingDir / "Project";

    m_runtime->ExecFunction("IcarianEditor", "EditorDefLibrary", ":SerializeDefs()", nullptr);
    m_runtime->ExecFunction("IcarianEditor", "EditorScene", ":Serialize()", nullptr);

    const e_DefEditor defEditor = EditorConfig::GetDefEditor();

    for (const Asset& a : m_assets)
    {
        const std::filesystem::path p = pPath / a.Path;

        const e_AssetType type = a.AssetType;

        switch (type) 
        {
        case AssetType_Script:
        {
            continue;
        }
        case AssetType_Def:
        {
            if (defEditor != DefEditor_Editor)
            {
                continue;
            }

            break;
        }
        default:
        {
            break;
        }
        }

        std::ofstream file = std::ofstream(p, std::ios::binary);
        if (file.good() && file.is_open())
        {
            file.write(a.Data, a.Size);
        }
    }

    const std::filesystem::file_time_type time = std::filesystem::file_time_type::clock::now();

    for (Asset& asset : m_assets)
    {
        const e_AssetType type = asset.AssetType;

        switch (type)
        {
        case AssetType_Script:
        {
            continue;
        }
        case AssetType_Def:
        {
            if (defEditor != DefEditor_Editor)
            {
                continue;
            }

            break;
        }
        default:
        {
            break;
        }
        }

        asset.ModifiedTime = time;
    }
}