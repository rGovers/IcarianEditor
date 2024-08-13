// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Window.h"

#include <cstdint>
#include <glad/glad.h>

class RuntimeManager;
class ShaderProgram;
class Workspace;

class EditorWindow : public Window
{
private:
    static uint32_t RefCount;
    static ShaderProgram* GridShader;

    bool            m_refresh;

    RuntimeManager* m_runtime;
    Workspace*      m_workspace;

    glm::vec3       m_translation;
    glm::quat       m_rotation;

    uint32_t        m_width;
    uint32_t        m_height;

    glm::vec2       m_prevMousePos;

    float           m_moveSpeed;
    float           m_zoom;

    GLuint          m_textureHandle;
    GLuint          m_depthTextureHandle;
    GLuint          m_framebufferHandle;

protected:

public:
    EditorWindow(RuntimeManager* a_runtime, Workspace* a_workspace);
    virtual ~EditorWindow();

    void Draw();

    inline void Refresh()
    {
        m_refresh = true;
    }

    virtual void Update(double a_delta);
};

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.