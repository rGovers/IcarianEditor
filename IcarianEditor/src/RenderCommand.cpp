// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "RenderCommand.h"

#include <cstring>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "EngineProcess.h"
#include "Logger.h"
#include "Runtime/RuntimeManager.h"

static RenderCommand* Instance = nullptr;

#define RENDERCOMMAND_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { RenderCommand::BindMaterial(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawModel, { RenderCommand::DrawModel(a_transform, a_addr); }, glm::mat4 a_transform, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawMesh, { RenderCommand::DrawMesh(a_transform, a_addr, a_indexCount); }, glm::mat4 a_transform, uint32_t a_addr, uint32_t a_indexCount) \

RENDERCOMMAND_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

#include "EditorDrawInteropStructures.h"
#include "EditorRenderCommandInterop.h"

EDITOR_RENDERCOMMAND_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

RenderCommand::RenderCommand()
{
    m_ambientLightCount = 0;
    m_ambientLightCapacity = 0;
    m_ambientLightData = NULL;

    m_directionalLightCount = 0;
    m_directionalLightCapacity = 0;
    m_directionalLightData = NULL;

    m_pointLightCount = 0;
    m_pointLightCapacity = 0;
    m_pointLightData = NULL;

    m_spotLightCount = 0;
    m_spotLightCapacity = 0;
    m_spotLightData = NULL;

    m_meshTableCount = 0;
    m_meshTableCapacity = 0;
    m_meshTables = NULL;

    m_modelTableCount = 0;
    m_modelTableCapacity = 0;
    m_modelTables = NULL;

    m_currentMaterial = uint32_t(-1);

    RENDERCOMMAND_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    EDITOR_RENDERCOMMAND_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
RenderCommand::~RenderCommand()
{
    if (m_ambientLightData != NULL)
    {
        free(m_ambientLightData);
        m_ambientLightData = NULL;
    }
    if (m_directionalLightData != NULL)
    {
        free(m_directionalLightData);
        m_directionalLightData = NULL;
    }
    if (m_pointLightData != NULL)
    {
        free(m_pointLightData);
        m_pointLightData = NULL;
    }
    if (m_spotLightData != NULL)
    {
        free(m_spotLightData);
        m_spotLightData = NULL;
    }

    if (m_meshTables != NULL)
    {
        for (uint32_t i = 0; i < m_meshTableCount; ++i)
        {
            free(m_meshTables[i].Transforms);
        }

        free(m_meshTables);
        m_meshTables = NULL;
    }

    if (m_modelTables != NULL)
    {
        for (uint32_t i = 0; i < m_modelTableCount; ++i)
        {
            free(m_modelTables[i].Transforms);
        }

        free(m_modelTables);
        m_modelTables = NULL;
    }
}

void RenderCommand::Init()
{
    if (Instance == nullptr)
    {
        Instance = new RenderCommand();
    }
}
void RenderCommand::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void RenderCommand::FlushAmbientLight(EngineProcess* a_process)
{
    if (Instance->m_ambientLightCount <= 0)
    {
        return;
    }
    IDEFER(Instance->m_ambientLightCount = 0);

    const uint32_t ambientLightDataSize = Instance->m_ambientLightCount * sizeof(AmbientLightData);
    const uint32_t bufferSize = sizeof(LightHeader) + ambientLightDataSize;

    uint8_t* data = new uint8_t[bufferSize];
    IDEFER(delete[] data);

    const LightHeader header = 
    {
        .Version = 0,
        .DataCount = Instance->m_ambientLightCount,
        .DataOffset = sizeof(LightHeader),
    };

    memcpy(data, &header, sizeof(LightHeader));
    memcpy(data + header.DataOffset, Instance->m_ambientLightData, ambientLightDataSize);

    a_process->SendRuntimeMessage("Editor:SceneView:PushAmbientLight", data, bufferSize);
}
void RenderCommand::FlushDirectionalLight(EngineProcess* a_process)
{
    if (Instance->m_directionalLightCount <= 0)
    {
        return;
    }
    IDEFER(Instance->m_directionalLightCount = 0);

    const uint32_t directionalLightDataSize = Instance->m_directionalLightCount * sizeof(DirectionalLightData);
    const uint32_t bufferSize = sizeof(LightHeader) + directionalLightDataSize;

    uint8_t* data = new uint8_t[bufferSize];
    IDEFER(delete[] data);

    const LightHeader header =
    {
        .Version = 0,
        .DataCount = Instance->m_directionalLightCount,
        .DataOffset = sizeof(LightHeader),
    };

    memcpy(data, &header, sizeof(LightHeader));
    memcpy(data + header.DataOffset, Instance->m_directionalLightData, directionalLightDataSize);

    a_process->SendRuntimeMessage("Editor:SceneView:PushDirectionalLight", data, bufferSize);
}
void RenderCommand::FlushPointLight(EngineProcess* a_process)
{
    if (Instance->m_pointLightCount <= 0)
    {
        return;
    }
    IDEFER(Instance->m_pointLightCount = 0);

    const uint32_t pointLightDataSize = Instance->m_pointLightCount * sizeof(PointLightData);
    const uint32_t bufferSize = sizeof(LightHeader) + pointLightDataSize;

    uint8_t* data = new uint8_t[bufferSize];
    IDEFER(delete[] data);

    const LightHeader header =
    {
        .Version = 0,
        .DataCount = Instance->m_pointLightCount,
        .DataOffset = sizeof(LightHeader)
    };

    memcpy(data, &header, sizeof(LightHeader));
    memcpy(data + header.DataOffset, Instance->m_pointLightData, pointLightDataSize);

    a_process->SendRuntimeMessage("Editor:SceneView:PushPointLight", data, bufferSize);
}
void RenderCommand::FlushSpotLight(EngineProcess* a_process)
{
    if (Instance->m_spotLightCount <= 0)
    {
        return;
    }
    IDEFER(Instance->m_spotLightCount = 0);

    const uint32_t spotLightDataSize = Instance->m_spotLightCount * sizeof(SpotLightData);
    const uint32_t bufferSize = sizeof(LightHeader) + spotLightDataSize;

    uint8_t* data = new uint8_t[bufferSize];
    IDEFER(delete[] data);

    const LightHeader header = 
    {
        .Version = 0,
        .DataCount = Instance->m_spotLightCount,
        .DataOffset = sizeof(LightHeader)
    };

    memcpy(data, &header, sizeof(LightHeader));
    memcpy(data + header.DataOffset, Instance->m_spotLightData, spotLightDataSize);

    a_process->SendRuntimeMessage("Editor:SceneView:PushSpotLight", data, bufferSize);
}

void RenderCommand::FlushMesh(EngineProcess* a_process)
{
    if (Instance->m_meshTableCount <= 0)
    {
        return;
    }

    IDEFER(
    {
        for (uint32_t i = 0; i < Instance->m_meshTableCount; ++i)
        {
            free(Instance->m_meshTables[i].Transforms);
        }

        Instance->m_meshTableCount = 0;
    });

    // TODO: Can probably look ahead and allocate all at once then process in 2nd pass
    uint32_t bufferSize = 0;
    uint32_t bufferCapacity = sizeof(DrawDataHeader);
    uint8_t* buffer = (uint8_t*)malloc(bufferCapacity);
    IDEFER(free(buffer));

    {
        bufferSize += sizeof(DrawDataHeader);

        DrawDataHeader& header = *(DrawDataHeader*)buffer;
        header.Version = 0;
        header.HeaderOffset = bufferSize;
        header.HeaderCount = Instance->m_meshTableCount;
    }

    const uint32_t headerBufferSize = Instance->m_meshTableCount * sizeof(DrawMeshBufferHeader);
    const uint32_t headerBufferOffset = bufferSize;

    bufferSize += headerBufferSize;
    if (bufferSize >= bufferCapacity)
    {
        const uint32_t newCapacity = glm::max(bufferSize, bufferCapacity << 1);

        buffer = (uint8_t*)realloc(buffer, newCapacity);
        bufferCapacity = newCapacity;
    }

    for (uint32_t i = 0; i < Instance->m_meshTableCount; ++i)
    {
        const MeshTable& t = Instance->m_meshTables[i];
        const uint32_t transformOffset = bufferSize;

        {
            // Using realloc on the buffer so have to re get the pointer every iteration
            // Could cache and refresh but lazy and KISS
            DrawMeshBufferHeader* headers = (DrawMeshBufferHeader*)(buffer + headerBufferOffset);

            headers[i].MeshID = t.MeshID;
            headers[i].MaterialID = t.MaterialID;
            headers[i].IndexCount = t.IndexCount;
            headers[i].TransformCount = t.TransformCount;
            headers[i].TransformOffset = transformOffset;
        }

        const uint32_t transformSize = t.TransformCount * sizeof(glm::mat4);
        bufferSize += transformSize;
        if (bufferSize >= bufferCapacity)
        {
            const uint32_t newCapacity = glm::max(bufferSize, bufferCapacity << 1);

            buffer = (uint8_t*)realloc(buffer, newCapacity);
            bufferCapacity = newCapacity;
        }

        glm::mat4* transforms = (glm::mat4*)(buffer + transformOffset);
        for (uint32_t j = 0; j < t.TransformCount; ++j)
        {
            transforms[j] = t.Transforms[j];
        }
    }

    // Should we compress this as it can get large in big scenes?
    a_process->SendRuntimeMessage("Editor:SceneView:DrawMesh", buffer, bufferSize);
}
void RenderCommand::FlushModel(EngineProcess* a_process)
{
    if (Instance->m_modelTableCount <= 0)
    {
        return;
    }

    IDEFER(
    {
        for (uint32_t i = 0; i < Instance->m_modelTableCount; ++i)
        {
            free(Instance->m_modelTables[i].Transforms);
        }

        Instance->m_modelTableCount = 0;
    });

    uint32_t bufferSize = 0;
    uint32_t bufferCapacity = sizeof(DrawDataHeader);
    uint8_t* buffer = (uint8_t*)malloc(bufferCapacity);
    IDEFER(free(buffer));

    {
        bufferSize += sizeof(DrawDataHeader);

        DrawDataHeader& header = *(DrawDataHeader*)buffer;
        header.Version = 0;
        header.HeaderOffset = bufferSize;
        header.HeaderCount = Instance->m_modelTableCount;
    }

    const uint32_t headerBufferSize = Instance->m_modelTableCount * sizeof(DrawModelBufferHeader);
    const uint32_t headerBufferOffset = bufferSize;

    bufferSize += headerBufferSize;
    if (bufferSize >= bufferCapacity)
    {
        const uint32_t newCapacity = glm::max(bufferSize, bufferCapacity << 1);

        buffer = (uint8_t*)realloc(buffer, newCapacity);
        bufferCapacity = newCapacity;
    }

    for (uint32_t i = 0; i < Instance->m_modelTableCount; ++i)
    {
        const ModelTable& t = Instance->m_modelTables[i];
        const uint32_t transformOffset = bufferSize;

        {
            DrawModelBufferHeader* headers = (DrawModelBufferHeader*)(buffer + headerBufferOffset);

            headers[i].ModelID = t.ModelID;
            headers[i].MaterialID = t.MaterialID;
            headers[i].TransformCount = t.TransformCount;
            headers[i].TransformOffset = transformOffset;
        }

        const uint32_t transformSize = t.TransformCount * sizeof(glm::mat4);
        bufferSize += transformSize;
        if (bufferSize >= bufferCapacity)
        {
            const uint32_t newCapacity = glm::max(bufferSize, bufferCapacity << 1);

            buffer = (uint8_t*)realloc(buffer, newCapacity);
            bufferCapacity = newCapacity;
        }

        glm::mat4* transforms = (glm::mat4*)(buffer + transformOffset);
        for (uint32_t j = 0; j < t.TransformCount; ++j)
        {
            transforms[j] = t.Transforms[j];
        }
    }

    a_process->SendRuntimeMessage("Editor:SceneView:DrawModel", buffer, bufferSize);
}

void RenderCommand::Flush(EngineProcess* a_process)
{
    ICARIAN_ASSERT(Instance != nullptr);
    ICARIAN_ASSERT(a_process != nullptr);

    FlushAmbientLight(a_process);
    FlushDirectionalLight(a_process);
    FlushPointLight(a_process);
    FlushSpotLight(a_process);

    FlushMesh(a_process);
    FlushModel(a_process);
}

void RenderCommand::BindMaterial(uint32_t a_materialAddr)
{
    ICARIAN_ASSERT(Instance != nullptr);

    Instance->m_currentMaterial = a_materialAddr;
}

void RenderCommand::PushAmbientLight(float a_intensity, const glm::vec4& a_color)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_ambientLightCount >= Instance->m_ambientLightCapacity)
    {
        const uint32_t newCapacity = glm::max(uint32_t(1), Instance->m_ambientLightCapacity) << 1;

        Instance->m_ambientLightData = (AmbientLightData*)realloc(Instance->m_ambientLightData, newCapacity * sizeof(AmbientLightData));
        Instance->m_ambientLightCapacity = newCapacity;
    }

    AmbientLightData& data = Instance->m_ambientLightData[Instance->m_ambientLightCount++];
    data.Color = a_color;
    data.Intensity = a_intensity;
}
void RenderCommand::PushDirectionalLight(const glm::mat4& a_transform, float a_intensity, const glm::vec4& a_color)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_directionalLightCount >= Instance->m_directionalLightCapacity)
    {
        const uint32_t newCapacity = glm::max(uint32_t(1), Instance->m_directionalLightCapacity) << 1;

        Instance->m_directionalLightData = (DirectionalLightData*)realloc(Instance->m_directionalLightData, newCapacity * sizeof(DirectionalLightData));
        Instance->m_directionalLightCapacity = newCapacity;
    }

    DirectionalLightData& data = Instance->m_directionalLightData[Instance->m_directionalLightCount++];
    data.Color = a_color;
    data.Intensity = a_intensity;
    data.Transform = a_transform;
}
void RenderCommand::PushPointLight(const glm::mat4& a_transform, float a_intensity, float a_radius, const glm::vec4& a_color)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_pointLightCount >= Instance->m_pointLightCapacity)
    {
        const uint32_t newCapacity = glm::max(uint32_t(1), Instance->m_pointLightCapacity) << 1;

        Instance->m_pointLightData = (PointLightData*)realloc(Instance->m_pointLightData, newCapacity * sizeof(PointLightData));
        Instance->m_pointLightCapacity = newCapacity;
    }

    PointLightData& data = Instance->m_pointLightData[Instance->m_pointLightCount++];
    data.Color = a_color;
    data.Data = glm::vec2(a_intensity, a_radius);
    data.Transform = a_transform;
}
void RenderCommand::PushSpotLight(const glm::mat4& a_transform, float a_intensity, float a_radius, float a_innerCutoffAngle, float a_outerCutoffAngle, const glm::vec4& a_color)
{
    ICARIAN_ASSERT(Instance != nullptr);

    if (Instance->m_spotLightCount >= Instance->m_spotLightCapacity)
    {
        const uint32_t newCapacity = glm::max(uint32_t(1), Instance->m_spotLightCapacity) << 1;

        Instance->m_spotLightData = (SpotLightData*)realloc(Instance->m_spotLightData, newCapacity * sizeof(SpotLightData));
        Instance->m_spotLightCapacity = newCapacity;
    }

    SpotLightData& data = Instance->m_spotLightData[Instance->m_spotLightCount++];
    data.Color = a_color;
    data.Data = glm::vec4(a_intensity, a_radius, a_innerCutoffAngle, a_outerCutoffAngle);
    data.Transform = a_transform;
}

