// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Gizmos.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <unordered_map>

#include "Core/IcarianDefer.h"
#include "PixelShader.h"
#include "Runtime/RuntimeManager.h"
#include "ShaderProgram.h"
#include "Shaders.h"
#include "VertexShader.h"

static Gizmos* Instance = nullptr;

#define GIZMOS_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEditor, Gizmos, DrawLine, { Gizmos::DrawLine(a_start, a_end, a_width, a_colour); }, glm::vec3 a_start, glm::vec3 a_end, float a_width, glm::vec4 a_colour) \
    F(void, IcarianEditor, Gizmos, MultiDrawLine, { Gizmos::MultiDrawLine(a_start, a_end, a_width, a_colour, a_dir, a_delta, a_count); }, const glm::vec3 a_start, const glm::vec3 a_end, float a_width, const glm::vec4 a_colour, const glm::vec3 a_dir, float a_delta, uint32_t a_count) \
    F(void, IcarianEditor, Gizmos, DrawUVSphere, { Gizmos::DrawUVSphere(a_pos, a_radius, a_subDivisions, a_width, a_colour); }, glm::vec3 a_pos, float a_radius, uint32_t a_subDivisions, float a_width, glm::vec4 a_colour) \
    F(void, IcarianEditor, Gizmos, DrawIcoSphere, { Gizmos::DrawIcoSphere(a_pos, a_radius, a_subDivisions, a_width, a_colour); }, glm::vec3 a_pos, float a_radius, uint32_t a_subDivisions, float a_width, glm::vec4 a_colour) \
    F(void, IcarianEditor, Gizmos, DrawCylinder, { Gizmos::DrawCylinder(a_pos, a_height, a_radius, a_subDivisions, a_width, a_colour); }, glm::vec3 a_pos, float a_height, float a_radius, uint32_t a_subDivisions, float a_width, glm::vec4 a_colour) \
    F(void, IcarianEditor, Gizmos, DrawCapsule, { Gizmos::DrawCapsule(a_pos, a_height, a_radius, a_subDivisions, a_width, a_colour); }, glm::vec3 a_pos, float a_height, float a_radius, uint32_t a_subDivisions, float a_width, glm::vec4 a_colour) \
    \
    F(uint32_t, IcarianEditor, Gizmos, GetManipulating, { return (uint32_t)ImGuizmo::IsUsing(); })

GIZMOS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(TransformValue, Gizmos, GetManipulation,
{
    TransformValue t;
    t.Translation = a_translation;
    t.Rotation = a_rotation;
    t.Scale = a_scale;

    Gizmos::Manipulation((e_ManipulationMode)a_mode, &t.Translation, &t.Rotation, &t.Scale);

    return t;
}, uint32_t a_mode, glm::vec3 a_translation, glm::quat a_rotation, glm::vec3 a_scale)

static constexpr ImGuizmo::OPERATION GetOperation(e_ManipulationMode a_mode)
{
    switch (a_mode)
    {
    case ManipulationMode_Translate:
    {
        return ImGuizmo::TRANSLATE;
    }
    case ManipulationMode_Rotate:
    {
        return ImGuizmo::ROTATE;
    }
    case ManipulationMode_Scale:
    {
        return ImGuizmo::SCALE;
    }
    }

    return ImGuizmo::TRANSLATE;
}

Gizmos::Gizmos()
{
    VertexShader* vShader = VertexShader::GenerateShader(GizmoVertexShader);
    IDEFER(delete vShader);
    PixelShader* pShader = PixelShader::GenerateShader(GizmoPixelShader);
    IDEFER(delete pShader);

    m_shader = ShaderProgram::GenerateProgram(vShader, pShader);

    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
}
Gizmos::~Gizmos()
{
    delete m_shader;

    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
}

void Gizmos::Init()
{
    if (Instance == nullptr)
    {
        Instance = new Gizmos();

        GIZMOS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

        BIND_FUNCTION(IcarianEditor, Gizmos, GetManipulation);
    }
}
void Gizmos::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void Gizmos::SetMatrices(const glm::mat4& a_view, const glm::mat4& a_proj)
{
    Instance->m_view = a_view;
    Instance->m_proj = a_proj;

    const glm::mat4 inv = glm::inverse(a_view);

    Instance->m_forward = glm::normalize(inv[2].xyz());
}

