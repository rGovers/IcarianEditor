// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#define EDITOR_SCENE_EXPORT_TABLE(F) \
    F(void, IcarianEditor, EditorSceneInterop, WriteScene, \
    { \
        mono_unichar4* str = mono_string_to_utf32(a_path); \
        IDEFER(mono_free(str)); \
        const std::filesystem::path p = std::filesystem::path(std::u32string((char32_t*)str)); \
        const uintptr_t len = mono_array_length(a_data); \
        uint8_t* data = new uint8_t[len]; \
        for (uintptr_t i = 0; i < len; ++i) \
        { \
            data[i] = mono_array_get(a_data, uint8_t, i); \
        } \
        Instance->WriteScene(p, (uint32_t)len, data); \
    }, IOP_STRING a_path, IOP_ARRAY(byte[]) a_data) \

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