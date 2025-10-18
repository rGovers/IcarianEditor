// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

class EngineProcess;

struct MeshTable
{
    uint32_t MaterialID;
    uint32_t MeshID;
    uint32_t IndexCount;

    uint32_t TransformCount;
    uint32_t TransformCapacity;
    glm::mat4* Transforms;
};

struct ModelTable
{
    uint32_t MaterialID;
    uint32_t ModelID;

    uint32_t TransformCount;
    uint32_t TransformCapapcity;
    glm::mat4* Transforms;
};

#include "EditorLightInteropStructures.h"

class RenderCommand
{
private:
    uint32_t              m_ambientLightCount;
    uint32_t              m_ambientLightCapacity;
    AmbientLightData*     m_ambientLightData;

    uint32_t              m_directionalLightCount;
    uint32_t              m_directionalLightCapacity;
    DirectionalLightData* m_directionalLightData;

    uint32_t              m_pointLightCount;
    uint32_t              m_pointLightCapacity;
    PointLightData*       m_pointLightData;

    uint32_t              m_spotLightCount;
    uint32_t              m_spotLightCapacity;
    SpotLightData*        m_spotLightData;

    uint32_t              m_meshTableCount;
    uint32_t              m_meshTableCapacity;
    MeshTable*            m_meshTables;

    uint32_t              m_modelTableCount;
    uint32_t              m_modelTableCapacity;
    ModelTable*           m_modelTables;

    uint32_t              m_currentMaterial;

    RenderCommand();

    static void FlushAmbientLight(EngineProcess* a_process);
    static void FlushDirectionalLight(EngineProcess* a_process);
    static void FlushPointLight(EngineProcess* a_process);
    static void FlushSpotLight(EngineProcess* a_process);

    static void FlushMesh(EngineProcess* a_process);
    static void FlushModel(EngineProcess* a_process);

protected:

public:
    ~RenderCommand();

    static void Init();
    static void Destroy();

    static void Flush(EngineProcess* a_process);

    static void BindMaterial(uint32_t a_materialAddr);

    static void PushAmbientLight(float a_intensity, const glm::vec4& a_color);
    static void PushDirectionalLight(const glm::mat4& a_transform, float a_intensity, const glm::vec4& a_color);
    static void PushPointLight(const glm::mat4& a_transform, float a_intensity, float a_radius, const glm::vec4& a_color);
    static void PushSpotLight(const glm::mat4& a_transform, float a_intensity, float a_radius, float a_innerCutoffAngle, float a_outerCutoffAngle, const glm::vec4& a_color);

    static void DrawMesh(const glm::mat4& a_transform, uint32_t a_meshAddr, uint32_t a_indexCount);
    static void DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr);
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