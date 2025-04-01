// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Runtime/RuntimeStorage.h"

#define STBI_NO_STDIO
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cstring>

#include "AssetLibrary.h"
#include "Core/FlareShader.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "KtxHelpers.h"
#include "Logger.h"
#include "Model.h"
#include "PixelShader.h"
#include "Runtime/RuntimeManager.h"
#include "ShaderStorage.h"
#include "Texture.h"
#include "TextureSampler.h"
#include "VertexShader.h"

#include "EngineSkeletonInteropStructures.h"
#include "EngineAnimationClipInteropStructures.h"

static RuntimeStorage* Instance = nullptr;

#define RUNTIMESTORAGE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.Shaders, VertexShader, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Instance->GenerateVertexShader(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, VertexShader, DestroyShader, { Instance->DestroyVertexShader(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering.Shaders, PixelShader, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Instance->GeneratePixelShader(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, PixelShader, DestroyShader, { Instance->DestroyPixelShader(a_addr); }, uint32_t a_addr) \
    \
    F(RenderProgram, IcarianEngine.Rendering, Material, GetProgramBuffer, { return Instance->GetRenderProgram(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Material, SetProgramBuffer, { Instance->SetRenderProgram(a_addr, a_program); }, uint32_t a_addr, RenderProgram a_program) \
    F(void, IcarianEngine.Rendering, Material, SetTexture, { Instance->SetProgramTexture(a_addr, a_slot, a_samplerAddr); }, uint32_t a_addr, uint32_t a_slot, uint32_t a_samplerAddr) \
    \
    F(void, IcarianEngine.Rendering, Texture, DestroyTexture, { Instance->DestroyTexture(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateTextureSampler, { return Instance->GenerateTextureSampler(a_texture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return 0; }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return 0; }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, IcarianEngine.Rendering, TextureSampler, DestroySampler, { Instance->DestroyTextureSampler(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { Instance->DestroyModel(a_addr); }, uint32_t a_addr) \
    \
    \
    /* Just to stop mono complaining */ \
    F(uint32_t, IcarianEngine.Rendering.Animation, Animator, GenerateBuffer, { return 0; }) \
    F(void, IcarianEngine, Transform, SetTransformMatrix, { }, uint32_t a_addr, MonoArray* a_transform) \

RUNTIMESTORAGE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(void, VertexShader, AddImport,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    Instance->AddVertexImport(key, value);
}, MonoString* a_key, MonoString* a_value)
RUNTIME_FUNCTION(void, PixelShader, AddImport,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    Instance->AddPixelImport(key, value);
}, MonoString* a_key, MonoString* a_value)

RUNTIME_FUNCTION(uint32_t, Material, GenerateProgram, 
{
    RenderProgram program;
    memset(&program, 0, sizeof(RenderProgram));
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.ShadowVertexShader = a_shadowVertexShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (e_CullMode)a_cullMode;
    program.PrimitiveMode = (e_PrimitiveMode)a_primitiveMode;
    program.ColorBlendMode = (e_MaterialBlendMode)a_colorBlendMode;
    program.RenderLayer = a_renderLayer;

    if (a_attributes != NULL)
    {
        program.VertexInputCount = (uint16_t)mono_array_length(a_attributes);
        program.VertexAttributes = new VertexInputAttribute[program.VertexInputCount];

        for (uint16_t i = 0; i < program.VertexInputCount; ++i)
        {
            program.VertexAttributes[i] = mono_array_get(a_attributes, VertexInputAttribute, i);
        }
    }
    else
    {
        program.VertexInputCount = 0;
        program.VertexAttributes = nullptr;
    }

    if (a_uboBuffer != NULL)
    {
        program.UBODataSize = a_uboSize;
        program.UBOData = malloc((size_t)program.UBODataSize);

        memcpy(program.UBOData, a_uboBuffer, (size_t)program.UBODataSize);
    }
    else 
    {
        program.UBODataSize = 0;
        program.UBOData = NULL;
    }

    return Instance->GenerateRenderProgram(program);
}, uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_attributes, uint32_t a_cullMode, uint32_t a_primitiveMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, uint32_t a_uboSize, void* a_uboBuffer)
RUNTIME_FUNCTION(uint32_t, Material, GenerateMeshProgram, 
{
    // List initialisers are being drunk so guess zero and init it is
    RenderProgram program;
    memset(&program, 0, sizeof(RenderProgram));
    program.VertexShader = a_meshShader;
    program.PixelShader = a_pixelShader;
    program.ShadowVertexShader = a_shadowVertexShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (e_CullMode)a_cullMode;
    program.ColorBlendMode = (e_MaterialBlendMode)a_colorBlendMode;
    program.MaterialMode = MaterialMode_BaseMesh;
    program.RenderLayer = a_renderLayer;

    if (a_uboData != NULL)
    {
        program.UBODataSize = a_uboSize;
        program.UBOData = malloc((size_t)program.UBODataSize);

        memcpy(program.UBOData, a_uboData, program.UBODataSize);
    }

    return Instance->GenerateRenderProgram(program);
}, uint32_t a_meshShader, uint32_t a_pixelShader, uint16_t a_vertexStride, uint32_t a_cullMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, uint32_t a_uboSize, void* a_uboData)
RUNTIME_FUNCTION(void, Material, DestroyProgram, 
{
    RenderProgram program = Instance->GetRenderProgram(a_addr);

    IDEFER(
    {
        if (program.VertexAttributes != nullptr)
        {
            delete[] program.VertexAttributes;
        }

        if (program.UBOData != NULL)
        {
            free(program.UBOData);
        }
    });

    Instance->DestroyRenderProgram(a_addr);
}, uint32_t a_addr)

// MSVC workaround
static uint32_t M_Model_GenerateModel(MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride)
{
    const uint32_t vertexCount = (uint32_t)mono_array_length(a_vertices);
    const uint32_t indexCount = (uint32_t)mono_array_length(a_indices);

    const uint32_t vertexSize = vertexCount * a_vertexStride;

    char* vertices = new char[vertexSize];
    IDEFER(delete[] vertices);
    for (uint32_t i = 0; i < vertexSize; ++i)
    {
        vertices[i] = *mono_array_addr_with_size(a_vertices, 1, i);
    }

    uint32_t* indices = new uint32_t[indexCount];
    IDEFER(delete[] indices);
    for (uint32_t i = 0; i < indexCount; ++i)
    {
        indices[i] = mono_array_get(a_indices, uint32_t, i);
    }

    return Instance->GenerateModel(vertices, vertexCount, indices, indexCount, a_vertexStride);
}
RUNTIME_FUNCTION(uint32_t, Model, GenerateModel,
{
    return M_Model_GenerateModel(a_vertices, a_indices, a_vertexStride);
}, MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride)

static void LoadMesh(const aiMesh* a_mesh, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_rSqr)
{
    const bool hasNormals = a_mesh->HasNormals();
    const bool hasTexCoordsA = a_mesh->HasTextureCoords(0);
    const bool hasTexCoordsB = a_mesh->HasTextureCoords(1);
    const bool hasColour = a_mesh->HasVertexColors(0);

    for (uint32_t i = 0; i < a_mesh->mNumVertices; ++i) 
    {
        Vertex v;

        const aiVector3D& pos = a_mesh->mVertices[i];
        v.Position = glm::vec4(pos.x, -pos.y, pos.z, 1.0f);

        *a_rSqr = glm::max(pos.SquareLength(), *a_rSqr);

        if (hasNormals) 
        {
            const aiVector3D& norm = a_mesh->mNormals[i];
            v.Normal = glm::vec3(norm.x, -norm.y, norm.z);
        }

        if (hasTexCoordsA) 
        {
            const aiVector3D& uv = a_mesh->mTextureCoords[0][i];
            v.TexCoordsA = glm::vec2(uv.x, uv.y);
        }

        if (hasTexCoordsB)
        {
            const aiVector3D& uv = a_mesh->mTextureCoords[1][i];
            v.TexCoordsB = glm::vec2(uv.x, uv.y);
        }

        if (hasColour) 
        {
            const aiColor4D& colour = a_mesh->mColors[0][i];
            v.Color = glm::vec4(colour.r, colour.g, colour.b, colour.a);
        }

        a_vertices->emplace_back(v);
    }

    for (uint32_t i = 0; i < a_mesh->mNumFaces; ++i) 
    {
        const aiFace& face = a_mesh->mFaces[i];

        a_indices->emplace_back(face.mIndices[0]);
        a_indices->emplace_back(face.mIndices[2]);
        a_indices->emplace_back(face.mIndices[1]);
    }
}

static void WalkTreeMesh(const aiScene* a_scene, const aiNode* a_node, uint8_t a_data, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radSqr, uint8_t* a_index)
{
    if (a_data == std::numeric_limits<uint8_t>::max())
    {
        for (uint32_t i = 0; i < a_node->mNumMeshes; ++i)
        {
            LoadMesh(a_scene->mMeshes[a_node->mMeshes[i]], a_vertices, a_indices, a_radSqr);
        }
    }
    else 
    {
        if (*a_index > a_data)
        {
            return;
        }

        if (a_data < *a_index + a_node->mNumMeshes)
        {
            LoadMesh(a_scene->mMeshes[a_node->mMeshes[a_data - *a_index]], a_vertices, a_indices, a_radSqr);
            *a_index = std::numeric_limits<uint8_t>::max();

            return;
        }

        *a_index += a_node->mNumMeshes;
    }

    for (uint32_t i = 0; i < a_node->mNumChildren; ++i)
    {
        WalkTreeMesh(a_scene, a_node->mChildren[i], a_data, a_vertices, a_indices, a_radSqr, a_index);
    }
}

RUNTIME_FUNCTION(uint32_t, Model, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();
    const std::string extStr = ext.string();

    AssetLibrary* library = Instance->GetLibrary();
    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".obj"):
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        const uint8_t* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);
        if (size <= 0 || dat == nullptr)
        {
            Logger::Error(std::string("Cannot find mesh file: ") + str);

            break;
        }

        Assimp::Importer importer; 

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, aiProcess_Triangulate | aiProcess_PreTransformVertices, extStr.c_str() + 1);
        ICARIAN_ASSERT(scene != nullptr);
        const aiNode* root = scene->mRootNode;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        float radSqr = 0.0f;
        uint8_t index = 0;
        WalkTreeMesh(scene, root, (uint8_t)a_index, &vertices, &indices, &radSqr, &index);

        if (vertices.empty() || indices.empty() || radSqr <= 0.0f)
        {
            Logger::Warning(std::string("Empty model: ") + str);

            break;
        }

        return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
    }
    default:
    {
        Logger::Error(std::string("Mesh invalid file extension: ") + str);

        break;
    }
    }

    return -1;
}, MonoString* a_path, uint32_t a_index)

