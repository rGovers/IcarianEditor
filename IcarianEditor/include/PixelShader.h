#pragma once

#include <string_view>

#include "Shader.h"

class PixelShader : public Shader
{
private:    
    PixelShader(GLuint a_handle, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);
    
protected:

public:
    ~PixelShader();

    static PixelShader* GenerateShader(const std::string_view& a_str, const ShaderBufferInput* a_inputs = nullptr, uint32_t a_inputCount = 0);
};