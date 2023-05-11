#include "VertexShader.h"

#include "Logger.h"

VertexShader::VertexShader()
{
    
}
VertexShader::~VertexShader()
{
    glDeleteShader(m_handle);
}

VertexShader* VertexShader::GenerateShader(const std::string_view& a_str)
{
    GLuint handle = glCreateShader(GL_VERTEX_SHADER);

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

    VertexShader* s = new VertexShader();
    s->m_handle = handle;

    return s;
}