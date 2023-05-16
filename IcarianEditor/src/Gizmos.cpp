#include "Gizmos.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <unordered_map>

#include "PixelShader.h"
#include "Runtime/RuntimeManager.h"
#include "ShaderProgram.h"
#include "Shaders/GizmoPixel.h"
#include "Shaders/GizmoVertex.h"
#include "VertexShader.h"

static Gizmos* Instance = nullptr;

struct TransformValue
{
    glm::vec3 Translation;
    glm::quat Rotation;
    glm::vec3 Scale;
};

#define GIZMOS_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define GIZMOS_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEditor, Gizmos, DrawLine, { Gizmos::DrawLine(a_start, a_end, a_width, a_color); }, glm::vec3 a_start, glm::vec3 a_end, float a_width, glm::vec4 a_color) \
    F(void, IcarianEditor, Gizmos, DrawIcoSphere, { Gizmos::DrawIcoSphere(a_pos, a_radius, a_subDivisions, a_width, a_color); }, glm::vec3 a_pos, float a_radius, uint32_t a_subDivisions, float a_width, glm::vec4 a_color) \
    \
    F(uint32_t, IcarianEditor, Gizmos, GetManipulating, { return (uint32_t)ImGuizmo::IsUsing(); })

GIZMOS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

FLARE_MONO_EXPORT(TransformValue, RUNTIME_FUNCTION_NAME(Gizmos, GetManipulation), uint32_t a_mode, glm::vec3 a_translation, glm::quat a_rotation, glm::vec3 a_scale)
{
    TransformValue t;
    t.Translation = a_translation;
    t.Rotation = a_rotation;
    t.Scale = a_scale;

    Gizmos::Manipulation((e_ManipulationMode)a_mode, &t.Translation, &t.Rotation, &t.Scale);

    return t;
}

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
    VertexShader* vShader = VertexShader::GenerateShader(GIZMOVERTEX);
    PixelShader* pShader = PixelShader::GenerateShader(GIZMOPIXEL);

    m_shader = ShaderProgram::GenerateProgram(vShader, pShader);

    delete vShader;
    delete pShader;

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

void Gizmos::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        Instance = new Gizmos();

        GIZMOS_BINDING_FUNCTION_TABLE(GIZMOS_RUNTIME_ATTACH);

        BIND_FUNCTION(a_runtime, IcarianEditor, Gizmos, GetManipulation);
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

    glm::mat4 transform = glm::scale(glm::toMat4(*a_rotation) * glm::translate(glm::identity<glm::mat4>(), *a_translation), *a_scale);

    const bool ret = ImGuizmo::Manipulate((float*)&Instance->m_view, (float*)&p, GetOperation(a_mode), ImGuizmo::MODE::WORLD, (float*)&transform);
    if (ret)
    {
        glm::vec3 skew;
        glm::vec4 per;
        glm::decompose(transform, *a_scale, *a_rotation, *a_translation, skew, per);
    }

    return ret;
}


void Gizmos::DrawLine(const glm::vec3& a_start, const glm::vec3& a_end, float a_width, const glm::vec4& a_color)
{
    const float halfWidth = a_width * 0.5f;

    const glm::vec3 dir = glm::normalize(a_end - a_start);

    const glm::vec3 c = glm::normalize(glm::cross(Instance->m_forward, dir));
    const glm::vec3 hC = (c * halfWidth);

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

void Gizmos::DrawIcoSphere(const glm::vec3& a_pos, float a_radius, uint32_t a_subDivisions, float a_width, const glm::vec4& a_color)
{
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

        Gizmos::DrawLine(vertA, vertB, a_width, a_color);
        Gizmos::DrawLine(vertB, vertC, a_width, a_color);
        Gizmos::DrawLine(vertC, vertA, a_width, a_color);
    }
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