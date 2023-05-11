#pragma once

#include <glad/glad.h>
#include <string>

class VertexShader
{
private:
    GLuint m_handle;
    
    VertexShader();
    
protected:

public:
    ~VertexShader();

    static VertexShader* GenerateShader(const std::string_view& a_str);

    inline GLuint GetHandle() const
    {
        return m_handle;
    }
};