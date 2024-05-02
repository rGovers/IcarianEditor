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