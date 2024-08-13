// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "VertexShader.h"

#include "Core/IcarianDefer.h"
#include "Logger.h"

VertexShader::VertexShader(GLuint a_handle, const ShaderBufferInput* a_inputs, uint32_t a_inputCount) : Shader(a_handle, a_inputs, a_inputCount)
{
    
}
VertexShader::~VertexShader()
{
    
}

VertexShader* VertexShader::GenerateShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs, uint32_t a_inputCount)
{
    const GLuint handle = glCreateShader(GL_VERTEX_SHADER);

    const char* d = a_str.data();
    const GLint len = (GLint)a_str.size();

    glShaderSource(handle, 1, &d, &len);
    glCompileShader(handle);

    GLint success;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint logSize = 0;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize);

        char* buffer = new char[logSize];
        IDEFER(delete[] buffer);
        glGetShaderInfoLog(handle, (GLsizei)logSize, NULL, buffer);

        Logger::Error(buffer);

        return nullptr;
    }

    return new VertexShader(handle, a_inputs, a_inputCount);
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