static void LoadSkinnedMesh(const aiMesh* a_mesh, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, const std::unordered_map<std::string, int>& a_boneMap, float* a_rSqr)
{
    for (uint32_t i = 0; i < a_mesh->mNumVertices; ++i) 
    {
        SkinnedVertex v;

        const aiVector3D& pos = a_mesh->mVertices[i];
        v.Position = glm::vec4(pos.x, -pos.y, pos.z, 1.0f);

        *a_rSqr = glm::max(pos.SquareLength(), *a_rSqr);

        if (a_mesh->HasNormals()) 
        {
            const aiVector3D& norm = a_mesh->mNormals[i];
            v.Normal = glm::vec3(norm.x, -norm.y, norm.z);
        }

        if (a_mesh->HasTextureCoords(0)) 
        {
            const aiVector3D& uv = a_mesh->mTextureCoords[0][i];
            v.TexCoords = glm::vec2(uv.x, uv.y);
        }

        if (a_mesh->HasVertexColors(0)) 
        {
            const aiColor4D& colour = a_mesh->mColors[0][i];
            v.Color = glm::vec4(colour.r, colour.g, colour.b, colour.a);
        }

        if (a_mesh->HasBones())
        {
            const aiBone* bone = a_mesh->mBones[i];

            const uint32_t weights = glm::min(uint32_t(4), (uint32_t)bone->mNumWeights);
            for (uint32_t j = 0; j < weights; ++j)
            {
                const auto iter = a_boneMap.find(bone->mName.C_Str());
                if (iter == a_boneMap.end())
                {
                    continue;
                }

                v.BoneIndices[j] = iter->second;
                v.BoneWeights[j] = bone->mWeights[j].mWeight;
            }
        }

        a_vertices->emplace_back(v);
    }

    for (uint32_t i = 0; i < a_mesh->mNumFaces; ++i) 
    {
        const aiFace& face = a_mesh->mFaces[i];

        a_indices->emplace_back(face.mIndices[0]);
        a_indices->emplace_back(face.mIndices[2]);
        a_indices->emplace_back(face.mIndices[1]);
    }
}

