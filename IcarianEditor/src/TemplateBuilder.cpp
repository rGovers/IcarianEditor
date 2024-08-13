// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "TemplateBuilder.h"

#include <algorithm>

std::string TemplateBuilder::GenerateFromTemplate(const std::string_view& a_str, const std::string_view& a_projectName, const std::string_view& a_fileName)
{
    std::string str = std::string(a_str);

    constexpr std::string_view ProjectNamePlaceHolder = "@ProjectName@";
    constexpr size_t ProjectNamePlaceHolderLen = ProjectNamePlaceHolder.length();

    const size_t projectNameLen = a_projectName.length();

    size_t pos = str.find(ProjectNamePlaceHolder);
    while (pos != std::string::npos)
    {
        str.replace(pos, ProjectNamePlaceHolderLen, a_projectName);
        pos += projectNameLen;
        pos = str.find(ProjectNamePlaceHolder, pos);
    }

    constexpr std::string_view ScriptNamePlaceHolder = "@ScriptName@";
    constexpr size_t ScriptNamePlaceHolderLen = ScriptNamePlaceHolder.length();

    const size_t scriptNameLen = a_fileName.length();

    pos = str.find(ScriptNamePlaceHolder);
    while (pos != std::string::npos)
    {
        str.replace(pos, ScriptNamePlaceHolderLen, a_fileName);
        pos += ScriptNamePlaceHolderLen;
        pos = str.find(ScriptNamePlaceHolder, pos);
    }

    return str;
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