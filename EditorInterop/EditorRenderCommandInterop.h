// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#define EDITOR_RENDERCOMMAND_EXPORT_TABLE(F) \
    F(void, IcarianEditor, EditorRenderCommandInterop, PushAmbientLight, \
    { \
        RenderCommand::PushAmbientLight(a_intensity, a_color); \
    }, float a_intensity, IOP_VEC4 a_color) \
    F(void, IcarianEditor, EditorRenderCommandInterop, PushDirectionalLight, \
    { \
        RenderCommand::PushDirectionalLight(a_transform, a_intensity, a_color); \
    }, IOP_MAT4 a_transform, float a_intensity, IOP_VEC4 a_color) \
    F(void, IcarianEditor, EditorRenderCommandInterop, PushPointLight, \
    { \
        RenderCommand::PushPointLight(a_transform, a_intensity, a_radius, a_color); \
    }, IOP_MAT4 a_transform, float a_intensity, float a_radius, IOP_VEC4 a_color) \
    F(void, IcarianEditor, EditorRenderCommandInterop, PushSpotLight, \
    { \
        RenderCommand::PushSpotLight(a_transform, a_intensity, a_radius, a_innerCutoffAngle, a_outerCutoffAngle, a_color); \
    }, IOP_MAT4 a_transform, float a_intensity, float a_radius, float a_innerCutoffAngle, float a_outerCutoffAngle, IOP_VEC4 a_color) \

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