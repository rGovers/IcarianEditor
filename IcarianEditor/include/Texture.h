#pragma once 

#include <cstdint>
#include <glad/glad.h>

class Texture
{
private:
    GLuint m_handle;


protected:

public:
    Texture(GLuint a_handle);
    ~Texture();

    static Texture* CreateRGBA(uint32_t a_width, uint32_t a_height, const unsigned char* a_data);

    inline GLuint GetHandle() const
    {
        return m_handle;
    }
};