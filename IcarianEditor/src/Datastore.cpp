#include "Datastore.h"

#include <filesystem>
#include <stb_image.h>

#include "Texture.h"

Datastore* Datastore::Instance = nullptr;

Datastore::Datastore()
{
    
}
Datastore::~Datastore()
{
    for (auto iter = m_textures.begin(); iter != m_textures.end(); ++iter)
    {
        if (iter->second != nullptr)
        {
            delete iter->second;
        }
    }
}

void Datastore::Init()
{
    if (Instance == nullptr)
    {
        Instance = new Datastore();
    }
}
void Datastore::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

Texture* Datastore::GetTexture(const std::string_view& a_path)
{
    const std::string pStr = std::string(a_path);

    const auto iter = Instance->m_textures.find(pStr);

    if (iter != Instance->m_textures.end())
    {
        return iter->second;
    }

    const std::filesystem::path p = std::filesystem::path(a_path);
    if (p.has_extension())
    {   
        if (p.extension() == ".png")
        {
            int width;
            int height;

            int comp;

            const unsigned char* data = stbi_load(a_path.data(), &width, &height, &comp, STBI_rgb_alpha);
            if (data != nullptr)
            {
                Texture* tex = new Texture((uint32_t)width, (uint32_t)height, data);

                Instance->m_textures.emplace(pStr, tex);

                stbi_image_free((void*)data);

                return tex;
            }
        }
    }   

    return nullptr;
}