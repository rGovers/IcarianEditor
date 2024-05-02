#include "PixelShader.h"

#include "Core/IcarianDefer.h"
#include "Logger.h"

PixelShader::PixelShader(GLuint a_handle, const ShaderBufferInput* a_inputs, uint32_t a_inputCount) : Shader(a_handle, a_inputs, a_inputCount)
{

}
PixelShader::~PixelShader()
{
    
}

PixelShader* PixelShader::GenerateShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs, uint32_t a_inputCount)
{
    const GLuint handle = glCreateShader(GL_FRAGMENT_SHADER);

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

    return new PixelShader(handle, a_inputs, a_inputCount);
}