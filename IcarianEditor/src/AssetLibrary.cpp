#include "AssetLibrary.h"

#include <cstdint>
#include <fstream>
#include <glad/glad.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/object.h>
#include <mono/metadata/object-forward.h>
#include <stb_image.h>
#include <thread>
#include <tinyxml2.h>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "EditorConfig.h"
#include "IO.h"
#include "KtxHelpers.h"
#include "Logger.h"
#include "Project.h"
#include "Runtime/RuntimeManager.h"

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

template<typename T>
constexpr static T ToWInt(const unsigned char* a_data)
{
    constexpr uint32_t Size = sizeof(T);

    T val = 0;
    for (T i = 0; i < Size; ++i)
    {
        val |= (T)(a_data[i]) << (i * 8);
    }

    return val;
}

// Microsoft magic bullshit
// Reference: https://learn.microsoft.com/en-us/cpp/dotnet/how-to-determine-if-an-image-is-native-or-clr?view=msvc-170
constexpr static bool IsManagedAssembly(const unsigned char* a_data, uint32_t a_length)
{
    if (a_length < 64)
    {
        return false;
    }

    constexpr uint16_t MagicDLLNumber = 0x5A4D;
    constexpr uint32_t MagicNTAddress = 0x00004550;
    
    const bool isExe = ToWInt<uint16_t>(a_data) == MagicDLLNumber;
    if (!isExe)
    {
        return false;
    }

    const uint32_t winNTHdr = ToWInt<uint32_t>(a_data + 60);
    const bool validAddress = ToWInt<uint32_t>(a_data + winNTHdr) == MagicNTAddress;
    if (!validAddress)
    {
        return false;
    }

    const uint32_t lightningAddr = winNTHdr + 24 + 208;
    for (uint32_t i = 0; i < 8; ++i)
    {
        if (a_data[i + lightningAddr])
        {
            return true;
        }
    }

    return false;
}

static void TraverseTree(std::vector<Asset>* a_assets, const std::filesystem::path& a_path, const std::filesystem::path& a_workingDir)
{
    for (const auto& iter : std::filesystem::directory_iterator(a_path, std::filesystem::directory_options::skip_permission_denied))
    {
        const std::filesystem::path path = iter.path();

        if (iter.is_regular_file())
        {
            Asset asset = 
            { 
                .ModifiedTime = std::filesystem::last_write_time(path),
                .Path = IO::GetRelativePath(a_workingDir, path),
            };

            const std::filesystem::path ext = asset.Path.extension();
            const std::filesystem::path name = asset.Path.filename();

            const std::string extStr = ext.string();

            switch (StringHash<uint32_t>(extStr.c_str())) 
            {
            case StringHash<uint32_t>(".cs"):
            {
                asset.AssetType = AssetType_Script;

                break;
            }
            case StringHash<uint32_t>(".fvert"):
            case StringHash<uint32_t>(".fpix"):
            case StringHash<uint32_t>(".ffrag"):
            {
                asset.AssetType = AssetType_Shader;

                break;
            }
            case StringHash<uint32_t>(".dll"):
            case StringHash<uint32_t>(".so"):
            {
                asset.AssetType = AssetType_Assembly;

                break;
            }
            case StringHash<uint32_t>(".def"):
            {
                asset.AssetType = AssetType_Def;

                break;
            }
            case StringHash<uint32_t>(".ui"):
            {
                asset.AssetType = AssetType_UI;

                break;
            }
            case StringHash<uint32_t>(".scrb"):
            {
                asset.AssetType = AssetType_Scribe;

                break;
            }
            case StringHash<uint32_t>(".iscene"):
            {
                asset.AssetType = AssetType_Scene;

                break;
            }
            case StringHash<uint32_t>(".png"):
            case StringHash<uint32_t>(".ktx2"):
            {
                asset.AssetType = AssetType_Texture;

                break;
            }
            case StringHash<uint32_t>(".obj"):
            case StringHash<uint32_t>(".dae"):
            case StringHash<uint32_t>(".fbx"):
            case StringHash<uint32_t>(".glb"):
            case StringHash<uint32_t>(".gltf"):
            {
                asset.AssetType = AssetType_Model;

                break;
            }
            default:
            {
                if (name == "about.xml")
                {
                    asset.AssetType = AssetType_About;
                }
                else
                {
                    asset.AssetType = AssetType_Other;
                }

                break;
            }
            }

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

            asset.Data = new uint8_t[asset.Size];
            file.read((char*)asset.Data, (std::streamsize)asset.Size);
        }
    }
}