void RenderCommand::DrawMesh(const glm::mat4& a_transform, uint32_t a_meshAddr, uint32_t a_indexCount)
{
    ICARIAN_ASSERT(Instance != nullptr);

    const uint32_t currentMat = Instance->m_currentMaterial;
    if (currentMat == uint32_t(-1))
    {
        Logger::Warning("DrawMesh no bound Material");

        return;
    }

    if (a_indexCount == uint32_t(-1) && a_meshAddr == uint32_t(-1))
    {
        Logger::Warning("DrawMesh deriving index count from Mesh with no Mesh");

        return;
    }

    for (uint32_t i = 0; i < Instance->m_meshTableCount; ++i)
    {
        MeshTable& t = Instance->m_meshTables[i];

        if (t.MaterialID != currentMat)
        {
            continue;
        }

        if (t.MeshID != a_meshAddr)
        {
            continue;
        }

        if (t.IndexCount != a_indexCount)
        {
            continue;
        }

        if (t.TransformCount >= t.TransformCapacity)
        {
            const uint32_t newCapacity = t.TransformCapacity << 1;

            t.Transforms = (glm::mat4*)realloc(t.Transforms, newCapacity * sizeof(glm::mat4));
            t.TransformCapacity = newCapacity;
        }

        t.Transforms[t.TransformCount++] = a_transform;

        return;
    }

    if (Instance->m_meshTableCount >= Instance->m_meshTableCapacity)
    {
        const uint32_t newCapacity = glm::max(uint32_t(1), Instance->m_meshTableCapacity) << 1;

        Instance->m_meshTables = (MeshTable*)realloc(Instance->m_meshTables, newCapacity * sizeof(MeshTable));
        Instance->m_meshTableCapacity = newCapacity;
    }

    const MeshTable t = 
    {
        .MaterialID = currentMat,
        .MeshID = a_meshAddr,
        .IndexCount = a_indexCount,
        .TransformCount = 1,
        .TransformCapacity = 1,
        .Transforms = ILAMBDA(
        {
            glm::mat4* vals = (glm::mat4*)calloc(1, sizeof(glm::mat4));
            vals[0] = a_transform;

            ILRETURN vals;
        })
    };

    Instance->m_meshTables[Instance->m_meshTableCount++] = t;
}
void RenderCommand::DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr)
{
    ICARIAN_ASSERT(Instance != nullptr);

    const uint32_t currentMat = Instance->m_currentMaterial;
    if (currentMat == uint32_t(-1))
    {
        Logger::Warning("DrawModel no bound Material");

        return;
    }

    if (a_modelAddr == uint32_t(-1))
    {
        Logger::Warning("DrawModel with no Model");

        return;
    }

    for (uint32_t i = 0; i < Instance->m_modelTableCount; ++i)
    {
        ModelTable& t = Instance->m_modelTables[i];

        if (t.MaterialID != currentMat)
        {
            continue;
        }

        if (t.ModelID != a_modelAddr)
        {
            continue;
        }

        if (t.TransformCount >= t.TransformCapapcity)
        {
            const uint32_t newCapacity = t.TransformCapapcity << 1;

            t.Transforms = (glm::mat4*)realloc(t.Transforms, newCapacity * sizeof(glm::mat4));
            t.TransformCapapcity = newCapacity;
        }

        t.Transforms[t.TransformCount++] = a_transform;

        return;
    }

    if (Instance->m_modelTableCount >= Instance->m_modelTableCapacity)
    {
        const uint32_t newCapacity = glm::max(uint32_t(1), Instance->m_modelTableCapacity) << 1;

        Instance->m_modelTables = (ModelTable*)realloc(Instance->m_modelTables, newCapacity * sizeof(ModelTable));
        Instance->m_modelTableCapacity = newCapacity;
    }

    const ModelTable t = 
    {
        .MaterialID = currentMat,
        .ModelID = a_modelAddr,
        .TransformCount = 1,
        .TransformCapapcity = 1,
        .Transforms = ILAMBDA(
        {
            glm::mat4* vals = (glm::mat4*)calloc(1, sizeof(glm::mat4));
            vals[0] = a_transform;

            ILRETURN vals;
        }),
    };

    Instance->m_modelTables[Instance->m_modelTableCount++] = t;
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
