#include "Texture.h"

Texture::Texture(GLuint a_handle)
{
    m_handle = a_handle;

    glBindTexture(GL_TEXTURE_2D, m_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
Texture::~Texture()
{
    glDeleteTextures(1, &m_handle);
}

Texture* Texture::CreateRGBA(uint32_t a_width, uint32_t a_height, const unsigned char* a_data)
{
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, (GLsizei)a_width, (GLsizei)a_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_data);

    return new Texture(handle);
}