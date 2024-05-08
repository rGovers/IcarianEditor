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