#pragma once

#include <glad/glad.h>

class ShaderStorageObject
{
private:
    GLuint m_handle;

protected:

public:
    ShaderStorageObject(const void* a_object, uint32_t a_size);
    ~ShaderStorageObject();

    inline GLuint GetHandle() const
    {
        return m_handle;
    }

    void WriteBuffer(const void* a_object, uint32_t a_size) const;
};