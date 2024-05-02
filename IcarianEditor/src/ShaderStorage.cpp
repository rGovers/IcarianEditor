#include "ShaderStorage.h"

#include "Texture.h"
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

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, binding.Tex->GetHandle());
        glUniform1i((GLint)binding.Slot, (GLint)i);
    }

    if (m_userUBOSlot != -1 && m_userUniformBuffer != nullptr)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, m_userUBOSlot, m_userUniformBuffer->GetHandle());
    }
}

void ShaderStorage::SetTexture(uint32_t a_slot, Texture* a_texture)
{
    const uint32_t size = (uint32_t)m_texBindings.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_texBindings[i].Slot == a_slot)
        {
            m_texBindings[i].Tex = a_texture;

            return;
        }
    }

    const TextureBinding binding =
    {
        .Slot = a_slot,
        .Tex = a_texture
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