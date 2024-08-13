// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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

    std::vector<GizmoVertex>  m_vertices;
    std::vector<GLuint>       m_indices;

    glm::mat4                 m_view;
    glm::mat4                 m_proj;
    glm::vec3                 m_forward;

    Gizmos();
    
    static void DrawUVHemisphere(const glm::vec3& a_pos, float a_dir, float a_radius, uint32_t a_subDivisions, float a_width = 0.1f, const glm::vec4& a_color = glm::vec4(1.0f));

protected:

public:
    ~Gizmos();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static void SetMatrices(const glm::mat4& a_view, const glm::mat4& a_proj);

    static bool Manipulation(e_ManipulationMode a_mode, glm::vec3* a_translation, glm::quat* a_rotation, glm::vec3* a_scale);

    static void DrawLine(const glm::vec3& a_start, const glm::vec3& a_end, float a_width = 0.1f, const glm::vec4& a_color = glm::vec4(1.0f));
    
    static void DrawIcoSphere(const glm::vec3& a_pos, float a_radius, uint32_t a_subDivisions, float a_width = 0.1f, const glm::vec4& a_color = glm::vec4(1.0f));
    static void DrawUVSphere(const glm::vec3& a_pos, float a_radius, uint32_t a_subDivisions, float a_width = 0.1f, const glm::vec4& a_color = glm::vec4(1.0f));

    static void DrawCylinder(const glm::vec3& a_pos, float a_height, float a_radius, uint32_t a_subDivisions, float a_width = 0.1f, const glm::vec4& a_color = glm::vec4(1.0f));
    static void DrawCapsule(const glm::vec3& a_pos, float a_height, float a_radius, uint32_t a_subDivisions, float a_width = 0.1f, const glm::vec4& a_color = glm::vec4(1.0f));

    static void Render();
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