void AssetLibrary::WriteDef(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data)
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

void AssetLibrary::WriteScene(const std::filesystem::path& a_path, uint32_t a_size, uint8_t* a_data)
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

    for (const Asset& externalAsset : assets)
    {
        for (const Asset& internalAsset : m_assets)
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

    std::vector<const uint8_t*> defAssets;
    std::vector<uint32_t> defSizes;
    std::vector<std::filesystem::path> defPaths;

    std::vector<const uint8_t*> sceneAssets;
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

static bool ShouldWriteFile(const std::filesystem::path& a_path, const std::filesystem::file_time_type& a_modifiedTime)
{   
    if (!std::filesystem::exists(a_path))
    {
        return true;
    }

    if (std::filesystem::last_write_time(a_path) < a_modifiedTime)
    {
        return true;
    }

    return false;
}

static void WriteData(const std::filesystem::path& a_path, const uint8_t* a_data, uint32_t a_size, const std::filesystem::file_time_type& a_modifiedTime)
{
    const std::filesystem::path dir = a_path.parent_path();
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }

    if (ShouldWriteFile(a_path, a_modifiedTime))
    {
        std::ofstream file = std::ofstream(a_path, std::ios_base::binary);
        if (file.good() && file.is_open()) 
        {
            file.write((char*)a_data, (std::streamsize)a_size);
        } 
        else 
        {
            Logger::Warning("Failed writing file: " + a_path.string());
        }
    }
}

constexpr static ktx_uint32_t VKFormatFromSTBIChannels(int a_channelCount)
{
    switch (a_channelCount)
    {
    case 1:
    {
        return KTX_VKFORMAT_R8_UNORM;
    }
    case 2:
    {
        return KTX_VKFORMAT_R8G8_UNORM;
    }
    case 3:
    {
        return KTX_VKFORMAT_R8G8B8_UNORM;
    }
    case 4:
    {
        return KTX_VKFORMAT_R8G8B8A8_UNORM;
    }
    }

    ICARIAN_ASSERT(0);

    return KTX_VKFORMAT_R8_SNORM;
}

