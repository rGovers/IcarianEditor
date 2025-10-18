// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEditor.Engine {
#endif

IOP_CSPUBLIC enum IOP_ENUM_NAME(EditorLightMode) : IOP_UINT32
{
    IOP_ENUM_VALUE(EditorLightMode, Ambient) = 0,
    IOP_ENUM_VALUE(EditorLightMode, Viewport) = 1,
    IOP_ENUM_VALUE(EditorLightMode, Scene) = 2,
};

IOP_PACKED IOP_CSPUBLIC struct LightHeader
{
    IOP_CSPUBLIC IOP_UINT32 Version;
    IOP_CSPUBLIC IOP_UINT32 DataCount;
    IOP_CSPUBLIC IOP_UINT32 DataOffset;
};

IOP_PACKED IOP_CSPUBLIC struct AmbientLightData
{
    IOP_CSPUBLIC IOP_VEC4 Color;
    IOP_CSPUBLIC float Intensity;
};

IOP_PACKED IOP_CSPUBLIC struct DirectionalLightData
{
    IOP_CSPUBLIC IOP_MAT4 Transform;
    IOP_CSPUBLIC IOP_VEC4 Color;
    IOP_CSPUBLIC float Intensity;
};

IOP_PACKED IOP_CSPUBLIC struct PointLightData
{
    IOP_CSPUBLIC IOP_MAT4 Transform;
    IOP_CSPUBLIC IOP_VEC4 Color;
    IOP_CSPUBLIC IOP_VEC2 Data;
};

IOP_PACKED IOP_CSPUBLIC struct SpotLightData
{
    IOP_CSPUBLIC IOP_MAT4 Transform;
    IOP_CSPUBLIC IOP_VEC4 Color;
    IOP_CSPUBLIC IOP_VEC4 Data;
};

#ifdef  CUBE_LANGUAGE_CSHARP
}
#endif

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