static void WalkTreeSkinned(const aiScene* a_scene, const aiNode* a_node, uint8_t a_data, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, const std::unordered_map<std::string, int>& a_boneMap, float* a_radSqr, uint8_t* a_index)
{
    if (a_data == std::numeric_limits<uint8_t>::max())
    {
        for (uint32_t i = 0; i < a_node->mNumMeshes; ++i)
        {
            LoadSkinnedMesh(a_scene->mMeshes[a_node->mMeshes[i]], a_vertices, a_indices, a_boneMap, a_radSqr);
        }
    }
    else
    {
        if (*a_index > a_data)
        {
            return;
        }

        if (a_data < *a_index + a_node->mNumMeshes)
        {
            LoadSkinnedMesh(a_scene->mMeshes[a_node->mMeshes[a_data - *a_index]], a_vertices, a_indices, a_boneMap, a_radSqr);
            *a_index = std::numeric_limits<uint8_t>::max();

            return;
        }

        *a_index += a_node->mNumMeshes;
    }

    for (uint32_t i = 0; i < a_node->mNumChildren; ++i)
    {
        WalkTreeSkinned(a_scene, a_node->mChildren[i], a_data, a_vertices, a_indices, a_boneMap, a_radSqr, a_index);
    }
}
RUNTIME_FUNCTION(uint32_t, Model, GenerateSkinnedFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    const std::string extStr = ext.string();

    AssetLibrary* library = Instance->GetLibrary();
    
    switch (StringHash<uint32_t>(extStr.c_str()))
    {
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        const uint8_t* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);
        if (size <= 0 || dat == nullptr)
        {
            Logger::Error(std::string("Cannot find skinned mesh file: ") + str);

            break;
        }

        Assimp::Importer importer; 

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, aiProcess_Triangulate | aiProcess_PreTransformVertices, extStr.c_str() + 1);
        ICARIAN_ASSERT(scene != nullptr);

        if (scene->mNumSkeletons <= 0)
        {
            Logger::Warning(std::string("No skeletons: ") + str);

            break;
        }

        // Hic sunt dracones
        // Compiler is being weird this is a dodgy hack
        // This should not be broken and inserting the arguments directly should work and using a macro should not fix it but it does
        // I think the compiler has had enough of my preprocessor shannigans
        // DO NOT REMOVE otherwise compiler error on GCC and clang
