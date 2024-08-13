// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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
                Texture* tex = Texture::CreateRGBA((uint32_t)width, (uint32_t)height, data);

                Instance->m_textures.emplace(pStr, tex);

                stbi_image_free((void*)data);

                return tex;
            }
        }
    }   

    return nullptr;
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