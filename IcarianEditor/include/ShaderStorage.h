#pragma once

#include <cstdint>
#include <vector>

class RuntimeStorage;
class UniformBuffer;

struct TextureBinding
{
    uint32_t Slot;
    uint32_t Sampler;
};

class ShaderStorage
{
private:
    RuntimeStorage*             m_storage;

    uint32_t                    m_userUBOSlot;
    UniformBuffer*              m_userUniformBuffer;    

    std::vector<TextureBinding> m_texBindings;

protected:

public:
    ShaderStorage(RuntimeStorage* a_runtimeStorage);
    ~ShaderStorage();

    void Bind();

    void SetTexture(uint32_t a_slot, uint32_t a_sampler);
    void SetUserUBO(uint32_t a_slot, const void* a_object, uint32_t a_size);
};