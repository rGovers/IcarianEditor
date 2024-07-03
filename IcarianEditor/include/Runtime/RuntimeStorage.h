#pragma once

#include <cstdint>
#include <filesystem>
#include <mono/metadata/object.h>
#include <vector>

class AssetLibrary;
class Model;
class PixelShader;
class RuntimeManager;
class Texture;
class VertexShader;

#include "EngineMaterialInteropStructures.h"
#include "EngineTextureSamplerInteropStructures.h"

class RuntimeStorage
{
private:
    AssetLibrary*                     m_assets;
    RuntimeManager*                   m_runtime;
     
    std::vector<Model*>               m_models;
    std::vector<Texture*>             m_textures;
    std::vector<TextureSamplerBuffer> m_samplers;

    std::vector<VertexShader*>        m_vertexShaders;
    std::vector<PixelShader*>         m_pixelShaders;

    std::vector<RenderProgram>        m_renderPrograms;

protected:

public:
    RuntimeStorage(RuntimeManager* a_runtime, AssetLibrary* a_assets);
    ~RuntimeStorage();

    inline AssetLibrary* GetLibrary() const
    {
        return m_assets;
    }

    uint32_t GenerateVertexShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);
    void DestroyVertexShader(uint32_t a_addr);
    inline VertexShader* GetVertexShader(uint32_t a_addr) const
    {
        return m_vertexShaders[a_addr];
    }

    uint32_t GeneratePixelShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);
    void DestroyPixelShader(uint32_t a_addr);
    inline PixelShader* GetPixelShader(uint32_t a_addr) const
    {
        return m_pixelShaders[a_addr];
    }

    uint32_t GenerateRenderProgram(const RenderProgram& a_program);
    void SetProgramTexture(uint32_t a_addr, uint32_t a_slot, uint32_t a_textureAddr);
    void DestroyRenderProgram(uint32_t a_addr);
    inline RenderProgram GetRenderProgram(uint32_t a_addr) const
    {
        return m_renderPrograms[a_addr];
    }
    inline void SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program) 
    {
        m_renderPrograms[a_addr] = a_program;
    }

    uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride);
    void DestroyModel(uint32_t a_addr);
    inline Model* GetModel(uint32_t a_addr) const
    {
        return m_models[a_addr];
    }
    
    uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, const unsigned char* a_data);
    uint32_t GenerateTextureFromHandle(uint32_t a_handle);
    void DestroyTexture(uint32_t a_addr);
    inline Texture* GetTexture(uint32_t a_addr) const
    {
        return m_textures[a_addr];
    }

    uint32_t GenerateTextureSampler(uint32_t a_texture, e_TextureFilter a_filter, e_TextureAddress a_address);
    void DestroyTextureSampler(uint32_t a_addr);
    inline TextureSamplerBuffer GetSamplerBuffer(uint32_t a_addr) const
    {
        return m_samplers[a_addr];
    }

    MonoArray* LoadExternalAnimationClip(const std::filesystem::path& a_path);

    void Clear();
};