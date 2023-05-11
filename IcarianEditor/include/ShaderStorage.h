#pragma once

#include <cstdint>
#include <vector>

#include "Texture.h"

class RuntimeStorage;

struct TextureBinding
{
    uint32_t Slot;
    Texture* Tex;
};

class ShaderStorage
{
private:
    RuntimeStorage*             m_storage;

    std::vector<TextureBinding> m_texBindings;

protected:

public:
    ShaderStorage(RuntimeStorage* a_runtimeStorage);
    ~ShaderStorage();

    void Bind();

    void SetTexture(uint32_t a_slot, Texture* a_texture);
};