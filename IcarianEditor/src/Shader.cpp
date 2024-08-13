// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Shader.h"

Shader::Shader(GLuint a_handle, const ShaderBufferInput* a_inputs, uint32_t a_inputCount)
{
    m_handle = a_handle;

    m_inputCount = a_inputCount;
    if (m_inputCount > 0)
    {
        m_inputs = new ShaderBufferInput[m_inputCount];
        for (uint32_t i = 0; i < m_inputCount; ++i)
        {
            m_inputs[i] = a_inputs[i];
        }
    }
    else
    {
        m_inputs = nullptr;
    }
}
Shader::~Shader()
{
    glDeleteShader(m_handle);

    if (m_inputs != nullptr)
    {
        delete[] m_inputs;
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