bool Gizmos::Manipulation(e_ManipulationMode a_mode, glm::vec3* a_translation, glm::quat* a_rotation, glm::vec3* a_scale)
{
    glm::mat4 p = Instance->m_proj;
    // Need to flip projection matrix as in screen space instead of buffer space
    p[1][1] *= -1;

    const glm::mat4 translationMatrix = glm::translate(glm::identity<glm::mat4>(), *a_translation);
    const glm::mat4 rotationMatrix = glm::toMat4(*a_rotation);
    const glm::mat4 scaleMatrix = glm::scale(glm::identity<glm::mat4>(), *a_scale);

    glm::mat4 transform = translationMatrix * rotationMatrix * scaleMatrix;

    const bool ret = ImGuizmo::Manipulate((float*)&Instance->m_view, (float*)&p, GetOperation(a_mode), ImGuizmo::MODE::WORLD, (float*)&transform);
    if (ret)
    {
        glm::vec3 skew;
        glm::vec4 per;
        glm::decompose(transform, *a_scale, *a_rotation, *a_translation, skew, per);
    }

    return ret;
}

void Gizmos::DrawUVHemisphere(const glm::vec3& a_pos, float a_dir, float a_radius, uint32_t a_subDivisions, float a_width, const glm::vec4& a_color)
{
    const uint32_t halfSub = a_subDivisions / 2;

    for (uint32_t x = 0; x < a_subDivisions; ++x)
    {
        const float angleA = (float(x + 0) / a_subDivisions) * glm::two_pi<float>();
        const float angleB = (float(x + 1) / a_subDivisions) * glm::two_pi<float>();

        const glm::vec2 dirA = glm::vec2(glm::sin(angleA), glm::cos(angleA));
        const glm::vec2 dirB = glm::vec2(glm::sin(angleB), glm::cos(angleB));

        for (uint32_t y = 0; y < halfSub; ++y)
        {
            const float lowFac = float(y + 0) / halfSub;
            const float highFac = float(y + 1) / halfSub;
            const float lowInvFac = 1 - lowFac;
            const float highInvFac = 1 - highFac;

            const glm::vec3 lowAngleA = glm::normalize(glm::vec3(dirA.x * lowInvFac, lowFac * a_dir, dirA.y * lowInvFac)) * a_radius;
            const glm::vec3 lowAngleB = glm::normalize(glm::vec3(dirB.x * lowInvFac, lowFac * a_dir, dirB.y * lowInvFac)) * a_radius;
            const glm::vec3 highAngleA = glm::normalize(glm::vec3(dirA.x * highInvFac, highFac * a_dir, dirA.y * highInvFac)) * a_radius;

            DrawLine(a_pos + lowAngleA, a_pos + lowAngleB, a_width, a_color);
            DrawLine(a_pos + lowAngleA, a_pos + highAngleA, a_width, a_color);
        }
    }
}

void Gizmos::DrawLine(const glm::vec3& a_start, const glm::vec3& a_end, float a_width, const glm::vec4& a_color)
{
    const float halfWidth = a_width * 0.5f;

    const glm::vec3 dir = glm::normalize(a_end - a_start);

    const glm::vec3 c = glm::normalize(glm::cross(Instance->m_forward, dir));
    const glm::vec3 hC = c * halfWidth;

    const GLuint startIndex = (GLuint)Instance->m_vertices.size();

    Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(a_start + hC, 1.0f), a_color));
    Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(a_start - hC, 1.0f), a_color));
    Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(a_end + hC, 1.0f), a_color));
    Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(a_end - hC, 1.0f), a_color));

    Instance->m_indices.emplace_back(startIndex + 0);
    Instance->m_indices.emplace_back(startIndex + 1);
    Instance->m_indices.emplace_back(startIndex + 2);

    Instance->m_indices.emplace_back(startIndex + 1);
    Instance->m_indices.emplace_back(startIndex + 3);
    Instance->m_indices.emplace_back(startIndex + 2);
}
void Gizmos::MultiDrawLine(const glm::vec3& a_start, const glm::vec3& a_end, float a_width, const glm::vec4& a_color, const glm::vec3& a_dir, float a_delta, uint32_t a_count)
{
    const float halfWidth = a_width * 0.5f;

    const glm::vec3 dir = glm::normalize(a_end - a_start);

    const glm::vec3 c = glm::normalize(glm::cross(Instance->m_forward, dir));
    const glm::vec3 hC = c * halfWidth;

    const uint32_t startIndex = (uint32_t)Instance->m_vertices.size();

    const glm::vec3 vA = a_start + hC;
    const glm::vec3 vB = a_start - hC;
    const glm::vec3 vC = a_end + hC;
    const glm::vec3 vD = a_end - hC;

    for (uint32_t i = 0; i < a_count; ++i)
    {
        const glm::vec3 s = a_dir * (float)i * a_delta;

        Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(vA + s, 1.0f), a_color));
        Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(vB + s, 1.0f), a_color));
        Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(vC + s, 1.0f), a_color));
        Instance->m_vertices.emplace_back(GizmoVertex(glm::vec4(vD + s, 1.0f), a_color));

        const GLuint offset = (GLuint)(startIndex + i * 4);

        Instance->m_indices.emplace_back(offset + 0);
        Instance->m_indices.emplace_back(offset + 1);
        Instance->m_indices.emplace_back(offset + 2);

        Instance->m_indices.emplace_back(offset + 1);
        Instance->m_indices.emplace_back(offset + 3);
        Instance->m_indices.emplace_back(offset + 2);
    }
}

