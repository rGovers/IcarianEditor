#pragma once 

#include <cstdint>
#include <glad/glad.h>

class Texture
{
private:
    GLuint m_handle;

protected:

public:
    Texture(uint32_t a_width, uint32_t a_height, const unsigned char* a_data);
    ~Texture();

    inline GLuint GetHandle() const
    {
        return m_handle;
    }
};