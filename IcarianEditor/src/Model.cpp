#include "Model.h"

#include "Flare/ColladaLoader.h"
#include "Flare/OBJLoader.h"
#include "Logger.h"

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
    constexpr FlareBase::Vertex Vertices[] = 
    {
        // 0
        FlareBase::Vertex(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 3
        FlareBase::Vertex(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 6
        FlareBase::Vertex(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 9
        FlareBase::Vertex(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 12
        FlareBase::Vertex(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 15
        FlareBase::Vertex(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 18
        FlareBase::Vertex(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),

        // 21
        FlareBase::Vertex(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
        FlareBase::Vertex(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f)),
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

    return new Model(Vertices, sizeof(Vertices) / sizeof(*Vertices), Indices, sizeof(Indices) / sizeof(*Indices), sizeof(FlareBase::Vertex));
}

Model* Model::FromFile(const std::filesystem::path& a_path)
{
    if (!std::filesystem::exists(a_path))
    {
        Logger::Error("Model does not exist: " + a_path.string());

        return nullptr;
    }

    const std::filesystem::path& ext = a_path.extension();

    std::vector<FlareBase::Vertex> vertices;
    std::vector<uint32_t> indices;

    if (ext == ".dae")
    {
        FlareBase::ColladaLoader_LoadFile(a_path, &vertices, &indices);
    }
    else if (ext == ".obj")
    {
        FlareBase::OBJLoader_LoadFile(a_path, &vertices, &indices);
    }

    const uint32_t vertexCount = (uint32_t)vertices.size();
    const uint32_t indexCount = (uint32_t)indices.size();

    if (vertexCount > 0 && indexCount > 0)
    {
        return new Model(vertices.data(), vertexCount, (GLuint*)indices.data(), indexCount, sizeof(FlareBase::Vertex));
    }

    Logger::Error("Failed to load file: " + a_path.string());

    return nullptr;
}