static uint32_t PlaceVertex(std::unordered_map<uint64_t, uint32_t>* a_map, uint32_t a_indexA, uint32_t a_indexB, std::vector<glm::vec3>* a_vertexData)
{
    const uint64_t key = (uint64_t)a_indexA << 32 | a_indexB;
    
    const auto iter = a_map->find(key);
    if (iter != a_map->end())
    {
        return iter->second;
    }

    const glm::vec3 vA = a_vertexData->at(a_indexA);
    const glm::vec3 vB = a_vertexData->at(a_indexB);

    const uint32_t size = a_vertexData->size();

    a_vertexData->emplace_back(glm::normalize(vA + vB));

    a_map->emplace(key, size);

    return size;
}

void Gizmos::DrawUVSphere(const glm::vec3 &a_pos, float a_radius, uint32_t a_subDivisions, float a_width, const glm::vec4& a_color)
{
    DrawUVHemisphere(a_pos, 1.0f, a_radius, a_subDivisions, a_width, a_color);
    DrawUVHemisphere(a_pos, -1.0f, a_radius, a_subDivisions, a_width, a_color);
}
void Gizmos::DrawIcoSphere(const glm::vec3& a_pos, float a_radius, uint32_t a_subDivisions, float a_width, const glm::vec4& a_color)
{
    // TODO: Wasteful could improve down the line
    constexpr float X = 0.525731112119133606f;
    constexpr float Z = 0.850650808352039932f;
    constexpr float N = 0.0f;

    constexpr glm::vec3 BaseVertices[] =
    {
        { -X, N, Z }, { X, N, Z }, { -X, N, -Z }, { X, N, -Z },
        { N, Z, X }, { N, Z, -X }, { N, -Z, X }, { N, -Z, -X },
        { Z, X, N }, { -Z, X, N }, { Z, -X, N }, { -Z, -X, N }
    };

    constexpr GLuint BaseIndices[] =
    {
        0, 4, 1,    0, 9, 4,    9, 5, 4,    4, 5, 8,    4, 8, 1,
        8, 10, 1,   8, 3, 10,   5, 3, 8,    5, 2, 3,    2, 7, 3,
        7, 10, 3,   7, 6, 10,   7, 11, 6,   11, 0, 6,   0, 1, 6,
        6, 1, 10,   9, 0, 11,   9, 11, 2,   9, 2, 5,    7, 2, 11
    };

    constexpr uint32_t BaseVertexCount = sizeof(BaseVertices) / sizeof(*BaseVertices);
    constexpr uint32_t BaseIndexCount = sizeof(BaseIndices) / sizeof(*BaseIndices);

    std::vector<glm::vec3> vertices = std::vector<glm::vec3>(BaseVertices, BaseVertices + BaseVertexCount);
    std::vector<uint32_t> indices = std::vector<uint32_t>(BaseIndices, BaseIndices + BaseIndexCount);

    for (uint32_t i = 0; i < a_subDivisions; ++i)
    {
        std::unordered_map<uint64_t, uint32_t> indexMap;

        const uint32_t indexCount = (uint32_t)indices.size();
        std::vector<glm::vec3> newVertices = std::vector<glm::vec3>(vertices.begin(), vertices.end());
        newVertices.reserve(vertices.size() * 2);
        std::vector<uint32_t> newIndices = std::vector<uint32_t>();
        newIndices.reserve(indices.size() * 4);

        const uint32_t triCount = indexCount / 3;

        for (uint32_t j = 0; j < triCount; ++j)
        {
            const uint32_t indexA = indices[j * 3 + 0];
            const uint32_t indexB = indices[j * 3 + 1];
            const uint32_t indexC = indices[j * 3 + 2];

            const uint32_t midA = PlaceVertex(&indexMap, indexA, indexB, &newVertices);
            const uint32_t midB = PlaceVertex(&indexMap, indexB, indexC, &newVertices);
            const uint32_t midC = PlaceVertex(&indexMap, indexC, indexA, &newVertices);

            newIndices.emplace_back(indexA); newIndices.emplace_back(midA); newIndices.emplace_back(midC);
            newIndices.emplace_back(indexB); newIndices.emplace_back(midB); newIndices.emplace_back(midA);
            newIndices.emplace_back(indexC); newIndices.emplace_back(midC); newIndices.emplace_back(midB);
            newIndices.emplace_back(midA); newIndices.emplace_back(midB); newIndices.emplace_back(midC);
        }

        indices = newIndices;
        vertices = newVertices;
    }

    const uint32_t indexCount = (uint32_t)indices.size();

    const uint32_t triCount = indexCount / 3;
    for (uint32_t i = 0; i < triCount; ++i)
    {
        const uint32_t indexA = indices[i * 3 + 0];
        const uint32_t indexB = indices[i * 3 + 1];
        const uint32_t indexC = indices[i * 3 + 2];

        const glm::vec3 vertA = a_pos + vertices[indexA] * a_radius;
        const glm::vec3 vertB = a_pos + vertices[indexB] * a_radius;
        const glm::vec3 vertC = a_pos + vertices[indexC] * a_radius;

        DrawLine(vertA, vertB, a_width, a_color);
        DrawLine(vertB, vertC, a_width, a_color);
        DrawLine(vertC, vertA, a_width, a_color);
    }
}

