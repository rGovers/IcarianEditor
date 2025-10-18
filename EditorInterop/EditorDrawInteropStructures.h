// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEditor.Engine {
#endif

IOP_PACKED IOP_CSPUBLIC struct DrawDataHeader
{
    IOP_CSPUBLIC IOP_UINT32 Version;
    IOP_CSPUBLIC IOP_UINT32 HeaderOffset;
    IOP_CSPUBLIC IOP_UINT32 HeaderCount;
};

IOP_PACKED IOP_CSPUBLIC struct DrawMeshBufferHeader
{
    IOP_CSPUBLIC IOP_UINT32 MaterialID;
    IOP_CSPUBLIC IOP_UINT32 MeshID;
    IOP_CSPUBLIC IOP_UINT32 IndexCount;
    IOP_CSPUBLIC IOP_UINT32 TransformOffset;
    IOP_CSPUBLIC IOP_UINT32 TransformCount;
};

IOP_PACKED IOP_CSPUBLIC struct DrawModelBufferHeader
{
    IOP_CSPUBLIC IOP_UINT32 MaterialID;
    IOP_CSPUBLIC IOP_UINT32 ModelID;
    IOP_CSPUBLIC IOP_UINT32 TransformOffset;
    IOP_CSPUBLIC IOP_UINT32 TransformCount;
};

#ifdef CUBE_LANGUAGE_CSHARP
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