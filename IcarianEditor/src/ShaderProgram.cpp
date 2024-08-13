// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "ShaderProgram.h"

#include "Logger.h"
#include "PixelShader.h"
#include "VertexShader.h"

ShaderProgram::ShaderProgram()
{
    
}
ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_handle);
}

ShaderProgram* ShaderProgram::GenerateProgram(const VertexShader* a_vertexShader, const PixelShader* a_pixelShader)
{
    if (a_vertexShader == nullptr || a_pixelShader == nullptr)
    {
        return nullptr;
    }

    GLuint handle = glCreateProgram();

    glAttachShader(handle, a_vertexShader->GetHandle());
    glAttachShader(handle, a_pixelShader->GetHandle());

    glLinkProgram(handle);

    GLint success;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint len;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);

        char* buff = new char[len];
        glGetProgramInfoLog(handle, (GLsizei)len, NULL, buff);

        Logger::Error(buff);

        delete[] buff;

        return nullptr;
    }

    glDetachShader(handle, a_vertexShader->GetHandle());
    glDetachShader(handle, a_pixelShader->GetHandle());

    ShaderProgram* shader = new ShaderProgram();
    shader->m_handle = handle;

    return shader;
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