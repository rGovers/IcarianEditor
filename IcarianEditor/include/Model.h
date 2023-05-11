#pragma once

#include <filesystem>
#include <glad/glad.h>

#include "Flare/Vertices.h"

class Model
{
private:
    GLuint   m_vbo;
    GLuint   m_ibo;

    uint32_t m_indexCount;

protected:

public:
    Model(const void* a_vertices, uint32_t a_vertexCount, const GLuint* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride);
    ~Model();

    inline GLuint GetVBO() const
    {
        return m_vbo;
    }
    inline GLuint GetIBO() const
    {
        return m_ibo;
    }

    inline uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }

    static Model* CreateCube();

    static Model* FromFile(const std::filesystem::path& a_path);
};