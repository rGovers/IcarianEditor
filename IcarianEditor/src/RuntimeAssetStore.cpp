// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "RuntimeAssetStore.h"

#include <cstring>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "EngineProcess.h"
#include "Logger.h"
#include "Runtime/RuntimeManager.h"

static RuntimeAssetStore* Instance = nullptr;

#include "EditorRenderProgramHeaderInteropStructures.h"
#include "EngineModelInteropStructures.h"

#define RENDERCOMMAND_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.Shaders, MeshShader, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return RuntimeAssetStore::GenerateMeshShaderFromFile(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, MeshShader, DestroyShader, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Shaders, VertexShader, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return RuntimeAssetStore::GenerateVertexShaderFromFile(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, VertexShader, DestroyShader, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Shaders, PixelShader, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return RuntimeAssetStore::GeneratePixelShaderFromFile(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, PixelShader, DestroyShader, { }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Material, DestroyProgram, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Mesh, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return RuntimeAssetStore::GenerateMeshFromFile(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering, Mesh, DestroyMesh, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return RuntimeAssetStore::GenerateModelFromFile(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { }, uint32_t a_addr) \

RUNTIME_FUNCTION(uint32_t, Material, GenerateProgram, 
{
    const uint32_t userArrayCount = ILAMBDA(
    {
        if (a_userArray == NULL)
        {
            ILRETURN uint32_t(0);
        }

        ILRETURN (uint32_t)mono_array_length(a_userArray);
    });

    const uint8_t* userArrayData = ILAMBDA(
    {
        if (userArrayCount <= 0)
        {
            ILRETURN (uint8_t*)nullptr;
        }
        if (a_arrayStride <= 0)
        {
            ILRETURN (uint8_t*)nullptr;
        }

        const uint32_t size = userArrayCount * a_arrayStride;
        uint8_t* val = new uint8_t[size];
        for (uint32_t i = 0; i < userArrayCount; ++i)
        {
            const uint8_t* dat = (uint8_t*)mono_array_addr_with_size(a_userArray, a_arrayStride, i);

            memcpy(val + i * a_arrayStride, dat, a_arrayStride);
        }

        ILRETURN val;
    });
    IDEFER(
    {
        if (userArrayData != nullptr)
        {
            delete[] userArrayData;
        }
    });

    const uint32_t attributeCount = ILAMBDA(
    {
        if (a_attributes == NULL)
        {
            ILRETURN uint32_t(0);
        }

        ILRETURN (uint32_t)mono_array_length(a_attributes);
    });

    const VertexInputAttribute* attributes = ILAMBDA(
    {
        if (attributeCount <= 0)
        {
            ILRETURN (VertexInputAttribute*)nullptr;
        }

        VertexInputAttribute* val = new VertexInputAttribute[attributeCount];
        for (uint32_t i = 0; i < attributeCount; ++i)
        {
            val[i] = mono_array_get(a_attributes, VertexInputAttribute, i);
        }

        return val;
    });
    IDEFER(
    {
        if (attributes != nullptr)
        {
            delete[] attributes;
        }
    });

    return RuntimeAssetStore::GenerateRenderProgram
    (
        a_vertexShader,
        a_pixelShader,
        a_vertexStride,
        attributeCount,
        attributes,
        (e_CullMode)a_cullMode,
        (e_MaterialBlendMode)a_colorBlendMode,
        a_renderLayer,
        a_uboSize,
        a_uboBuffer,
        a_arrayStride,
        userArrayCount,
        userArrayData
    );
}, uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_attributes, uint32_t a_cullMode, uint32_t a_primitiveMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, uint32_t a_uboSize, void* a_uboBuffer, MonoArray* a_userArray, uint32_t a_arrayStride)
RUNTIME_FUNCTION(uint32_t, Material, GenerateMeshProgram, 
{
    const uint32_t userArrayCount = ILAMBDA(
    {
        if (a_userArray == NULL)
        {
            ILRETURN uint32_t(0);
        }

        ILRETURN (uint32_t)mono_array_length(a_userArray);
    });

    const uint8_t* userArrayData = ILAMBDA(
    {
        if (userArrayCount <= 0)
        {
            ILRETURN (uint8_t*)nullptr;
        }
        if (a_arrayStride <= 0)
        {
            ILRETURN (uint8_t*)nullptr;
        }

        const uint32_t size = userArrayCount * a_arrayStride;
        uint8_t* val = new uint8_t[size];
        for (uint32_t i = 0; i < userArrayCount; ++i)
        {
            const uint8_t* dat = (uint8_t*)mono_array_addr_with_size(a_userArray, a_arrayStride, i);

            memcpy(val + i * a_arrayStride, dat, a_arrayStride);
        }

        ILRETURN val;
    });
    IDEFER(
    {
        if (userArrayData != nullptr)
        {
            delete[] userArrayData;
        }
    });

    return RuntimeAssetStore::GenerateMeshRenderProgram
    (
        a_meshShader,
        a_pixelShader,
        a_vertexStride,
        (e_CullMode)a_cullMode,
        (e_MaterialBlendMode)a_colorBlendMode,
        a_renderLayer,
        a_uboSize,
        a_uboBuffer,
        a_arrayStride,
        userArrayCount,
        userArrayData
    );
}, uint32_t a_meshShader, uint32_t a_pixelShader, uint16_t a_vertexStride, uint32_t a_cullMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_uboSize, void* a_uboBuffer, MonoArray* a_userArray, uint32_t a_arrayStride)