void Gizmos::DrawCylinder(const glm::vec3 &a_pos, float a_height, float a_radius, uint32_t a_subDivisions, float a_width, const glm::vec4& a_color)
{
    const glm::vec3 cylinderTop = glm::vec3(0.0f, 1.0f, 0.0f) * a_height;
    const glm::vec3 topPos = a_pos + cylinderTop;
    const glm::vec3 bottomPos = a_pos - cylinderTop;

    for (uint32_t i = 0; i < a_subDivisions; ++i)
    {
        const float angleA = (float(i + 0) / a_subDivisions) * glm::two_pi<float>();
        const float angleB = (float(i + 1) / a_subDivisions) * glm::two_pi<float>();

        const glm::vec3 dirA = glm::vec3(glm::sin(angleA), 0.0f, glm::cos(angleA)) * a_radius;
        const glm::vec3 dirB = glm::vec3(glm::sin(angleB), 0.0f, glm::cos(angleB)) * a_radius;

        DrawLine(topPos + dirA, topPos + dirB, a_width, a_color);
        DrawLine(bottomPos + dirA, bottomPos + dirB, a_width, a_color);
    }

    const glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f) * a_radius;
    const glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f) * a_radius;

    DrawLine(topPos + right, bottomPos + right, a_width, a_color);
    DrawLine(topPos - right, bottomPos - right, a_width, a_color);
    DrawLine(topPos + forward, bottomPos + forward, a_width, a_color);
    DrawLine(topPos - forward, bottomPos - forward, a_width, a_color);
}
void Gizmos::DrawCapsule(const glm::vec3& a_pos, float a_height, float a_radius, uint32_t a_subDivisions, float a_width, const glm::vec4& a_color)
{
    const glm::vec3 cylinderTop = glm::vec3(0.0f, 1.0f, 0.0f) * (a_height - a_radius);
    const glm::vec3 topPos = a_pos + cylinderTop;
    const glm::vec3 bottomPos = a_pos - cylinderTop;

    const glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f) * a_radius;
    const glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f) * a_radius;

    DrawUVHemisphere(topPos, 1.0f, a_radius, a_subDivisions, a_width, a_color);
    DrawUVHemisphere(bottomPos, -1.0f, a_radius, a_subDivisions, a_width, a_color);

    DrawLine(topPos + right, bottomPos + right, a_width, a_color);
    DrawLine(topPos - right, bottomPos - right, a_width, a_color);
    DrawLine(topPos + forward, bottomPos + forward, a_width, a_color);
    DrawLine(topPos - forward, bottomPos - forward, a_width, a_color);
}

void Gizmos::Render()
{
    const GLuint indexCount = (GLuint)Instance->m_indices.size();
    const GLuint vertexCount = (GLuint)Instance->m_vertices.size();
    if (indexCount > 0 && vertexCount > 0)
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(Instance->m_shader->GetHandle());

        glBindBuffer(GL_ARRAY_BUFFER, Instance->m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GizmoVertex), Instance->m_vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Instance->m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), Instance->m_indices.data(), GL_STATIC_DRAW);

        glBindVertexBuffer(0, Instance->m_vbo, 0, sizeof(GizmoVertex));

        glEnableVertexAttribArray(0);
        glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, (GLuint)offsetof(GizmoVertex, Position));
        glVertexAttribBinding(0, 0);
        glEnableVertexAttribArray(1);
        glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, (GLuint)offsetof(GizmoVertex, Color));
        glVertexAttribBinding(1, 0);

        glUniformMatrix4fv(0, 1, GL_FALSE, (GLfloat*)&Instance->m_view);
        glUniformMatrix4fv(1, 1, GL_FALSE, (GLfloat*)&Instance->m_proj);

        glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, NULL);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    Instance->m_vertices.clear();
    Instance->m_indices.clear();
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