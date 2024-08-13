// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "ShaderStorageObject.h"

#include <cstddef>

#include "Core/IcarianDefer.h"

ShaderStorageObject::ShaderStorageObject()
{
    int32_t count = 0;

    m_size = sizeof(int32_t);

    glGenBuffers(1, &m_handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int32_t), &count, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
ShaderStorageObject::~ShaderStorageObject()
{
    glDeleteBuffers(1, &m_handle);
}

void ShaderStorageObject::WriteBuffer(const void* a_data, uint16_t a_stride, uint32_t a_count)
{
    IDEFER(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    const int32_t count = (int32_t)a_count;
    const uint64_t dataSize = (uint64_t)a_stride * a_count;
    const uint64_t bufferSize = 16 + dataSize;

    if (bufferSize > m_size)
    {
        m_size = bufferSize;

        const GLuint oldHandle = m_handle;
        IDEFER(glDeleteBuffers(1, &oldHandle));

        glGenBuffers(1, &m_handle);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)bufferSize, NULL, GL_DYNAMIC_DRAW);
    }
    else
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
    }

    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(count), &count);
    if (a_count > 0)
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, (GLsizeiptr)dataSize, a_data);
    }
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