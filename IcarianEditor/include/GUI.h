#pragma once

#include <cstdint>
#include <list>
#include <string>
#include <vector>

class AppMain;
class AssetLibrary;
class RuntimeManager;

struct PathString
{
    std::string IDStr;
    std::string Path;
};

class GUI
{
private:
    AppMain*                m_app;
    AssetLibrary*           m_assets;
    RuntimeManager*         m_runtime;
    
    std::list<std::string>  m_id;
    std::vector<PathString> m_pathStrings;

    float                   m_width;

    GUI(AppMain* a_app, RuntimeManager* a_runtime, AssetLibrary* a_assets);

protected:

public:
    GUI() = delete;
    ~GUI();

    inline RuntimeManager* GetRuntime() const
    {
        return m_runtime;
    }
    std::string GetID() const;

    inline float GetWidth() const
    {
        return m_width;
    }
    inline float GetTextWidth() const
    {
        return m_width * 0.3f;
    }
    inline float GetFieldWidth() const
    {
        return m_width * 0.6f;
    }

    static void SetWidth(float a_width);

    inline void PushID(const std::string_view& a_str)
    {
        m_id.push_front(std::string(a_str));
    }
    inline void PopID()
    {
        m_id.pop_front();
    }

    void OpenAssetPathModal(char* const* a_extensions, uint32_t a_extensionCount);
    inline void PushPathString(const PathString& a_string)
    {
        m_pathStrings.emplace_back(a_string);
    }
    std::string GetPathString();

    static void Init(AppMain* a_app, RuntimeManager* a_runtime, AssetLibrary* a_assets);
    static void Destroy();
};
