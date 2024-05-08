#include "Runtime/RuntimeStorage.h"

#define STBI_NO_STDIO
#include <stb_image.h>

#include <cstring>
#include <ktx.h>

#include "AssetLibrary.h"
#include "Core/ColladaLoader.h"
#include "Core/FBXLoader.h"
#include "Core/FlareShader.h"
#include "Core/GLTFLoader.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/OBJLoader.h"
#include "Logger.h"
#include "Model.h"
#include "PixelShader.h"
#include "Runtime/RuntimeManager.h"
#include "ShaderStorage.h"
#include "Texture.h"
#include "TextureSampler.h"
#include "VertexShader.h"

#include "EngineSkeletonInteropStructures.h"
#include "EngineAnimationDataInteropStructures.h"

static RuntimeStorage* Instance = nullptr;

#define RUNTIMESTORAGE_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define RUNTIMESTORAGE_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Rendering, VertexShader, DestroyShader, { Instance->DestroyVertexShader(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, PixelShader, DestroyShader, { Instance->DestroyPixelShader(a_addr); }, uint32_t a_addr) \
    \
    F(RenderProgram, IcarianEngine.Rendering, Material, GetProgramBuffer, { return Instance->GetRenderProgram(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Material, SetProgramBuffer, { Instance->SetRenderProgram(a_addr, a_program); }, uint32_t a_addr, RenderProgram a_program) \
    F(void, IcarianEngine.Rendering, Material, SetTexture, { Instance->SetProgramTexture(a_addr, a_slot, a_samplerAddr); }, uint32_t a_addr, uint32_t a_slot, uint32_t a_samplerAddr) \
    \
    F(MonoArray*, IcarianEngine.Rendering.Animation, AnimationClip, LoadColladaAnimation, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Instance->LoadDAEAnimationClip(str); }, MonoString* a_path) \
    F(MonoArray*, IcarianEngine.Rendering.Animation, AnimationClip, LoadFBXAnimation, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Instance->LoadFBXAnimationClip(str); }, MonoString* a_path) \
    F(MonoArray*, IcarianEngine.Rendering.Animation, AnimationClip, LoadGLTFAnimation, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Instance->LoadGLTFAnimationClip(str); }, MonoString* a_path) \
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

RUNTIME_FUNCTION(uint32_t, VertexShader, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    AssetLibrary* library = Instance->GetLibrary();
    if (ext == ".fvert")
    {
        uint32_t size; 
        const char* dat;
        library->GetAsset(p, &size, &dat);

        std::string error;
        std::vector<ShaderBufferInput> inputs;
        const std::string s = IcarianCore::GLSLFromFlareShader(std::string_view(dat, size), IcarianCore::ShaderPlatform_OpenGL, &inputs, &error);
        if (s.empty())
        {
            Logger::Error("Failed to parse vertex shader: " + error);

            return -1;
        }

        return Instance->GenerateVertexShader(s, inputs.data(), (uint32_t)inputs.size());
    }

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(uint32_t, PixelShader, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    AssetLibrary* library = Instance->GetLibrary();
    if (ext == ".fpix" || ext == ".ffrag")
    {
        uint32_t size;
        const char* dat;
        library->GetAsset(p, &size, &dat);

        std::string error;
        std::vector<ShaderBufferInput> inputs;
        const std::string s = IcarianCore::GLSLFromFlareShader(std::string_view(dat, size), IcarianCore::ShaderPlatform_OpenGL, &inputs, &error);
        if (s.empty())
        {
            Logger::Error("Failed to parse pixel shader: " + error);

            return -1;
        }

        return Instance->GeneratePixelShader(s, inputs.data(), (uint32_t)inputs.size());
    }

    return -1;
}, MonoString* a_path)

