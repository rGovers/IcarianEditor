// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#define WORKSPACE_EXPORT_TABLE(F) \
    F(IOP_STRING, IcarianEditor, WorkspaceInterop, GetCurrentScene, { return mono_string_new_wrapper(Instance->GetCurrentScene().string().c_str()); }) \
    F(void, IcarianEditor, WorkspaceInterop, SetCurrentScene, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); Instance->SetCurrentScene(str); }, IOP_STRING a_path) \
    \
    F(IOP_UINT32, IcarianEditor, WorkspaceInterop, GetManipulationMode, { return (uint32_t)Instance->GetManipulationMode(); })

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