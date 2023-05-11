#include "Gizmos.h"

#include <imgui.h>
#include <ImGuizmo.h>

// #include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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

    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);   
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void*)offsetof(GizmoVertex, Position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void*)offsetof(GizmoVertex, Color));
}
Gizmos::~Gizmos()
{
    delete m_shader;

    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
    glDeleteVertexArrays(1, &m_vao);
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
    const glm::vec3 vec = a_end - a_start;

    const float mag = glm::length(vec);

    const glm::vec3 dir = vec / mag;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::abs(glm::dot(dir, up)) >= 0.95f)
    {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    const glm::vec3 c = glm::cross(dir, up);
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
void Gizmos::Render()
{
    const GLuint indexCount = (GLuint)Instance->m_indices.size();
    const GLuint vertexCount = (GLuint)Instance->m_vertices.size();
    if (indexCount > 0 && vertexCount > 0)
    {
        const glm::mat4 mat = Instance->m_proj * Instance->m_view;

        glDisable(GL_CULL_FACE);

        glBindBuffer(GL_ARRAY_BUFFER, Instance->m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GizmoVertex), Instance->m_vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Instance->m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), Instance->m_indices.data(), GL_STATIC_DRAW);

        glUseProgram(Instance->m_shader->GetHandle());

        glUniformMatrix4fv(0, 1, GL_FALSE, (GLfloat*)&mat);

        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, NULL);

        glEnable(GL_CULL_FACE);
    }

    Instance->m_vertices.clear();
    Instance->m_indices.clear();
}