#define MCR_GenerateSkinnedFromFile_MAPKEYS std::string, int
        std::unordered_map<MCR_GenerateSkinnedFromFile_MAPKEYS> boneMap;

        const aiSkeleton* skeleton = scene->mSkeletons[0];
        for (int i = 0; i < skeleton->mNumBones; ++i)
        {
            const aiSkeletonBone* bone = skeleton->mBones[i];
            const std::string name = bone->mNode->mName.C_Str();

            boneMap.emplace(name, i);
        }

        const aiNode* root = scene->mRootNode;

        std::vector<SkinnedVertex> vertices;
        std::vector<uint32_t> indices;
        float radSqr = 0.0f;
        uint8_t index = 0;
        WalkTreeSkinned(scene, root, (uint8_t)a_index, &vertices, &indices, boneMap, &radSqr, &index);

        if (vertices.empty() || indices.empty() || radSqr <= 0)
        {
            Logger::Warning(std::string("Empty Model: ") + str);

            break;
        }

        return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(SkinnedVertex));
    }
    default:
    {
        Logger::Error(std::string("Skinned mesh invalid file extension: ") + str);

        break;
    }
    }

    return -1;
}, MonoString* a_path, uint32_t a_index)

RUNTIME_FUNCTION(RuntimeImportBoneData, Skeleton, LoadBoneData, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();
    const std::string extStr = ext.string();

    RuntimeImportBoneData data = { 0 };

    AssetLibrary* library = Instance->GetLibrary();

    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        const uint8_t* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);
        if (size <= 0 || dat == nullptr)
        {
            Logger::Error(std::string("Cannot find skeleton file: ") + str);

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, 0, extStr.c_str() + 1);
        ICARIAN_ASSERT(scene != nullptr);

        if (scene->mNumSkeletons <= 0)
        {
            break;
        }

        const aiSkeleton* skeleton = scene->mSkeletons[0];

        const uint32_t boneCount = (uint32_t)skeleton->mNumBones;
        if (boneCount <= 0)
        {
            break;
        }

        MonoDomain* domain = mono_domain_get();
        MonoClass* fClass = mono_get_single_class();

        data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)boneCount);
        data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)boneCount);
        data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)boneCount);

        for (uint32_t i = 0; i < boneCount; ++i)
        {
            const aiSkeletonBone* bone = skeleton->mBones[i];

            aiMatrix4x4 bindPose = bone->mOffsetMatrix;
            bindPose.Inverse();

            MonoArray* bindPoseArr = mono_array_new(domain, fClass, 16);
            for (uint32_t j = 0; j < 16; ++j)
            {
                mono_array_set(bindPoseArr, float, j, bindPose[j / 4][j % 4]);
            }

            mono_array_set(data.BindPoses, MonoArray*, i, bindPoseArr);
            mono_array_set(data.Names, MonoString*, i, mono_string_new(domain, bone->mNode->mName.C_Str()));
            mono_array_set(data.Parents, uint32_t, i, (uint32_t)bone->mParent);
        }

        break;
    }
    default:
    {
        Logger::Error(std::string("Skeleton invalid file extension: ") + str);

        break;
    }
    }

    return data;
}, MonoString* a_path)

RUNTIME_FUNCTION(MonoArray*, AnimationClip, LoadExternalAnimationData, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    return Instance->LoadExternalAnimationClip(str);
}, MonoString* a_path)

