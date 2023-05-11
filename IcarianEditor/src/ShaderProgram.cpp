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