#pragma once

#include <string_view>

#include "Shader.h"

class VertexShader : public Shader
{
private:
    VertexShader(GLuint a_handle, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);
    
protected:

public:
    virtual ~VertexShader();

    static VertexShader* GenerateShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs = nullptr, uint32_t a_inputCount = 0);
};