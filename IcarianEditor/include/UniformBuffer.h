#pragma once

#include <glad/glad.h>

class UniformBuffer
{
private:
    GLuint m_handle;

protected:

public:
    UniformBuffer(const void* a_object, uint32_t a_size);
    ~UniformBuffer();

    inline GLuint GetHandle() const
    {
        return m_handle;
    }

    void WriteBuffer(const void* a_object, uint32_t a_size) const;
};