void AssetLibrary::BuildDirectory(const std::filesystem::path& a_path, const Project* a_project) const
{
    std::vector<FileAlias> fileAliases;

    for (const Asset& asset : m_assets)
    {
        switch (asset.AssetType)
        {
        case AssetType_About:
        {
            WriteData(a_path / "Core" / "about.xml", asset.Data, asset.Size, asset.ModifiedTime);

            break;
        }
        case AssetType_Assembly:
        {
            const std::filesystem::path filename = asset.Path.filename();
            const std::filesystem::path ext = asset.Path.extension();
            std::filesystem::path p;

            if (ext == ".dll" && IsManagedAssembly((unsigned char*)asset.Data, asset.Size))
            {
                p = a_path / "Core" / "Assemblies" / filename;
            }
            else
            {
                p = a_path / "Core" / "Assemblies" / "Native" / filename;
            }

            WriteData(p, asset.Data, asset.Size, asset.ModifiedTime);

            break;
        }
        case AssetType_Def:
        {
            std::filesystem::path p = a_path / "Core" / "Defs" / asset.Path;

            if (asset.Path.begin()->string() == "Defs")
            {
                p = a_path / "Core" / asset.Path;
            }

            WriteData(p, asset.Data, asset.Size, asset.ModifiedTime);

            break;
        }
        case AssetType_Scene:
        {
            WriteData(a_path / "Core" / "Scenes" / asset.Path.filename(), asset.Data, asset.Size, asset.ModifiedTime);

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

            WriteData(p, asset.Data, asset.Size, asset.ModifiedTime);

            break;
        }
        case AssetType_Texture:
        {
            const std::filesystem::path basePath = a_path / "Core" / "Assets" / asset.Path;
            const std::filesystem::path ext = asset.Path.extension();

            if (a_project->ConvertKTX() && ext != ".ktx2")
            {
                const std::filesystem::path filename = basePath.stem();
                const std::filesystem::path dir = basePath.parent_path();

                const std::filesystem::path assetPath = asset.Path.parent_path();
                const std::string ktxFileName = filename.string() + ".ktx2";

                const std::filesystem::path writePath = dir / ktxFileName;
                if (ShouldWriteFile(writePath, asset.ModifiedTime))
                {
                    if (!std::filesystem::exists(dir))
                    {
                        std::filesystem::create_directories(dir);
                    }

                    if (ext == ".png")
                    {
                        int width;
                        int height;
                        int channels;
                        stbi_uc* data = stbi_load_from_memory((stbi_uc*)asset.Data, (int)asset.Size, &width, &height, &channels, 0);
                        if (data != NULL)
                        {
                            IDEFER(stbi_image_free(data));

                            const uint64_t size = (uint64_t)width * height * channels;

                            ktxTextureCreateInfo createInfo = 
                            {
                                .vkFormat = VKFormatFromSTBIChannels(channels),
                                .baseWidth = (ktx_uint32_t)width,
                                .baseHeight = (ktx_uint32_t)height,
                                .baseDepth = 1,
                                .numDimensions = 2,
                                .numLevels = 1,
                                .numLayers = 1,
                                .numFaces = 1,
                                .generateMipmaps = KTX_TRUE
                            };

                            ktxTexture2* ktxTex;
                            ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktxTex);
                            IDEFER(ktxTexture_Destroy((ktxTexture*)ktxTex));

                            ICARIAN_ASSERT_R(ktxTexture_SetImageFromMemory((ktxTexture*)ktxTex, 0, 0, 0, data, (ktx_size_t)size) == KTX_SUCCESS);

                            ktxBasisParams basisParam =  { 0 };
                            basisParam.structSize = sizeof(basisParam);
                            basisParam.compressionLevel = KTX_ETC1S_DEFAULT_COMPRESSION_LEVEL;
                            basisParam.threadCount = std::thread::hardware_concurrency() / 2;

                            ICARIAN_ASSERT_R(ktxTexture2_CompressBasisEx(ktxTex, &basisParam) == KTX_SUCCESS);

                            ktx_uint8_t* ktxDat;
                            ktx_size_t ktxDatSize;
                            ICARIAN_ASSERT_R(ktxTexture_WriteToMemory((ktxTexture*)ktxTex, &ktxDat, &ktxDatSize) == KTX_SUCCESS);

                            WriteData(writePath, (uint8_t*)ktxDat, (uint32_t)ktxDatSize, asset.ModifiedTime);
                        }
                    }
                }

                const FileAlias alias =
                {
                    .SourceFile = asset.Path,
                    .AliasFile = assetPath / ktxFileName
                };

                fileAliases.emplace_back(alias);
            }
            else
            {
                WriteData(basePath, asset.Data, asset.Size, asset.ModifiedTime);
            }

            break;
        }
        case AssetType_Model:
        case AssetType_Shader:
        case AssetType_UI:
        case AssetType_Other:
        {
            WriteData(a_path / "Core" / "Assets" / asset.Path, asset.Data, asset.Size, asset.ModifiedTime);

            break;
        }
        default:
        {
            Logger::Warning("Invalid Asset: " + asset.Path.string());
        }
        }
    }

    if (!fileAliases.empty())
    {
        tinyxml2::XMLDocument doc;
        doc.InsertEndChild(doc.NewDeclaration());
        tinyxml2::XMLElement* rootElement = doc.NewElement("AliasList");
        doc.InsertEndChild(rootElement);

        for (const FileAlias& a : fileAliases)
        {
            tinyxml2::XMLElement* aliasElement = doc.NewElement("Alias");
            rootElement->InsertEndChild(aliasElement);

            const std::string srcStr = a.SourceFile.string();
            const std::string dstStr = a.AliasFile.string();

            tinyxml2::XMLElement* sourceElement = doc.NewElement("Source");
            aliasElement->InsertEndChild(sourceElement);
            sourceElement->SetText(srcStr.c_str());

            tinyxml2::XMLElement* destinationElement = doc.NewElement("Destination");
            aliasElement->InsertEndChild(destinationElement);
            destinationElement->SetText(dstStr.c_str());
        }

        const std::filesystem::path aliasPath = a_path / "Core" / "alias.xml";
        const std::string aliasPathStr = aliasPath.string();

        doc.SaveFile(aliasPathStr.c_str());
    }
}

