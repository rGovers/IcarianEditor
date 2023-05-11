#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const void* a_object, uint32_t a_size)
{
    glGenBuffers(1, &m_handle);

    glBindBuffer(GL_UNIFORM_BUFFER, m_handle);

    glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)a_size, a_object, GL_DYNAMIC_DRAW);
}
UniformBuffer::~UniformBuffer()
{  
    glDeleteBuffers(1, &m_handle); 
}

void UniformBuffer::WriteBuffer(const void* a_object, uint32_t a_size) const
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_handle);

    glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)a_size, a_object, GL_DYNAMIC_DRAW);
}