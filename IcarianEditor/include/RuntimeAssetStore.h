// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <string_view>

class EngineProcess;

#include "EngineMaterialInteropStructures.h"

struct AssetDataBuffer
{
    void* Data;
    uint32_t DataSize;
    uint32_t ID;
};

class RuntimeAssetStore
{
private:
    static constexpr const char GenerateRenderProgramString[] = "Editor:AssetStore:GenerateRenderProgram";

    static constexpr const char GenerateMeshShaderString[] = "Editor:AssetStore:GenerateMeshShaderFromFile";
    static constexpr const char GenerateVertexShaderString[] = "Editor:AssetStore:GenerateVertexShaderFromFile";
    static constexpr const char GeneratePixelShaderString[] = "Editor:AssetStore:GeneratePixelShaderFromFile";

    static constexpr const char GenerateMeshString[] = "Editor:AssetStore:GenerateMeshFromFile";

    EngineProcess**  m_activeProcesses;

    AssetDataBuffer* m_materialDataBuffers;

    AssetDataBuffer* m_meshShaderDataBuffers;
    AssetDataBuffer* m_vertexShaderDataBuffers;
    AssetDataBuffer* m_pixelShaderDataBuffers;

    AssetDataBuffer* m_meshDataBuffers;
    AssetDataBuffer* m_modelDataBuffers;

    uint32_t         m_activeProcessCount;

    uint32_t         m_materialDataBufferCount;

    uint32_t         m_meshShaderDataBufferCount;
    uint32_t         m_vertexShaderDataBufferCount;
    uint32_t         m_pixelShaderDataBufferCount;

    uint32_t         m_meshDataBufferCount;
    uint32_t         m_modelDataBufferCount;

    uint32_t         m_materialID;

    uint32_t         m_meshShaderID;
    uint32_t         m_vertexShaderID;
    uint32_t         m_pixelShaderID;

    uint32_t         m_meshID;
    uint32_t         m_modelID;

    RuntimeAssetStore();

protected:

public:
    ~RuntimeAssetStore();

    static void Init();
    static void Destroy();

    static void Clear();

    static void RegisterEngineProcess(EngineProcess* a_process);

    static uint32_t GenerateRenderProgram
    (
        uint32_t a_vertexShader,
        uint32_t a_pixelShader,
        uint16_t a_vertexStride,
        uint32_t a_attributeCount,
        const VertexInputAttribute* a_attributes,
        e_CullMode a_cullMode,
        e_MaterialBlendMode a_colorBlendMode,
        uint32_t a_renderLayer,
        uint32_t a_uboSize,
        const void* a_ubo,
        uint32_t a_userArrayStride,
        uint32_t a_userArrayCount,
        const void* a_userArray
    );
    static uint32_t GenerateMeshRenderProgram
    (
        uint32_t a_meshShader,
        uint32_t a_pixelShader,
        uint16_t a_vertexStride,
        e_CullMode a_cullMode,
        e_MaterialBlendMode a_colorBlendMode,
        uint32_t a_renderLayer,
        uint32_t a_uboSize,
        const void* a_ubo,
        uint32_t a_userArrayStride,
        uint32_t a_userArrayCount,
        const void* a_userArray
    );

    static uint32_t GenerateMeshShaderFromFile(const std::string_view& a_path);
    static uint32_t GenerateVertexShaderFromFile(const std::string_view& a_path);
    static uint32_t GeneratePixelShaderFromFile(const std::string_view& a_path);

    static uint32_t GenerateMeshFromFile(const std::string_view& a_path);

    static uint32_t GenerateModelFromFile(const std::string_view& a_path);
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