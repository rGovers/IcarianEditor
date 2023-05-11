#pragma once

#include <glad/glad.h>

class VertexShader;
class PixelShader;

class ShaderProgram
{
private:
    GLuint m_handle;

    ShaderProgram();

protected:

public:
    ~ShaderProgram();

    inline GLuint GetHandle() const
    {
        return m_handle;
    }

    static ShaderProgram* GenerateProgram(const VertexShader* a_vertexShader, const PixelShader* a_pixelShader);
};