RUNTIME_FUNCTION(uint32_t, Texture, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));
    
    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    AssetLibrary* library = Instance->GetLibrary();
    if (ext == ".png")
    {
        const uint8_t* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0)
        {
            int width;
            int height;
            int channels;

            stbi_uc* pixels = stbi_load_from_memory((stbi_uc*)dat, (int)size, &width, &height, &channels, STBI_rgb_alpha);
            IDEFER(stbi_image_free(pixels));

            return Instance->GenerateTexture((uint32_t)width, (uint32_t)height, (unsigned char*)pixels);
        }
    }
    else if (ext == ".ktx2")
    {
        const uint8_t* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0)
        {
            ktxTexture2* ktxTex;
            if (ktxTexture2_CreateFromMemory((ktx_uint8_t*)dat, (ktx_size_t)size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex) == KTX_SUCCESS)
            {
                IDEFER(ktxTexture_Destroy((ktxTexture*)ktxTex));

                if (ktxTexture2_NeedsTranscoding(ktxTex))
                {
                    ktxTexture2_TranscodeBasis(ktxTex, KTX_TTF_BC3_RGBA, 0);
                }

                GLuint handle;
                glGenTextures(1, &handle);
                glBindTexture(GL_TEXTURE_2D, handle);
                
                const GLenum glInternalFormat = GLInternalFormatFromKtxVkFormat(ktxTex->vkFormat);
                
                if (ktxTex->isCompressed)
                {
                    const GLsizei size = (GLsizei)ktxTexture_GetImageSize((ktxTexture*)ktxTex, 0);

                    glCompressedTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, ktxTex->baseWidth, ktxTex->baseHeight, 0, size, ktxTex->pData);
                }
                else 
                {
                    const GLenum glFormat = GLFormatFromKtxVkFormat(ktxTex->vkFormat);
                    const GLenum glType = GLTypeFromKtxVkFormat(ktxTex->vkFormat);

                    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, ktxTex->baseWidth, ktxTex->baseHeight, 0, glFormat, glType, ktxTex->pData);
                }
                
                return Instance->GenerateTextureFromHandle((uint32_t)handle);
            }
        }
    }

    return -1;
}, MonoString* a_path)

RuntimeStorage::RuntimeStorage(AssetLibrary* a_assets)
{
    m_assets = a_assets;

    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, VertexShader, AddImport);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, PixelShader, AddImport);

    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateProgram);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateMeshProgram);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, DestroyProgram);

    BIND_FUNCTION(IcarianEngine.Rendering, Model, GenerateModel);
    BIND_FUNCTION(IcarianEngine.Rendering, Model, GenerateFromFile);
    BIND_FUNCTION(IcarianEngine.Rendering, Model, GenerateSkinnedFromFile);

    BIND_FUNCTION(IcarianEngine.Rendering.Animation, Skeleton, LoadBoneData);
    BIND_FUNCTION(IcarianEngine.Rendering.Animation, AnimationClip, LoadExternalAnimationData);

    BIND_FUNCTION(IcarianEngine.Rendering, Texture, GenerateFromFile);

    RUNTIMESTORAGE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    Instance = this;
}
RuntimeStorage::~RuntimeStorage()
{
    Clear();
}

void RuntimeStorage::Clear()
{
    for (const Model* mdl : m_models)
    {
        if (mdl != nullptr)
        {
            delete mdl;
        }
    }
    m_models.clear();

    // May have forgotten about this and been leaking bout 1GB of VRAM.......
    // Opps :D fixed now
    // No idea how this has not caused more issues
    for (const Texture* t : m_textures)
    {
        if (t != nullptr)
        {
            delete t;
        }
    }
    m_textures.clear();

    for (const TextureSamplerBuffer& sampler : m_samplers)
    {
        if (sampler.Data != nullptr)
        {
            delete (TextureSampler*)sampler.Data;
        }
    }
    m_samplers.clear();

    m_vertexImports.clear();

    for (const VertexShader* v : m_vertexShaders)
    {
        if (v != nullptr)
        {
            delete v;
        }
    }
    m_vertexShaders.clear();

    m_pixelImports.clear();

    for (const PixelShader* p : m_pixelShaders)
    {
        if (p != nullptr)
        {
            delete p;
        }
    }
    m_pixelShaders.clear();

    m_renderPrograms.clear();
    m_samplers.clear();
}

