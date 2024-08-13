// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "ShaderStorage.h"

#include "Runtime/RuntimeStorage.h"
#include "Texture.h"
#include "TextureSampler.h"
#include "UniformBuffer.h"

ShaderStorage::ShaderStorage(RuntimeStorage* a_runtimeStorage)
{
    m_storage = a_runtimeStorage;

    m_userUBOSlot = -1;
    m_userUniformBuffer = nullptr;
}
ShaderStorage::~ShaderStorage()
{
    if (m_userUniformBuffer != nullptr)
    {
        delete m_userUniformBuffer;
    }
}

void ShaderStorage::Bind()
{
    const uint32_t size = (uint32_t)m_texBindings.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        const TextureBinding& binding = m_texBindings[i];

        const TextureSamplerBuffer buffer = m_storage->GetSamplerBuffer(binding.Sampler);
        const Texture* tex = m_storage->GetTexture(buffer.Addr);
        const TextureSampler* sampler = (TextureSampler*)buffer.Data;

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, tex->GetHandle());
        glBindSampler(i, sampler->GetHandle());
        glUniform1i((GLint)binding.Slot, (GLint)i);
    }

    if (m_userUBOSlot != -1 && m_userUniformBuffer != nullptr)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, m_userUBOSlot, m_userUniformBuffer->GetHandle());
    }
}

void ShaderStorage::SetTexture(uint32_t a_slot, uint32_t a_sampler)
{
    const uint32_t size = (uint32_t)m_texBindings.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_texBindings[i].Slot == a_slot)
        {
            m_texBindings[i].Sampler = a_sampler;

            return;
        }
    }

    const TextureBinding binding =
    {
        .Slot = a_slot,
        .Sampler = a_sampler
    };

    m_texBindings.emplace_back(binding);
}
void ShaderStorage::SetUserUBO(uint32_t a_slot, const void* a_object, uint32_t a_size)
{
    m_userUBOSlot = a_slot;

    if (m_userUniformBuffer != nullptr)
    {
        m_userUniformBuffer->WriteBuffer(a_object, a_size);
    }
    else
    {
        m_userUniformBuffer = new UniformBuffer(a_object, a_size);
    }
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