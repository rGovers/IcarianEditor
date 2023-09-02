#include "ShaderStorageObject.h"

ShaderStorageObject::ShaderStorageObject(const void* a_object, uint32_t a_size)
{
    glGenBuffers(1, &m_handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, a_size, a_object, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
ShaderStorageObject::~ShaderStorageObject()
{
    glDeleteBuffers(1, &m_handle);
}

void ShaderStorageObject::WriteBuffer(const void* a_object, uint32_t a_size) const
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
    // glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, a_size, a_object);
    glBufferData(GL_SHADER_STORAGE_BUFFER, a_size, a_object, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}