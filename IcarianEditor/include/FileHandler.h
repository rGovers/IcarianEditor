#pragma once

#include <filesystem>
#include <functional>
#include <unordered_map>

class Texture;
class Workspace;

class FileHandler
{
public:
    using FileCallback = std::function<void(const std::filesystem::path&, uint32_t, const char*)>;

private:
    static FileHandler* Instance;

    std::unordered_map<std::string, Texture*>     m_extTex;
    std::unordered_map<std::string, FileCallback> m_extOpenCallback;
    std::unordered_map<std::string, FileCallback> m_extDragCallback;

    FileHandler(Workspace* a_workspace);
protected:

public:
    ~FileHandler();

    static void Init(Workspace* a_workspace);
    static void Destroy();

    static void GetFileData(const std::filesystem::path& a_path, FileCallback** a_openCallback, FileCallback** a_dragCallback, Texture** a_texture);
};