std::vector<std::filesystem::path> AssetLibrary::GetAssetPathWithExtension(const std::string_view& a_ext)
{
    std::vector<std::filesystem::path> paths; 

    for (const Asset& a : m_assets)
    {
        const std::filesystem::path ext = a.Path.extension();
        if (ext == a_ext)
        {
            paths.emplace_back(a.Path);
        }
    }

    return paths;
}

e_AssetType AssetLibrary::GetAssetType(const std::filesystem::path& a_path)
{
    for (const Asset& a : m_assets)
    {
        if (a_path == a.Path)
        {
            return a.AssetType;
        }
    }

    return AssetType_Null;
}
e_AssetType AssetLibrary::GetAssetType(const std::filesystem::path& a_workingPath, const std::filesystem::path& a_path)
{
    const std::filesystem::path rPath = IO::GetRelativePath(a_workingPath, a_path);

    return GetAssetType(rPath);
}

void AssetLibrary::GetAsset(const std::filesystem::path& a_path, uint32_t* a_size, const uint8_t** a_data, e_AssetType* a_type)
{
    *a_size = 0;
    *a_data = nullptr;
    if (a_type != nullptr)
    {
        *a_type = AssetType_Null;
    }

    for (const Asset& asset : m_assets)
    {
        if (a_path == asset.Path)
        {
            *a_size = asset.Size;
            *a_data = asset.Data;
            if (a_type != nullptr)
            {
                *a_type = asset.AssetType;
            }

            return;
        }
    }
}
void AssetLibrary::GetAsset(const std::filesystem::path& a_workingDir, const std::filesystem::path& a_path, uint32_t* a_size, const uint8_t** a_data, e_AssetType* a_type)
{
    const std::filesystem::path rPath = IO::GetRelativePath(a_workingDir, a_path);

    GetAsset(rPath, a_size, a_data, a_type);
}

void AssetLibrary::Serialize(const Project* a_project)
{
    const std::filesystem::path pPath = a_project->GetProjectPath();

    m_runtime->ExecFunction("IcarianEditor", "EditorDefLibrary", ":SerializeDefs()", nullptr);
    m_runtime->ExecFunction("IcarianEditor", "EditorScene", ":Serialize()", nullptr);

    const e_DefEditor defEditor = EditorConfig::GetDefEditor();

    for (Asset& a : m_assets)
    {
        const std::filesystem::path p = pPath / a.Path;

        const e_AssetType type = a.AssetType;

        switch (type) 
        {
        case AssetType_Script:
        case AssetType_Shader:
        case AssetType_Model:
        case AssetType_Texture:
        case AssetType_UI:
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
            file.write((char*)a.Data, a.Size);
        }

        a.ModifiedTime = std::filesystem::file_time_type::clock::now();
    }
}