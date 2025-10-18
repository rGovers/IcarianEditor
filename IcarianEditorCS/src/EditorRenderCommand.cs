// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System.Runtime.CompilerServices;

#include "InteropBinding.h"
#include "EditorRenderCommandInterop.h"

EDITOR_RENDERCOMMAND_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEditor
{
    public static class EditorRenderCommand
    {
        public static void PushAmbientLight(AmbientLightDef a_def)
        {
            EditorRenderCommandInterop.PushAmbientLight(a_def.Intensity, a_def.Color.ToVector4());
        }
        public static void PushDirectionalLight(Matrix4 a_transform, DirectionalLightDef a_def)
        {
            EditorRenderCommandInterop.PushDirectionalLight(a_transform, a_def.Intensity, a_def.Color.ToVector4());
        }
        public static void PushPointLight(Matrix4 a_transform, PointLightDef a_def)
        {
            EditorRenderCommandInterop.PushPointLight(a_transform, a_def.Intensity, a_def.Radius, a_def.Color.ToVector4());
        }
        public static void PushSpotLight(Matrix4 a_transform, SpotLightDef a_def)
        {
            EditorRenderCommandInterop.PushSpotLight(a_transform, a_def.Intensity, a_def.Radius, a_def.InnerCutoffAngle, a_def.OuterCutoffAngle, a_def.Color.ToVector4());
        }
    }
}

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