#include "Runtime/RuntimeStorage.h"

#include <cstring>
#include <ktx.h>
#include <stb_image.h>

#include "AssetLibrary.h"
#include "Flare/ColladaLoader.h"
#include "Flare/FBXLoader.h"
#include "Flare/GLTFLoader.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Flare/OBJLoader.h"
#include "Model.h"
#include "PixelShader.h"
#include "Runtime/RuntimeManager.h"
#include "ShaderGenerator.h"
#include "ShaderStorage.h"
#include "Texture.h"
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
    /* Have not had to but I dont not believe there is a way to seperate the texture and sampler in OpenGL */ \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateTextureSampler, { return a_texture; }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return 0; }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return 0; }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, IcarianEngine.Rendering, TextureSampler, DestroySampler, { }, uint32_t a_addr) \
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

        const std::string s = GLSL_FromFShader(std::string_view(dat, size));

        return Instance->GenerateVertexShader(s);
    }
    else if (ext == ".vert")
    {
        uint32_t size; 
        const char* dat;
        library->GetAsset(p, &size, &dat);

        return Instance->GenerateVertexShader(dat);
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

        const std::string s = GLSL_FromFShader(std::string_view(dat, size));

        return Instance->GeneratePixelShader(s);
    }
    else if (ext == ".pix" || ext == ".frag")
    {
        uint32_t size; 
        const char* dat;
        library->GetAsset(p, &size, &dat);

        return Instance->GeneratePixelShader(dat);
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
    program.EnableColorBlending = (uint8_t)a_enableColorBlending;
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

    if (a_shaderInputs != NULL)
    {
        program.ShaderBufferInputCount = (uint16_t)mono_array_length(a_shaderInputs);
        program.ShaderBufferInputs = new ShaderBufferInput[program.ShaderBufferInputCount];

        for (uint16_t i = 0; i < program.ShaderBufferInputCount; ++i)
        {
            program.ShaderBufferInputs[i] = mono_array_get(a_shaderInputs, ShaderBufferInput, i);
        }
    }
    else
    {
        program.ShaderBufferInputCount = 0;
        program.ShaderBufferInputs = nullptr;
    }

    if (a_shadowShaderInputs != NULL)
    {
        program.ShadowShaderBufferInputCount = (uint16_t)mono_array_length(a_shadowShaderInputs);
        program.ShadowShaderBufferInputs = new ShaderBufferInput[program.ShadowShaderBufferInputCount];

        for (uint16_t i = 0; i < program.ShadowShaderBufferInputCount; ++i)
        {
            program.ShadowShaderBufferInputs[i] = mono_array_get(a_shadowShaderInputs, ShaderBufferInput, i);
        }
    }
    else
    {
        program.ShadowShaderBufferInputCount = 0;
        program.ShadowShaderBufferInputs = nullptr;
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
}, uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_attributes, MonoArray* a_shaderInputs, uint32_t a_cullMode, uint32_t a_primitiveMode, uint32_t a_enableColorBlending, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, MonoArray* a_shadowShaderInputs, uint32_t a_uboSize, void* a_uboBuffer)
RUNTIME_FUNCTION(void, Material, DestroyProgram, 
{
    RenderProgram program = Instance->GetRenderProgram(a_addr);

    IDEFER(
    {
        if (program.VertexAttributes != nullptr)
        {
            delete[] program.VertexAttributes;
        }

        if (program.ShaderBufferInputs != nullptr)
        {
            delete[] program.ShaderBufferInputs;
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

        if (dat != nullptr && size > 0 && FlareBase::OBJLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }
    else if (ext == ".dae")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && FlareBase::ColladaLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }
    else if (ext == ".fbx")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && FlareBase::FBXLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }
    else if (ext == ".glb" || ext == ".gltf")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && FlareBase::GLTFLoader_LoadData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(Vertex));
        }
    }

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

        if (dat != nullptr && size > 0 && FlareBase::ColladaLoader_LoadSkinnedData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(SkinnedVertex));
        }
    }
    else if (ext == ".fbx")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && FlareBase::FBXLoader_LoadSkinnedData(dat, size, &vertices, &indices, &radius))
        {
            return Instance->GenerateModel(vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(SkinnedVertex));
        }
    }
    else if (ext == ".glb" || ext == ".gltf")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && FlareBase::GLTFLoader_LoadSkinnedData(dat, size, &vertices, &indices, &radius))
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

    std::vector<BoneData> bones;

    AssetLibrary* library = Instance->GetLibrary();
    if (ext == ".dae")
    {
        const char* dat;
        uint32_t size;
        library->GetAsset(p, &size, &dat);

        if (dat != nullptr && size > 0 && FlareBase::ColladaLoader_LoadBoneData(dat, size, &bones))
        {
            MonoDomain* domain = mono_domain_get();
            MonoClass* fClass = mono_get_single_class();

            const uint32_t count = (uint32_t)bones.size();
            data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)count);
            data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)count);
            data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)count);

            for (uint32_t i = 0; i < count; ++i)
            {
                const BoneData& bone = bones[i];

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

        if (dat != nullptr && size > 0 && FlareBase::FBXLoader_LoadBoneData(dat, size, &bones))
        {
            MonoDomain* domain = mono_domain_get();
            MonoClass* fClass = mono_get_single_class();

            const uint32_t count = (uint32_t)bones.size();
            data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)count);
            data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)count);
            data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)count);

            for (uint32_t i = 0; i < count; ++i)
            {
                const BoneData& bone = bones[i];

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

        if (dat != nullptr && size > 0 && FlareBase::GLTFLoader_LoadBones(dat, size, &bones))
        {
            MonoDomain* domain = mono_domain_get();
            MonoClass* fClass = mono_get_single_class();

            const uint32_t count = (uint32_t)bones.size();
            data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)count);
            data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)count);
            data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)count);

            for (uint32_t i = 0; i < count; ++i)
            {
                const BoneData& bone = bones[i];

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

uint32_t RuntimeStorage::GenerateVertexShader(const std::string_view& a_str)
{
    VertexShader* vShader = VertexShader::GenerateShader(a_str);
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

uint32_t RuntimeStorage::GeneratePixelShader(const std::string_view& a_str)
{
    PixelShader* pShader = PixelShader::GenerateShader(a_str);
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

uint32_t RuntimeStorage::GenerateRenderProgram(const RenderProgram& a_program)
{
    RenderProgram program = a_program;

    ShaderStorage* storage = new ShaderStorage(this);
    program.Data = storage;

    for (uint32_t i = 0; i < a_program.ShaderBufferInputCount; ++i)
    {
        const ShaderBufferInput& input = a_program.ShaderBufferInputs[i];

        switch (input.BufferType)
        {
        case ShaderBufferType_UserUBO:
        {
            storage->SetUserUBO(input.Slot, a_program.UBOData, a_program.UBODataSize);

            break;
        }
        }
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
    storage->SetTexture(a_slot, m_textures[a_textureAddr]);
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

    std::vector<ColladaAnimationData> animations;
    if (size > 0 && dat != nullptr && FlareBase::ColladaLoader_LoadAnimationData(dat, size, &animations))
    {
        const uint32_t count = (uint32_t)animations.size();
        data = mono_array_new(domain, animationDataClass, (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const ColladaAnimationData& animation = animations[i];

            DAERAnimation animData;
            animData.Name = mono_string_new(domain, animation.Name.c_str());

            const uint32_t frameCount = (uint32_t)animation.Frames.size();
            animData.Frames = mono_array_new(domain, animationFrameClass, (uintptr_t)frameCount);
            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const ColladaAnimationFrame& frame = animation.Frames[j];

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

    std::vector<FBXAnimationData> animations;
    if (size > 0 && dat != nullptr && FlareBase::FBXLoader_LoadAnimationData(dat, size, &animations))
    {
        const uint32_t count = (uint32_t)animations.size();
        data = mono_array_new(domain, animationDataClass, (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const FBXAnimationData& animation = animations[i];

            FBXRAnimation animData;
            animData.Name = mono_string_new(domain, animation.Name.c_str());
            animData.Target = mono_string_new(domain, animation.PropertyName.c_str());

            const uint32_t frameCount = (uint32_t)animation.Frames.size();
            animData.Frames = mono_array_new(domain, animationFrameClass, (uintptr_t)frameCount);
            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const FBXAnimationFrame& frame = animation.Frames[j];

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

    std::vector<GLTFAnimationData> animations;
    if (size > 0 && dat != nullptr && FlareBase::GLTFLoader_LoadAnimationData(dat, size, &animations))
    {
        const uint32_t count = (uint32_t)animations.size();
        data = mono_array_new(domain, animationDataClass, (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const GLTFAnimationData& animation = animations[i];

            GLTFRAnimation animData;
            animData.Name = mono_string_new(domain, animation.Name.c_str());
            animData.Target = mono_string_new(domain, animation.Target.c_str());

            const uint32_t frameCount = (uint32_t)animation.Frames.size();
            animData.Frames = mono_array_new(domain, animationFrameClass, (uintptr_t)frameCount);
            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const GLTFAnimationFrame& frame = animation.Frames[j];

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