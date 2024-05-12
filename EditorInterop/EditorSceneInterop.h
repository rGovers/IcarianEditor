#pragma once

#include "InteropTypes.h"

#define EDITORSCENE_EXPORT_TABLE(F) \
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

