// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <filesystem>
#include <mono/metadata/object.h>
#include <unordered_map>
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
    AssetLibrary*                                m_assets;
    RuntimeManager*                              m_runtime;
     
    std::vector<Model*>                          m_models;
    std::vector<Texture*>                        m_textures;
    std::vector<TextureSamplerBuffer>            m_samplers;

    std::unordered_map<std::string, std::string> m_vertexImports;
    std::unordered_map<std::string, std::string> m_pixelImports;

    std::vector<VertexShader*>                   m_vertexShaders;
    std::vector<PixelShader*>                    m_pixelShaders;

    std::vector<RenderProgram>                   m_renderPrograms;

protected:

public:
    RuntimeStorage(RuntimeManager* a_runtime, AssetLibrary* a_assets);
    ~RuntimeStorage();

    inline AssetLibrary* GetLibrary() const
    {
        return m_assets;
    }

    uint32_t GenerateVertexShader(const std::filesystem::path& a_path);
    void AddVertexImport(const std::string_view& a_key, const std::string_view& a_value);
    void DestroyVertexShader(uint32_t a_addr);
    inline VertexShader* GetVertexShader(uint32_t a_addr) const
    {
        return m_vertexShaders[a_addr];
    }

    uint32_t GeneratePixelShader(const std::filesystem::path& a_path);
    void AddPixelImport(const std::string_view& a_key, const std::string_view& a_value);
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

// MIT License
// 
// Copyright (c) 2024 River Govers
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