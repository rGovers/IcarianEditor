#include "AssetLibrary.h"

#include <cstddef>
#include <cstdint>
#include <fstream>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "Runtime/RuntimeManager.h"
#include "mono/metadata/appdomain.h"
#include "mono/metadata/object-forward.h"
#include "mono/metadata/object.h"

static AssetLibrary* Instance = nullptr;

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(AssetProperties, WriteDef), MonoString* a_path, MonoArray* a_data)
{
    mono_unichar4* str = mono_string_to_utf32(a_path);
    const std::filesystem::path p = std::filesystem::path(std::u32string((char32_t*)str));

    const uintptr_t len = mono_array_length(a_data);

    char* data = new char[len];
    for (uintptr_t i = 0; i < len; ++i)
    {
        data[i] = (char)mono_array_get(a_data, mono_byte, i);
    }

    Instance->WriteDef(p, (uint32_t)len, data);

    mono_free(str);
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(SceneData, WriteScene), MonoString* a_path, MonoArray* a_data)
{
    mono_unichar4* str = mono_string_to_utf32(a_path);
    const std::filesystem::path p = std::filesystem::path(std::u32string((char32_t*)str));
    mono_free(str);

    const uintptr_t len = mono_array_length(a_data);

    char* data = new char[len];
    for (uintptr_t i = 0; i < len; ++i)
    {
        data[i] = (char)mono_array_get(a_data, mono_byte, i);
    }

    Instance->WriteScene(p, (uint32_t)len, data);
}

AssetLibrary::AssetLibrary(RuntimeManager* a_runtime)
{
    m_runtime = a_runtime;

    BIND_FUNCTION(m_runtime, IcarianEditor, AssetProperties, WriteDef);
    BIND_FUNCTION(m_runtime, IcarianEditor, SceneData, WriteScene);
    
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

std::filesystem::path AssetLibrary::GetRelativePath(const std::filesystem::path& a_relative, const std::filesystem::path& a_path)
{
    std::filesystem::path tempPath = a_path;
    std::filesystem::path path;

    while (tempPath != a_relative)
    {
        if (path.empty())
        {
            path = tempPath.stem();
            path.replace_extension(tempPath.extension());
        }
        else
        {
            path = tempPath.stem() / path;
        }
        
        tempPath = tempPath.parent_path();
    }

    return path;
}

void AssetLibrary::TraverseTree(const std::filesystem::path& a_path, const std::filesystem::path& a_workingDir)
{
    for (const auto& iter : std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied))
    {
        if (iter.is_regular_file())
        {
            Asset asset;

            asset.Path = GetRelativePath(a_workingDir, iter.path());

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
            else if (ext == ".dae" || ext == ".obj")
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

            // Must resist urge to throw Windows under bus for own mistakes.
            // Forgot to pass second argument and was causing obscure bugs on Windows.
            std::ifstream file = std::ifstream(iter.path(), std::ios::binary);
            if (file.good() && file.is_open())
            {
                // ICARIAN_DEFER_closeIFile(file);
                IDEFER(file.close());

                asset.Size = (uint32_t)std::filesystem::file_size(iter.path());

                asset.Data = new char[asset.Size];
                file.read(asset.Data, asset.Size);

                m_assets.emplace_back(asset);
            }
            else
            {
                Logger::Warning("Failed to open: " + iter.path().string());
            }
        }
        else if (iter.is_directory())
        {
            TraverseTree(iter.path(), a_workingDir);
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

    TraverseTree(p, p);

    if (!m_runtime->IsBuilt())
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

    m_runtime->ExecFunction("IcarianEngine.Definitions", "DefLibrary", ":LoadDefs(byte[][],string[])", defArgs);
    m_runtime->ExecFunction("IcarianEngine.Definitions", "DefLibrary", ":ResolveDefs()", nullptr);

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

    m_runtime->ExecFunction("IcarianEditor", "SceneData", ":LoadScenes(byte[][],string[])", sceneArgs);
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
    const std::filesystem::path rPath = GetRelativePath(a_workingDir, a_path);

    GetAsset(rPath, a_size, a_data);
}

void AssetLibrary::Serialize(const std::filesystem::path& a_workingDir) const
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

    MonoDomain* domain = m_runtime->GetEditorDomain();
    MonoClass* stringClass = mono_get_string_class();

    const uintptr_t count = (uintptr_t)defs.size();
    MonoArray* pathArray = mono_array_new(domain, stringClass, count);
    for (uintptr_t i = 0; i < count; ++i)
    {
        const Asset& a = defs[i];
        
        const std::u32string pStr = a.Path.u32string();
        mono_array_set(pathArray, MonoString*, i, mono_string_new_utf32(domain, (mono_unichar4*)pStr.c_str(), (int32_t)pStr.size()));
    }

    void* args[] =
    {
        pathArray
    };

    m_runtime->ExecFunction("IcarianEditor", "AssetProperties", ":SerializeDefs(string[])", args);
    m_runtime->ExecFunction("IcarianEditor", "SceneData", ":Serialize()", nullptr);

    for (const Asset& a : m_assets)
    {
        const std::filesystem::path p = pPath / a.Path;

        std::ofstream file = std::ofstream(p, std::ios::binary);
        if (file.good() && file.is_open())
        {
            IDEFER(file.close());
            // ICARIAN_DEFER_closeOFile(file);

            file.write(a.Data, a.Size);
        }
    }
}