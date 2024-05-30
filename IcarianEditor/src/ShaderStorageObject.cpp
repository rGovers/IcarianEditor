#include "ShaderStorageObject.h"

#include <cstddef>

#include "Core/IcarianDefer.h"

ShaderStorageObject::ShaderStorageObject()
{
    int32_t count = 0;

    m_size = sizeof(int32_t);

    glGenBuffers(1, &m_handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int32_t), &count, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
ShaderStorageObject::~ShaderStorageObject()
{
    glDeleteBuffers(1, &m_handle);
}

void ShaderStorageObject::WriteBuffer(const void* a_data, uint16_t a_stride, uint32_t a_count)
{
    IDEFER(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    const int32_t count = (int32_t)a_count;
    const uint64_t dataSize = (uint64_t)a_stride * a_count;
    const uint64_t bufferSize = sizeof(count) + dataSize;

    if (bufferSize > m_size)
    {
        m_size = bufferSize;

        const GLuint oldHandle = m_handle;
        IDEFER(glDeleteBuffers(1, &oldHandle));

        glGenBuffers(1, &m_handle);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)bufferSize, NULL, GL_DYNAMIC_DRAW);
    }
    else
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
    }

    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(count), &count);
    if (a_count > 0)
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(count), (GLsizeiptr)dataSize, a_data);
    }
}