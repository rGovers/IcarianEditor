#pragma once

#include <glad/glad.h>

#include "EngineTextureSamplerInteropStructures.h"

class TextureSampler
{
private:
    GLuint        m_sampler;

protected:

public:
    TextureSampler(GLuint a_sampler);
    ~TextureSampler();

    static TextureSampler* CreateSampler(e_TextureFilter a_filter, e_TextureAddress a_addressMode);

    inline GLuint GetHandle() const
    {
        return m_sampler;
    }
};