uint32_t RuntimeStorage::GenerateVertexShader(const std::filesystem::path& a_path)
{
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".fvert"):
    {
        uint32_t size; 
        const uint8_t* dat;
        m_assets->GetAsset(a_path, &size, &dat);

        std::string error;
        std::vector<ShaderBufferInput> inputs;
        const std::string s = IcarianCore::GLSLFromFlareShader(std::string_view((char*)dat, size), IcarianCore::ShaderPlatform_OpenGL, m_vertexImports, &inputs, &error);
        if (s.empty())
        {
            Logger::Error("Failed to parse VertexShader: " + error + ":" + a_path.string());

            break;
        }

        VertexShader* vShader = VertexShader::GenerateShader(s, inputs.data(), (uint32_t)inputs.size());
        if (vShader == nullptr)
        {
            Logger::Error("Failed to generate VertexShader: " + a_path.string());

            break;
        }

        const uint32_t shaderCount = (uint32_t)m_vertexShaders.size();
        for (uint32_t i = 0; i < shaderCount; ++i)
        {
            if (m_vertexShaders[i] == nullptr)
            {
                m_vertexShaders[i] = vShader;

                return i;
            }
        }

        m_vertexShaders.emplace_back(vShader);

        return shaderCount;
    }
    }

    return -1;
}
void RuntimeStorage::AddVertexImport(const std::string_view& a_key, const std::string_view& a_value)
{
    const std::string key = std::string(a_key);
    const std::string value = std::string(a_value);

    auto iter = m_vertexImports.find(key);
    if (iter != m_vertexImports.end())
    {
        iter->second = value;

        return;
    }

    m_vertexImports.emplace(key, value);
}
void RuntimeStorage::DestroyVertexShader(uint32_t a_addr)
{
    delete m_vertexShaders[a_addr];
    m_vertexShaders[a_addr] = nullptr;
}

uint32_t RuntimeStorage::GeneratePixelShader(const std::filesystem::path& a_path)
{
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    switch (StringHash<uint32_t>(extStr.c_str()))
    {
    case StringHash<uint32_t>(".fpix"):
    case StringHash<uint32_t>(".ffrag"):
    {
        uint32_t size; 
        const uint8_t* dat;
        m_assets->GetAsset(a_path, &size, &dat);

        std::string error;
        std::vector<ShaderBufferInput> inputs;
        const std::string s = IcarianCore::GLSLFromFlareShader(std::string_view((char*)dat, size), IcarianCore::ShaderPlatform_OpenGL, m_pixelImports, &inputs, &error);
        if (s.empty())
        {
            Logger::Error("Failed to parse PixelShader: " + error + ":" + a_path.string());

            break;
        }

        PixelShader* pShader = PixelShader::GenerateShader(s, inputs.data(), (uint32_t)inputs.size());
        if (pShader == nullptr)
        {
            Logger::Error("Failed to generate PixelShader: " + a_path.string());

            break;
        }

        const uint32_t shaderCount = (uint32_t)m_pixelShaders.size();
        for (uint32_t i = 0; i < shaderCount; ++i)
        {
            if (m_pixelShaders[i] == nullptr)
            {
                m_pixelShaders[i] = pShader;

                return i;
            }
        }

        m_pixelShaders.emplace_back(pShader);

        return shaderCount;
    }
    }

    return -1;
}
void RuntimeStorage::AddPixelImport(const std::string_view& a_key, const std::string_view& a_value)
{
    const std::string key = std::string(a_key);
    const std::string value = std::string(a_value);

    auto iter = m_pixelImports.find(key);
    if (iter != m_pixelImports.end())
    {
        iter->second = value;

        return;
    }

    m_pixelImports.emplace(key, value);
}
void RuntimeStorage::DestroyPixelShader(uint32_t a_addr)
{
    delete m_pixelShaders[a_addr];
    m_pixelShaders[a_addr] = nullptr;
}

