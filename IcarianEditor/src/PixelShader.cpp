#include "PixelShader.h"

#include "Logger.h"

PixelShader::PixelShader()
{
    
}
PixelShader::~PixelShader()
{
    glDeleteShader(m_handle);
}

PixelShader* PixelShader::GenerateShader(const std::string_view& a_str)
{
    GLuint handle = glCreateShader(GL_FRAGMENT_SHADER);

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
        glGetShaderInfoLog(handle, (GLsizei)logSize, NULL, buffer);

        Logger::Error(buffer);

        delete[] buffer;

        return nullptr;
    }

    PixelShader* s = new PixelShader();
    s->m_handle = handle;

    return s;
}