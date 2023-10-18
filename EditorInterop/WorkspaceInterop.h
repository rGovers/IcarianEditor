#pragma once

#include "InteropTypes.h"

#define WORKSPACE_EXPORT_TABLE(F) \
    F(IOP_STRING, IcarianEditor, WorkspaceInterop, GetCurrentScene, { return mono_string_new_wrapper(Instance->GetCurrentScene().string().c_str()); }) \
    F(void, IcarianEditor, WorkspaceInterop, SetCurrentScene, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); Instance->SetCurrentScene(str); }, IOP_STRING a_path) \
    \
    F(IOP_UINT32, IcarianEditor, WorkspaceInterop, GetManipulationMode, { return (uint32_t)Instance->GetManipulationMode(); })
