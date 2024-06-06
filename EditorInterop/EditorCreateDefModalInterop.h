#pragma once 

#include "InteropTypes.h"

#define EDITOR_CREATEDEFMODAL_EXPORT_TABLE(F) \
    F(void, IcarianEditor, CreateDefModalInterop, CreateDef, \
    { \
        char* str = mono_string_to_utf8(a_path); \
        IDEFER(mono_free(str)); \
        const std::filesystem::path p = std::filesystem::path(str); \
        const uintptr_t len = mono_array_length(a_data); \
        uint8_t* data = new uint8_t[len]; \
        for (uintptr_t i = 0; i < len; ++i) \
        { \
            data[i] = mono_array_get(a_data, uint8_t, i); \
        } \
        Instance->CreateDef(p, (uint32_t)len, data); \
    }, IOP_STRING a_path, IOP_ARRAY(byte[]) a_data) \

