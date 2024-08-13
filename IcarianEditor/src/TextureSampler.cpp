// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "TextureSampler.h"

#include "Core/IcarianAssert.h"

constexpr static GLenum GetGLFilterMode(e_TextureFilter a_filter)
{
    switch (a_filter) 
    {
    case TextureFilter_Linear:
    {
        return GL_LINEAR;
    }
    case TextureFilter_Nearest:
    {
        return GL_NEAREST;
    }
    }

    ICARIAN_ASSERT_MSG(0, "Invalid filter mode");

    return GL_LINEAR;
}
constexpr static GLenum GetGLWrapMode(e_TextureAddress a_address)
{
    switch (a_address) 
    {
    case TextureAddress_Repeat:
    {
        return GL_REPEAT;
    }
    case TextureAddress_MirroredRepeat:
    {
        return GL_MIRRORED_REPEAT;
    }
    case TextureAddress_ClampToEdge:
    {
        return GL_CLAMP_TO_EDGE;
    }
    }

    ICARIAN_ASSERT_MSG(0, "Invalid address mode");

    return GL_REPEAT;
}

TextureSampler::TextureSampler(GLuint a_sampler)
{
    m_sampler = a_sampler;
}
TextureSampler::~TextureSampler()
{
    glDeleteSamplers(1, &m_sampler);
}

TextureSampler* TextureSampler::CreateSampler(e_TextureFilter a_filter, e_TextureAddress a_addressMode)
{
    GLuint sampler;

    glGenSamplers(1, &sampler);

    const GLenum filterMode = GetGLFilterMode(a_filter);
    const GLenum addressMode = GetGLWrapMode(a_addressMode);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, filterMode);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, filterMode);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, addressMode);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, addressMode);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, addressMode);

    return new TextureSampler(sampler);
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.