static void SetRenderBuffers(const Shader* a_shader, const RenderProgram& a_program)
{
    ShaderStorage* storage = (ShaderStorage*)a_program.Data;

    const uint32_t inputCount = a_shader->GetInputCount();
    for (uint32_t i = 0; i < inputCount; ++i)
    {
        const ShaderBufferInput input = a_shader->GetInput(i);

        switch (input.BufferType) 
        {
        case ShaderBufferType_UserUBO:
        {
            storage->SetUserUBO(input.Slot, a_program.UBOData, a_program.UBODataSize);

            break;
        }
        default:
        {
            break;
        }
        }
    }
}
uint32_t RuntimeStorage::GenerateRenderProgram(const RenderProgram& a_program)
{
    RenderProgram program = a_program;

    ShaderStorage* storage = new ShaderStorage(this);
    program.Data = storage;

    switch (program.MaterialMode) 
    {
    case MaterialMode_BaseVertex:
    {
        const VertexShader* vShader = GetVertexShader(program.VertexShader);
        if (vShader != nullptr)
        {
            SetRenderBuffers(vShader, program);
        }

        break;
    }
    case MaterialMode_BaseMesh:
    {
        // TODO: Look into this as it seems only Nvidia supports mesh shaders on OpenGL
        // Not sure if I should move the editor to Vulkan or just have the editor window run in engine as a seperate process
        // Leaning to the latter
        // There is also the possibility that AMD does keep their word and add OpenGL support for Mesh shaders
        // Improving the editor is probably the better choice but over waiting for AMD
        ICARIAN_ASSERT_MSG(0, "Not implemented yet!");

        break;
    }
    default:
    {
        Logger::Warning("Generating with invalid material mode");

        break;
    }
    }

    const PixelShader* pShader = GetPixelShader(program.PixelShader);
    if (pShader != nullptr)
    {
        SetRenderBuffers(pShader, program);
    }

    const uint32_t programCount = (uint32_t)m_renderPrograms.size();
    for (uint32_t i = 0; i < programCount; ++i)
    {
        if (m_renderPrograms[i].Flags & 0b1 << RenderProgram::FreeFlag)
        {            
            m_renderPrograms[i] = program;

            return i;
        }
    }

    m_renderPrograms.emplace_back(program);

    return programCount;
}
void RuntimeStorage::SetProgramTexture(uint32_t a_addr, uint32_t a_slot, uint32_t a_textureAddr)
{
    const RenderProgram& program = m_renderPrograms[a_addr];

    ShaderStorage* storage = (ShaderStorage*)program.Data;
    storage->SetTexture(a_slot, a_textureAddr);
}
void RuntimeStorage::DestroyRenderProgram(uint32_t a_addr)
{
    RenderProgram& program = m_renderPrograms[a_addr];
    if (program.Flags & 0b1 << RenderProgram::DestroyFlag)
    {
        DestroyVertexShader(program.VertexShader);
        DestroyPixelShader(program.PixelShader);
    }

    delete (ShaderStorage*)program.Data;
    program.Data = nullptr;

    program.Flags = 0b1 << RenderProgram::FreeFlag;
}

uint32_t RuntimeStorage::GenerateModel(const void* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride)
{
    Model* mdl = new Model(a_vertices, a_vertexCount, (GLuint*)a_indices, a_indexCount, a_vertexStride);

    const uint32_t modelCount = (uint32_t)m_models.size();
    for (uint32_t i = 0; i < modelCount; ++i)
    {
        if (m_models[i] == nullptr)
        {
            m_models[i] = mdl;

            return i;
        }
    }

    m_models.emplace_back(mdl);

    return modelCount;
}
void RuntimeStorage::DestroyModel(uint32_t a_addr)
{
    delete m_models[a_addr];
    m_models[a_addr] = nullptr;
}

uint32_t RuntimeStorage::GenerateTexture(uint32_t a_width, uint32_t a_height, const unsigned char* a_data)
{
    Texture* texture = Texture::CreateRGBA(a_width, a_height, a_data);

    const uint32_t textureCount = (uint32_t)m_textures.size();
    for (uint32_t i = 0; i < textureCount; ++i)
    {
        if (m_textures[i] == nullptr)
        {
            m_textures[i] = texture;

            return i;
        }
    }

    m_textures.emplace_back(texture);

    return textureCount;
}
uint32_t RuntimeStorage::GenerateTextureFromHandle(uint32_t a_handle)
{
    Texture* texture = new Texture((GLuint)a_handle);

    const uint32_t textureCount = (uint32_t)m_textures.size();
    for (uint32_t i = 0; i < textureCount; ++i)
    {
        if (m_textures[i] == nullptr)
        {
            m_textures[i] = texture;

            return i;
        }
    }

    m_textures.emplace_back(texture);

    return textureCount;
}
void RuntimeStorage::DestroyTexture(uint32_t a_addr)
{
    delete m_textures[a_addr];
    m_textures[a_addr] = nullptr;
}

uint32_t RuntimeStorage::GenerateTextureSampler(uint32_t a_texture, e_TextureFilter a_filter, e_TextureAddress a_address)
{
    TextureSampler* sampler = TextureSampler::CreateSampler(a_filter, a_address);
    const TextureSamplerBuffer buffer = 
    {
        .Addr = a_texture,
        .Slot = 0,
        .TextureMode = TextureMode_Texture,
        .FilterMode = a_filter,
        .AddressMode = a_address,
        .Data = sampler
    };

    const uint32_t samplerCount = (uint32_t)m_samplers.size();
    for (uint32_t i = 0; i < samplerCount; ++i)
    {
        if (m_samplers[i].Data == nullptr)
        {
            m_samplers[i] = buffer;

            return i;
        }
    }

    m_samplers.emplace_back(buffer);

    return samplerCount;
}
void RuntimeStorage::DestroyTextureSampler(uint32_t a_addr)
{
    delete (TextureSampler*)m_samplers[a_addr].Data;
    m_samplers[a_addr].Data = nullptr;
}

