// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "Windows/Window.h"

#include <glad/glad.h>

class EngineProcess;
class ShaderProgram;

#include "EditorGizmosInteropStructures.h"
#include "EditorLightInteropStructures.h"

class EditorWindow : public Window
{
private:
    static constexpr float TrayOffset = 55.0f;

    EngineProcess*     m_process;

    ShaderProgram*     m_compositeProgram;

    glm::quat          m_rotation;
    glm::vec3          m_translation;
    glm::vec2          m_prevMousePos;

    float              m_moveSpeed;
    float              m_zoom;
    double             m_lastUpdate;

    uint32_t           m_width;
    uint32_t           m_height;

    GLuint             m_gizmosRenderTexture;
    GLuint             m_renderTexture;

    GLuint             m_gizmosFramebuffer;
    GLuint             m_renderFramebuffer;

    e_EditorLightMode  m_lightMode;
    e_ManipulationMode m_manipulationMode;

    void UpdateProcess();
    void BuildFrame();

    void TransformToolbar();
    void LightModeToolbar();

protected:

public:
    EditorWindow();
    virtual ~EditorWindow();

    virtual void Refresh();
    virtual void Update(double a_delta);
};

// MIT License
// 
// Copyright (c) 2025 River Govers
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