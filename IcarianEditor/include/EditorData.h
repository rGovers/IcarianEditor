// Icarian Editor - Editor for the Icarian Game Engine
//
// License at end of file.

#pragma once

#include <filesystem>

class EditorData
{
private:
    static constexpr uint32_t MaxProjectCount = 32;

    // Add one so we do not have to deal with overflow logic when shifting values up to add another value
    std::string m_lastProjectPath[MaxProjectCount + 1];
    uint32_t    m_lastProjectCount;

    EditorData();
protected:

public:
    ~EditorData();

    static void Init();
    static void Destroy();

    static void Serialize();

    static void AddLastProjectPath(const std::filesystem::path& a_path);

    static uint32_t GetLastProjectCount();
    static const std::string* GetLastProjectPaths();
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
