#ifndef INCLUDED_HEADER_BUILDICARIANEDITORCS
#define INCLUDED_HEADER_BUILDICARIANEDITORCS

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

static CUBE_CSProject BuildIcarianEditorCSProject(CBBOOL a_optimise)
{
    CUBE_CSProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianEditorCS");
    project.Target = CUBE_CSProjectTarget_Library;
    project.OutputPath = CUBE_Path_CreateC("./build/");
    project.Optimise = a_optimise;

    CUBE_CSProject_AppendIncludePath(&project, "../EditorInterop");
    CUBE_CSProject_AppendIncludePath(&project, "../IcarianEngine/EngineInterop");

    CUBE_CSProject_AppendSources(&project, 
        "src/AnimationMaster.cs",
        "src/ClipBoard.cs",
        "src/ColliderRenderer.cs",
        "src/EditorConfig.cs",
        "src/EditorDefLibrary.cs",
        "src/EditorScene.cs",
        "src/FileHandler.cs",
        "src/Gizmos.cs",
        "src/GUI.cs",
        "src/Program.cs",
        "src/Workspace.cs",

        "src/Editor/EditorDisplay.cs",
        "src/Editor/MeshRendererEditorDisplay.cs",
        "src/Editor/PhysicsBodyEditorDisplay.cs",
        "src/Editor/RigidBodyEditorDisplay.cs",
        "src/Editor/SkeletonAnimatorEditorDisplay.cs",
        "src/Editor/SkinnedMeshRendererEditorDisplay.cs",
        "src/Editor/TriggerBodyEditorDisplay.cs",

        "src/Modals/AddComponentModal.cs",
        "src/Modals/CreateDefModal.cs",
        "src/Modals/ConfirmModal.cs",
        "src/Modals/CreateSceneDefModal.cs",
        "src/Modals/DeleteSceneObjectModal.cs",
        "src/Modals/GUIGetDefSelectorModal.cs",
        "src/Modals/Modal.cs",
        "src/Modals/NewSceneObjectModal.cs",

        "src/Properties/CameraPropertiesWindow.cs",
        "src/Properties/GameObjectPropertiesWindow.cs",
        "src/Properties/MaterialPropertiesWindow.cs",
        "src/Properties/PropertiesEditorWindow.cs",
        "src/Properties/SelectionObjectPropertiesWindow.cs",
        
        "src/Windows/EditorWindow.cs",
        "src/Windows/HierarchyWindow.cs",
        "src/Windows/PropertiesWindow.cs",
        "src/Windows/SceneDefsWindow.cs"
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