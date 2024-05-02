#pragma once

#include <glad/glad.h>

#include "EngineMaterialInteropStructures.h"

class Shader
{
private:
    GLuint             m_handle;
    
    uint32_t           m_inputCount;
    ShaderBufferInput* m_inputs;

protected:
    Shader(GLuint a_handle, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);

public:
    virtual ~Shader();

    inline uint32_t GetInputCount() const
    {
        return m_inputCount;
    }
    inline ShaderBufferInput GetInput(uint32_t a_index) const
    {
        return m_inputs[a_index];
    }

    inline GLuint GetHandle() const
    {
        return m_handle;
    }
};