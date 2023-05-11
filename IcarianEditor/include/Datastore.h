#pragma once

#include <string>
#include <unordered_map>

class Texture;

class Datastore
{
private:
    static Datastore* Instance;

    std::unordered_map<std::string, Texture*> m_textures;

    Datastore();
protected:

public:
    ~Datastore();

    static void Init();
    static void Destroy();

    static Texture* GetTexture(const std::string_view& a_path);
};