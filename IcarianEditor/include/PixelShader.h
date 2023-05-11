#pragma once

#include <glad/glad.h>
#include <string>

class PixelShader
{
private:
    GLuint m_handle;
    
    PixelShader();
    
protected:

public:
    ~PixelShader();

    inline GLuint GetHandle() const
    {
        return m_handle;
    }

    static PixelShader* GenerateShader(const std::string_view& a_str);
};