MonoArray* RuntimeStorage::LoadExternalAnimationClip(const std::filesystem::path& a_path)
{
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    AssetLibrary* library = Instance->GetLibrary();
    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        uint32_t size;
        const uint8_t* dat;
        library->GetAsset(a_path, &size, &dat);
        if (size <= 0 || dat == nullptr)
        {
            Logger::Error("Cannot find animation clip file: " + a_path.string());

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, 0, extStr.c_str() + 1);
        ICARIAN_ASSERT(scene != nullptr);

        if (scene->mNumAnimations <= 0)
        {
            break;
        }

        const aiAnimation* animation = scene->mAnimations[0];

        std::vector<AnimationDataExternal> dataArray;

        MonoDomain* domain = RuntimeManager::GetEditorDomain();
        MonoClass* frameClass = RuntimeManager::GetClass("IcarianEngine.Rendering.Animation", "AnimationFrameExternal");
        ICARIAN_ASSERT(frameClass != NULL);

        const uint32_t channelCount = (uint32_t)animation->mNumChannels;
        for (uint32_t i = 0; i < channelCount; ++i)
        {
            const aiNodeAnim* anim = animation->mChannels[i];

            const uint32_t posCount = (uint32_t)anim->mNumPositionKeys;
            if (posCount > 0)
            {
                const AnimationDataExternal dat =
                {
                    .Name = mono_string_new(domain, anim->mNodeName.C_Str()),
                    .Target = mono_string_new(domain, "Translation"),
                    .Frames = mono_array_new(domain, frameClass, (uintptr_t)posCount),
                };

                for (uint32_t j = 0; j < posCount; ++j)
                {
                    const aiVectorKey& posKey = anim->mPositionKeys[j];

                    const AnimationFrameExternal frame =
                    {
                        .Time = (float)posKey.mTime,
                        .Data = glm::vec4(posKey.mValue.x, -posKey.mValue.y, posKey.mValue.z, 1.0f),
                    };

                    mono_array_set(dat.Frames, AnimationFrameExternal, j, frame);
                }

                dataArray.emplace_back(dat);
            }

            const uint32_t rotCount = (uint32_t)anim->mNumRotationKeys;
            if (rotCount > 0)
            {
                const AnimationDataExternal dat =
                {
                    .Name = mono_string_new(domain, anim->mNodeName.C_Str()),
                    .Target = mono_string_new(domain, "Rotation"),
                    .Frames = mono_array_new(domain, frameClass, (uintptr_t)rotCount),
                };

                for (uint32_t j = 0; j < rotCount; ++j)
                {
                    const aiQuatKey& rotKey = anim->mRotationKeys[j];

                    const AnimationFrameExternal frame = 
                    {
                        .Time = (float)rotKey.mTime,
                        .Data = glm::vec4(rotKey.mValue.x, -rotKey.mValue.y, rotKey.mValue.z, rotKey.mValue.w),
                    };

                    mono_array_set(dat.Frames, AnimationFrameExternal, j, frame);
                }

                dataArray.emplace_back(dat);
            }

            const uint32_t scaleCount = (uint32_t)anim->mNumScalingKeys;
            if (scaleCount > 0)
            {
                const AnimationDataExternal dat =
                {
                    .Name = mono_string_new(domain, anim->mNodeName.C_Str()),
                    .Target = mono_string_new(domain, "Scale"),
                    .Frames = mono_array_new(domain, frameClass, (uintptr_t)scaleCount)
                };

                for (uint32_t j = 0; j < scaleCount; ++j)
                {
                    const aiVectorKey& scaleKey = anim->mScalingKeys[j];

                    const AnimationFrameExternal frame = 
                    {
                        .Time = (float)scaleKey.mTime,
                        .Data = glm::vec4(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z, 0.0f)
                    };

                    mono_array_set(dat.Frames, AnimationFrameExternal, j, frame);
                }

                dataArray.emplace_back(dat);
            }
        }

        const uint32_t dataCount = (uint32_t)dataArray.size();
        if (dataCount > 0)
        {
            MonoClass* dataClass = RuntimeManager::GetClass("IcarianEngine.Rendering.Animation", "AnimationDataExternal");
            ICARIAN_ASSERT(dataClass != NULL);

            MonoArray* array = mono_array_new(domain, dataClass, (uintptr_t)dataCount);

            for (uint32_t i = 0; i < dataCount; ++i)
            {
                mono_array_set(array, AnimationDataExternal, i, dataArray[i]);
            }

            return array;
        }

        break;
    }
    default:
    {
        Logger::Error("Animation clip invalid file extension: " + a_path.string());

        break;
    }
    }

    return NULL;
}

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