RUNTIME_FUNCTION(uint32_t, Material, GenerateProgram, 
{
    RenderProgram program;
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.ShadowVertexShader = a_shadowVertexShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (e_CullMode)a_cullMode;
    program.PrimitiveMode = (e_PrimitiveMode)a_primitiveMode;
    program.ColorBlendMode = (e_MaterialBlendMode)a_colorBlendMode;
    program.RenderLayer = a_renderLayer;
    program.Data = nullptr;
    program.Flags = 0;

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
RUNTIME_FUNCTION(uint32_t, Model, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    float radius;

    AssetLibrary* library = Instance->GetLibrary();
    if (ext == ".obj")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::OBJLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }
    else if (ext == ".dae")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::ColladaLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }
    else if (ext == ".fbx")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::FBXLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }
    else if (ext == ".glb" || ext == ".gltf")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::GLTFLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }

    Logger::Warning(std::string("Cannot find file") + str);

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(uint32_t, Model, GenerateSkinnedFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    std::vector<SkinnedVertex> vertices;
    std::vector<uint32_t> indices;
    float radius;

    AssetLibrary* library = Instance->GetLibrary();
    if (ext == ".dae")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::ColladaLoader_LoadSkinnedData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(SkinnedVertex));
        }
    }
    else if (ext == ".fbx")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::FBXLoader_LoadSkinnedData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(SkinnedVertex));
        }
    }
    else if (ext == ".glb" || ext == ".gltf")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::GLTFLoader_LoadSkinnedData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(SkinnedVertex));
        }
    }

    return -1;
}, MonoString* a_path)

RUNTIME_FUNCTION(RuntimeImportBoneData, Skeleton, LoadBoneData, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    RuntimeImportBoneData data;

    data.BindPoses = NULL;
    data.Names = NULL;
    data.Parents = NULL;

    std::vector<IcarianCore::BoneData> bones;

    AssetLibrary* library = Instance->GetLibrary();
    if (ext == ".dae")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::ColladaLoader_LoadBoneData(dat, size, &bones))
        {
            MonoDomain* domain = mono_domain_get();
            MonoClass* fClass = mono_get_single_class();

            const uint32_t count = (uint32_t)bones.size();
            data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)count);
            data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)count);
            data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)count);

            for (uint32_t i = 0; i < count; ++i)
            {
                const IcarianCore::BoneData& bone = bones[i];

                MonoArray* bindPose = mono_array_new(domain, fClass, 16);
                for (uint32_t j = 0; j < 16; ++j)
                {
                    mono_array_set(bindPose, float, j, bone.Transform[j / 4][j % 4]);
                }

                mono_array_set(data.BindPoses, MonoArray*, i, bindPose);
                mono_array_set(data.Names, MonoString*, i, mono_string_new(domain, bone.Name.c_str()));
                mono_array_set(data.Parents, uint32_t, i, bone.Parent);
            }
        }   
    }
    else if (ext == ".fbx")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::FBXLoader_LoadBoneData(dat, size, &bones))
        {
            MonoDomain* domain = mono_domain_get();
            MonoClass* fClass = mono_get_single_class();

            const uint32_t count = (uint32_t)bones.size();
            data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)count);
            data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)count);
            data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)count);

            for (uint32_t i = 0; i < count; ++i)
            {
                const IcarianCore::BoneData& bone = bones[i];

                MonoArray* bindPose = mono_array_new(domain, fClass, 16);
                for (uint32_t j = 0; j < 16; ++j)
                {
                    mono_array_set(bindPose, float, j, bone.Transform[j / 4][j % 4]);
                }

                mono_array_set(data.BindPoses, MonoArray*, i, bindPose);
                mono_array_set(data.Names, MonoString*, i, mono_string_new(domain, bone.Name.c_str()));
                mono_array_set(data.Parents, uint32_t, i, bone.Parent);
            }
        }
    }
    else if (ext == ".glb" || ext == ".gltf")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && IcarianCore::GLTFLoader_LoadBones(dat, size, &bones))
        {
            MonoDomain* domain = mono_domain_get();
            MonoClass* fClass = mono_get_single_class();

            const uint32_t count = (uint32_t)bones.size();
            data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)count);
            data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)count);
            data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)count);

            for (uint32_t i = 0; i < count; ++i)
            {
                const IcarianCore::BoneData& bone = bones[i];

                MonoArray* bindPose = mono_array_new(domain, fClass, 16);
                for (uint32_t j = 0; j < 16; ++j)
                {
                    mono_array_set(bindPose, float, j, bone.Transform[j / 4][j % 4]);
                }

                mono_array_set(data.BindPoses, MonoArray*, i, bindPose);
                mono_array_set(data.Names, MonoString*, i, mono_string_new(domain, bone.Name.c_str()));
                mono_array_set(data.Parents, uint32_t, i, bone.Parent);
            }
        }
    }

    return data;
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
        const char* dat;
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
        const char* dat;
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

                GLuint texHandle = 0;
                GLenum target;
                ktxTexture_GLUpload((ktxTexture*)ktxTex, &texHandle, &target, NULL);
                
                return Instance->GenerateTextureFromHandle((uint32_t)texHandle);
            }
        }
    }

    return -1;
}, MonoString* a_path)

