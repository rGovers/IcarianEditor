#pragma once

#include <cstdint>
#include <glad/glad.h>

class ShaderStorageObject
{
private:
    uint64_t m_size;
    GLuint   m_handle;

protected:

public:
    ShaderStorageObject();
    ~ShaderStorageObject();

    inline GLuint GetHandle() const
    {
        return m_handle;
    }

    void WriteBuffer(const void* a_data, uint16_t a_stride, uint32_t a_count);
};