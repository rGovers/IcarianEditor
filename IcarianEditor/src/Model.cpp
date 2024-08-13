// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Model.h"

#include "EngineModelInteropStructures.h"

Model::Model(const void* a_vertices, uint32_t a_vertexCount, const GLuint* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride)
{
    m_indexCount = a_indexCount;

    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)a_vertexCount * a_vertexStride, a_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)m_indexCount * sizeof(GLuint), a_indices, GL_STATIC_DRAW);
}
Model::~Model()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
}

Model* Model::CreateCube()
{
    constexpr Vertex Vertices[] = 
    {
        // 0
        Vertex(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 3
        Vertex(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 6
        Vertex(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 9
        Vertex(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 12
        Vertex(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 15
        Vertex(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 18
        Vertex(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 21
        Vertex(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        Vertex(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
    };

    constexpr GLuint Indices[] = 
    {
        0,  6,  18, 0,  18, 12,
        1,  4,  10, 1,  10, 7,
        2,  17, 5,  2,  14, 17,
        3,  21, 9,  3,  15, 21,
        8,  11, 23, 8,  23, 20,
        13, 22, 16, 13, 19, 22
    };

    return new Model(Vertices, sizeof(Vertices) / sizeof(*Vertices), Indices, sizeof(Indices) / sizeof(*Indices), sizeof(Vertex));
}

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