RuntimeStorage::RuntimeStorage(RuntimeManager* a_runtime, AssetLibrary* a_assets)
{
    m_assets = a_assets;
    m_runtime = a_runtime;

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, VertexShader, GenerateFromFile);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, PixelShader, GenerateFromFile);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Material, GenerateProgram);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Material, DestroyProgram);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Model, GenerateModel);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Model, GenerateFromFile);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Model, GenerateSkinnedFromFile);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.Animation, Skeleton, LoadBoneData);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Texture, GenerateFromFile);

    RUNTIMESTORAGE_BINDING_FUNCTION_TABLE(RUNTIMESTORAGE_RUNTIME_ATTACH);

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

    for (const VertexShader* v : m_vertexShaders)
    {
        if (v != nullptr)
        {
            delete v;
        }
    }
    m_vertexShaders.clear();

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

uint32_t RuntimeStorage::GenerateVertexShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs, uint32_t a_inputCount)
{
    VertexShader* vShader = VertexShader::GenerateShader(a_str, a_inputs, a_inputCount);
    if (vShader == nullptr)
    {
        return -1;
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
void RuntimeStorage::DestroyVertexShader(uint32_t a_addr)
{
    delete m_vertexShaders[a_addr];
    m_vertexShaders[a_addr] = nullptr;
}

uint32_t RuntimeStorage::GeneratePixelShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs, uint32_t a_inputCount)
{
    PixelShader* pShader = PixelShader::GenerateShader(a_str, a_inputs, a_inputCount);
    if (pShader == nullptr)
    {
        return -1;
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

    const VertexShader* vShader = GetVertexShader(program.VertexShader);
    if (vShader != nullptr)
    {
        SetRenderBuffers(vShader, program);
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

MonoArray* RuntimeStorage::LoadDAEAnimationClip(const std::filesystem::path& a_path) const
{
    MonoArray* data = NULL;

    MonoDomain* domain = m_runtime->GetEditorDomain();
    MonoClass* animationDataClass = m_runtime->GetClass("IcarianEngine.Rendering.Animation", "DAERAnimation");
    ICARIAN_ASSERT(animationDataClass != NULL);
    MonoClass* animationFrameClass = m_runtime->GetClass("IcarianEngine.Rendering.Animation", "DAERAnimationFrame");
    ICARIAN_ASSERT(animationFrameClass != NULL);
    MonoClass* floatClass = mono_get_single_class();

    uint32_t size;
    const char* dat;
    m_assets->GetAsset(a_path, &size, &dat);

    std::vector<IcarianCore::ColladaAnimationData> animations;
    if (size > 0 && dat != nullptr && IcarianCore::ColladaLoader_LoadAnimationData(dat, size, &animations))
    {
        const uint32_t count = (uint32_t)animations.size();
        data = mono_array_new(domain, animationDataClass, (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const IcarianCore::ColladaAnimationData& animation = animations[i];

            DAERAnimation animData;
            animData.Name = mono_string_new(domain, animation.Name.c_str());

            const uint32_t frameCount = (uint32_t)animation.Frames.size();
            animData.Frames = mono_array_new(domain, animationFrameClass, (uintptr_t)frameCount);
            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const IcarianCore::ColladaAnimationFrame& frame = animation.Frames[j];

                DAERAnimationFrame animFrame;
                animFrame.Time = frame.Time;

                const float* t = (float*)&frame.Transform;

                MonoArray* transform = mono_array_new(domain, floatClass, 16);
                for (uint32_t k = 0; k < 16; ++k)
                {
                    mono_array_set(transform, float, k, t[k]);
                }

                animFrame.Transform = transform;

                mono_array_set(animData.Frames, DAERAnimationFrame, j, animFrame);
            }

            mono_array_set(data, DAERAnimation, i, animData);
        }
    }

    return data;
}

MonoArray* RuntimeStorage::LoadFBXAnimationClip(const std::filesystem::path& a_path) const
{
    MonoArray* data = NULL;

    MonoDomain* domain = m_runtime->GetEditorDomain();
    MonoClass* animationDataClass = m_runtime->GetClass("IcarianEngine.Rendering.Animation", "FBXRAnimation");
    ICARIAN_ASSERT(animationDataClass != NULL);
    MonoClass* animationFrameClass = m_runtime->GetClass("IcarianEngine.Rendering.Animation", "FBXRAnimationFrame");
    ICARIAN_ASSERT(animationFrameClass != NULL);

    uint32_t size;
    const char* dat;
    m_assets->GetAsset(a_path, &size, &dat);

    std::vector<IcarianCore::FBXAnimationData> animations;
    if (size > 0 && dat != nullptr && IcarianCore::FBXLoader_LoadAnimationData(dat, size, &animations))
    {
        const uint32_t count = (uint32_t)animations.size();
        data = mono_array_new(domain, animationDataClass, (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const IcarianCore::FBXAnimationData& animation = animations[i];

            FBXRAnimation animData;
            animData.Name = mono_string_new(domain, animation.Name.c_str());
            animData.Target = mono_string_new(domain, animation.PropertyName.c_str());

            const uint32_t frameCount = (uint32_t)animation.Frames.size();
            animData.Frames = mono_array_new(domain, animationFrameClass, (uintptr_t)frameCount);
            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const IcarianCore::FBXAnimationFrame& frame = animation.Frames[j];

                FBXRAnimationFrame animFrame;
                animFrame.Time = frame.Time;
                animFrame.Data = frame.Data;

                mono_array_set(animData.Frames, FBXRAnimationFrame, j, animFrame);
            }   

            mono_array_set(data, FBXRAnimation, i, animData);
        }
    }

    return data;
}

MonoArray* RuntimeStorage::LoadGLTFAnimationClip(const std::filesystem::path& a_path) const
{
    MonoArray* data = NULL;

    MonoDomain* domain = m_runtime->GetEditorDomain();
    MonoClass* animationDataClass = m_runtime->GetClass("IcarianEngine.Rendering.Animation", "GLTFRAnimation");
    ICARIAN_ASSERT(animationDataClass != NULL);
    MonoClass* animationFrameClass = m_runtime->GetClass("IcarianEngine.Rendering.Animation", "GLTFRAnimationFrame");
    ICARIAN_ASSERT(animationFrameClass != NULL);

    uint32_t size;
    const char* dat;
    m_assets->GetAsset(a_path, &size, &dat);

    std::vector<IcarianCore::GLTFAnimationData> animations;
    if (size > 0 && dat != nullptr && IcarianCore::GLTFLoader_LoadAnimationData(dat, size, &animations))
    {
        const uint32_t count = (uint32_t)animations.size();
        data = mono_array_new(domain, animationDataClass, (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const IcarianCore::GLTFAnimationData& animation = animations[i];

            GLTFRAnimation animData;
            animData.Name = mono_string_new(domain, animation.Name.c_str());
            animData.Target = mono_string_new(domain, animation.Target.c_str());

            const uint32_t frameCount = (uint32_t)animation.Frames.size();
            animData.Frames = mono_array_new(domain, animationFrameClass, (uintptr_t)frameCount);
            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const IcarianCore::GLTFAnimationFrame& frame = animation.Frames[j];

                GLTFRAnimationFrame animFrame;
                animFrame.Time = frame.Time;
                animFrame.Data = frame.Data;

                mono_array_set(animData.Frames, GLTFRAnimationFrame, j, animFrame);
            }   

            mono_array_set(data, GLTFRAnimation, i, animData);
        }
    }

    return data;
}