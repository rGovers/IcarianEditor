#include "ShaderStorage.h"

ShaderStorage::ShaderStorage(RuntimeStorage* a_runtimeStorage)
{
    m_storage = a_runtimeStorage;
}
ShaderStorage::~ShaderStorage()
{

}

void ShaderStorage::Bind()
{
    const uint32_t size = (uint32_t)m_texBindings.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_texBindings[i].Tex->GetHandle());
        glUniform1i(m_texBindings[i].Slot, i);
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

    TextureBinding binding;
    binding.Slot = a_slot;
    binding.Tex = a_texture;

    m_texBindings.emplace_back(binding);
}