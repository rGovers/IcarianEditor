// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <list>
#include <string>
#include <vector>

class AppMain;
class AssetLibrary;

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
    
    std::list<std::string>  m_id;
    std::vector<PathString> m_pathStrings;

    float                   m_width;

    GUI(AppMain* a_app, AssetLibrary* a_assets);

protected:

public:
    GUI() = delete;
    ~GUI();

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

    static void Init(AppMain* a_app, AssetLibrary* a_assets);
    static void Destroy();
};

// MIT License
// 
// Copyright (c) 2025 River Govers
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