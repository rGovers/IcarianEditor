#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEditor {
#endif

IOP_CSINTERNAL enum IOP_ENUM_NAME(FileTextureMode) : IOP_UINT16
{
    IOP_ENUM_VALUE(FileTextureMode, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(FileTextureMode, Texture) = 0,
};

IOP_PACKED IOP_CSINTERNAL struct FileTextureHandle
{
    IOP_CSPUBLIC IOP_UINT32 Addr;
    IOP_CSPUBLIC IOP_ENUM_NAME(FileTextureMode) Mode;
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif