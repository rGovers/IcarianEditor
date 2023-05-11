#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glad/glad.h>
#include <vector>

class RuntimeManager;
class ShaderProgram;

struct GizmoVertex
{
    glm::vec4 Position;
    glm::vec4 Color;

    constexpr GizmoVertex(const glm::vec4& a_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), const glm::vec4& a_color = glm::vec4(1.0f)) :
        Position(a_pos),
        Color(a_color)
    {

    }
};

enum e_ManipulationMode : uint16_t
{
    ManipulationMode_Translate = 0,
    ManipulationMode_Rotate = 1,
    ManipulationMode_Scale = 2
};

class Gizmos
{
private:
    ShaderProgram*            m_shader;

    GLuint                    m_vbo;
    GLuint                    m_ibo;
                
    GLuint                    m_vao;

    std::vector<GizmoVertex>  m_vertices;
    std::vector<GLuint>       m_indices;

    glm::mat4                 m_view;
    glm::mat4                 m_proj;

    Gizmos();
    
protected:

public:
    ~Gizmos();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static void SetMatrices(const glm::mat4& a_view, const glm::mat4& a_proj);

    static bool Manipulation(e_ManipulationMode a_mode, glm::vec3* a_translation, glm::quat* a_rotation, glm::vec3* a_scale);

    static void DrawLine(const glm::vec3& a_start, const glm::vec3& a_end, float a_width = 0.1f, const glm::vec4& a_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    static void Render();
};