RENDERCOMMAND_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RuntimeAssetStore::RuntimeAssetStore()
{
    m_materialID = 0;

    m_meshShaderID = 0;
    m_vertexShaderID = 0;
    m_pixelShaderID = 0;

    m_meshID = 0;
    m_modelID = 0;

    RENDERCOMMAND_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateProgram);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateMeshProgram);
}
RuntimeAssetStore::~RuntimeAssetStore()
{

}

void RuntimeAssetStore::Init()
{
    if (Instance == nullptr)
    {
        Instance = new RuntimeAssetStore();
    }
}
void RuntimeAssetStore::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void RuntimeAssetStore::SetActiveEngineProcess(EngineProcess* a_process)
{
    ICARIAN_ASSERT(Instance != nullptr);

    Instance->m_activeProcess = a_process;
}

uint32_t RuntimeAssetStore::GenerateRenderProgram
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
)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_activeProcess == nullptr)
    {
        Logger::Warning("GenerateRenderProgram no active engine process");

        return uint32_t(-1);
    }

    const uint32_t id = Instance->m_materialID++;

    const uint32_t userArraySize = a_userArrayCount * a_userArrayStride;
    const uint32_t attributeSize = a_attributeCount * sizeof(VertexInputAttribute);

    constexpr uint32_t VertexAttributeOffset = sizeof(RenderProgramHeader);
    const uint32_t uboOffset = VertexAttributeOffset + attributeSize;
    const uint32_t userArrayOffset = uboOffset + a_uboSize;
    const uint32_t bufferSize = userArrayOffset + userArraySize;

    const RenderProgramHeader header = 
    {
        .ID = id,
        .IsMesh = 0,
        .VertexShader = a_vertexShader,
        .PixelShader = a_pixelShader,
        .VertexStride = (uint32_t)a_vertexStride,
        .VertexAttributeCount = a_attributeCount,
        .VertexAttributeOffset = VertexAttributeOffset,
        .CullMode = (uint32_t)a_cullMode,
        .ColorBlendMode = (uint32_t)a_colorBlendMode,
        .RenderLayer = a_renderLayer,
        .UBOSize = a_uboSize,
        .UBOOffset = uboOffset,
        .UserArrayCount = a_userArrayCount,
        .UserArrayStride = a_userArrayStride,
        .UserArrayOffset = userArrayOffset
    };

    uint8_t* buffer = (uint8_t*)malloc(bufferSize);
    IDEFER(free(buffer));

    memset(buffer, 0, bufferSize);

    memcpy(buffer, &header, sizeof(RenderProgramHeader));

    if (a_attributes != nullptr)
    {
        memcpy(buffer + VertexAttributeOffset, a_attributes, attributeSize);
    }
    if (a_ubo != nullptr)
    {
        memcpy(buffer + uboOffset, a_ubo, a_uboSize);
    }
    if (a_userArray != nullptr)
    {
        memcpy(buffer + userArrayOffset, a_userArray, userArraySize);
    }

    Instance->m_activeProcess->SendRuntimeMessage("Editor:AssetStore:GenerateRenderProgram", buffer, bufferSize);

    return id;
}
uint32_t RuntimeAssetStore::GenerateMeshRenderProgram
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
)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_activeProcess == nullptr)
    {
        Logger::Warning("GenerateMeshRenderProgram no active engine process");

        return uint32_t(-1);
    }

    const uint32_t id = Instance->m_materialID++;

    const uint32_t userArraySize = a_userArrayCount * a_userArrayStride;

    constexpr uint32_t UBOOffset = sizeof(RenderProgramHeader);
    const uint32_t userArrayOffset = UBOOffset + a_uboSize;
    const uint32_t bufferSize = userArrayOffset + userArraySize;

    const RenderProgramHeader header = 
    {
        .ID = id,
        .IsMesh = 1,
        .VertexShader = a_meshShader,
        .PixelShader = a_pixelShader,
        .VertexStride = (uint32_t)a_vertexStride,
        .CullMode = (uint32_t)a_cullMode,
        .ColorBlendMode = (uint32_t)a_colorBlendMode,
        .RenderLayer = a_renderLayer,
        .UBOSize = a_uboSize,
        .UBOOffset = UBOOffset,
        .UserArrayCount = a_userArrayCount,
        .UserArrayStride = a_userArrayStride,
        .UserArrayOffset = userArrayOffset,
    };

    uint8_t* buffer = (uint8_t*)malloc(bufferSize);
    IDEFER(free(buffer));

    memset(buffer, 0, bufferSize);

    memcpy(buffer, &header, sizeof(RenderProgramHeader));

    if (a_ubo != nullptr)
    {
        memcpy(buffer + UBOOffset, a_ubo, a_uboSize);
    }
    if (a_userArray != nullptr)
    {
        memcpy(buffer + userArrayOffset, a_userArray, userArraySize);
    }

    Instance->m_activeProcess->SendRuntimeMessage("Editor:AssetStore:GenerateRenderProgram", buffer, bufferSize);

    return id;
}

