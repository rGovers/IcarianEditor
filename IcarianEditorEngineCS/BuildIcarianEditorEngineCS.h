#ifndef INCLUDED_HEADER_BUILDICARIANEDITORENGINECS
#define INCLUDED_HEADER_BUILDICARIANEDITORENGINECS

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

static CUBE_CSProject BuildIcarianEditorEngineCSProject(CBBOOL a_optimise)
{
    CUBE_CSProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianEditorEngineCS");
    project.Target = CUBE_CSProjectTarget_Library;
    project.OutputPath = CUBE_Path_CreateC("./build/");
    project.Optimise = a_optimise;

    CUBE_CSProject_AppendIncludePath(&project, "../IcarianEngine/EngineInterop");
    CUBE_CSProject_AppendIncludePath(&project, "../EditorInterop");

    CUBE_CSProject_AppendSources(&project, 
        "./src/AssetStore.cs",
        "./src/CameraController.cs",
        "./src/IcarianEditorAssemblyControl.cs",
        "./src/SceneView.cs"
    );

    CUBE_CSProject_AppendReference(&project, "System.Core.dll");
    CUBE_CSProject_AppendReference(&project, "System.Xml.dll");
    CUBE_CSProject_AppendReference(&project, "../IcarianEngine/IcarianCS/build/IcarianCS.dll");

    return project;
}

#ifdef __cplusplus
}  
#endif

#endif 