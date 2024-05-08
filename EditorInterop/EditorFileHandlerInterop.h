#pragma once 

#include "InteropTypes.h"

#define FILEHANDLER_EXPORT_TABLE(F) \
    F(void, IcarianEditor, FileHandlerInterop, SetFileHandle, \
    { \
        FileTextureHandle handle; \
        handle.Addr = a_addr; \
        handle.Mode = (e_FileTextureMode)a_mode; \
        Instance->SetFileHandle(handle); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_mode) \