uint32_t RuntimeAssetStore::GenerateMeshShaderFromFile(const std::string_view& a_path)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_activeProcess == nullptr)
    {
        Logger::Warning("GenerateMeshShaderFromFile no active engine process");

        return uint32_t(-1);
    }

    const uint32_t id = Instance->m_meshShaderID++;
    const uint32_t stringLen = (uint32_t)a_path.length();

    const uint32_t bufferSize = sizeof(uint32_t) + stringLen + 1;

    char* buffer = (char*)malloc(bufferSize);
    IDEFER(free(buffer));

    memset(buffer, 0, bufferSize);

    memcpy(buffer, &id, sizeof(uint32_t));
    memcpy(buffer + sizeof(uint32_t), a_path.data(), stringLen);

    Instance->m_activeProcess->SendRuntimeMessage("Editor:AssetStore:GenerateMeshShaderFromFile", buffer, bufferSize);

    return id;
}
uint32_t RuntimeAssetStore::GenerateVertexShaderFromFile(const std::string_view& a_path)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_activeProcess == nullptr)
    {
        Logger::Warning("GenerateVertexShaderFromFile no active engine process");

        return uint32_t(-1);
    }

    const uint32_t id = Instance->m_vertexShaderID++;
    const uint32_t stringLen = (uint32_t)a_path.length();

    const uint32_t bufferSize = sizeof(uint32_t) + stringLen + 1;

    char* buffer = (char*)malloc(bufferSize);
    IDEFER(free(buffer));

    memset(buffer, 0, bufferSize);

    memcpy(buffer, &id, sizeof(uint32_t));
    memcpy(buffer + sizeof(uint32_t), a_path.data(), stringLen);

    Instance->m_activeProcess->SendRuntimeMessage("Editor:AssetStore:GenerateVertexShaderFromFile", buffer, bufferSize);

    return id;
}
uint32_t RuntimeAssetStore::GeneratePixelShaderFromFile(const std::string_view& a_path)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_activeProcess == nullptr)
    {
        Logger::Warning("GeneratePixelShaderFromFile no active engine process");

        return uint32_t(-1);
    }

    const uint32_t id = Instance->m_pixelShaderID++;
    const uint32_t stringLen = (uint32_t)a_path.length();

    const uint32_t bufferSize = sizeof(uint32_t) + stringLen + 1;

    char* buffer = (char*)malloc(bufferSize);
    IDEFER(free(buffer));

    memset(buffer, 0, bufferSize);

    memcpy(buffer, &id, sizeof(uint32_t));
    memcpy(buffer + sizeof(uint32_t), a_path.data(), stringLen);

    Instance->m_activeProcess->SendRuntimeMessage("Editor:AssetStore:GeneratePixelShaderFromFile", buffer, bufferSize);

    return id;
}

uint32_t RuntimeAssetStore::GenerateMeshFromFile(const std::string_view& a_path)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_activeProcess == nullptr)
    {
        Logger::Warning("GenerateMeshFromFile no active engine process");

        return uint32_t(-1);
    }

    const uint32_t id = Instance->m_meshID++;
    const uint32_t stringLen = (uint32_t)a_path.length();

    const uint32_t bufferSize = sizeof(uint32_t) + stringLen + 1;

    char* buffer = (char*)malloc(bufferSize);
    IDEFER(free(buffer));

    memset(buffer, 0, bufferSize);

    memcpy(buffer, &id, sizeof(uint32_t));
    memcpy(buffer + sizeof(uint32_t), a_path.data(), stringLen);

    Instance->m_activeProcess->SendRuntimeMessage("Editor:AssetStore:GenerateMeshFromFile", buffer, bufferSize);

    return id;
}

uint32_t RuntimeAssetStore::GenerateModelFromFile(const std::string_view& a_path)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_activeProcess == nullptr)
    {
        Logger::Warning("GenerateModelFromFile no active engine process");

        return uint32_t(-1);
    }

    const uint32_t id = Instance->m_modelID++;
    const uint32_t stringLen = (uint32_t)a_path.length();

    const uint32_t bufferSize = sizeof(uint32_t) + stringLen + 1;

    char* buffer = (char*)malloc(bufferSize);
    IDEFER(free(buffer));

    memset(buffer, 0, bufferSize);

    memcpy(buffer, &id, sizeof(uint32_t));
    memcpy(buffer + sizeof(uint32_t), a_path.data(), stringLen);

    Instance->m_activeProcess->SendRuntimeMessage("Editor:AssetStore:GenerateModelFromFile", buffer, bufferSize);

    return id;
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