#ifndef INCLUDED_HEADER_BUILDICARIANEDITORCS
#define INCLUDED_HEADER_BUILDICARIANEDITORCS

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

CUBE_CSProject BuildIcarianEditorCSProject(CBBOOL a_optimise)
{
    CUBE_CSProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianEditorCS");
    project.Target = CUBE_CSProjectTarget_Library;
    project.OutputPath = CUBE_Path_CreateC("./build/");
    project.Optimise = a_optimise;

    CUBE_CSProject_AppendSource(&project, "src/AnimationMaster.cs");
    CUBE_CSProject_AppendSource(&project, "src/AssetProperties.cs");
    CUBE_CSProject_AppendSource(&project, "src/ColliderRenderer.cs");
    CUBE_CSProject_AppendSource(&project, "src/EditorConfig.cs");
    CUBE_CSProject_AppendSource(&project, "src/Gizmos.cs");
    CUBE_CSProject_AppendSource(&project, "src/GUI.cs");
    CUBE_CSProject_AppendSource(&project, "src/Program.cs");
    CUBE_CSProject_AppendSource(&project, "src/SceneData.cs");
    CUBE_CSProject_AppendSource(&project, "src/Workspace.cs");

    CUBE_CSProject_AppendSource(&project, "src/Editor/EditorDisplay.cs");
    CUBE_CSProject_AppendSource(&project, "src/Editor/MeshRendererEditorDisplay.cs");
    CUBE_CSProject_AppendSource(&project, "src/Editor/PhysicsBodyEditorDisplay.cs");
    CUBE_CSProject_AppendSource(&project, "src/Editor/RigidBodyEditorDisplay.cs");
    CUBE_CSProject_AppendSource(&project, "src/Editor/SkeletonAnimatorEditorDisplay.cs");
    CUBE_CSProject_AppendSource(&project, "src/Editor/SkinnedMeshRendererEditorDisplay.cs");
    CUBE_CSProject_AppendSource(&project, "src/Editor/TriggerBodyEditorDisplay.cs");

    CUBE_CSProject_AppendSource(&project, "src/Modals/AddComponentModal.cs");
    CUBE_CSProject_AppendSource(&project, "src/Modals/ConfirmModal.cs");
    CUBE_CSProject_AppendSource(&project, "src/Modals/CreateSceneDefModal.cs");
    CUBE_CSProject_AppendSource(&project, "src/Modals/DeleteSceneObjectModal.cs");
    CUBE_CSProject_AppendSource(&project, "src/Modals/GUIGetDefSelectorModal.cs");
    CUBE_CSProject_AppendSource(&project, "src/Modals/Modal.cs");
    CUBE_CSProject_AppendSource(&project, "src/Modals/NewSceneObjectModal.cs");

    CUBE_CSProject_AppendSource(&project, "src/Properties/CameraPropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/DirectionalLightPropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/GameObjectPropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/LightPropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/MaterialPropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/PointLightPropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/PropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/SelectionObjectPropertiesWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Properties/SpotLightPropertiesWindow.cs");

    CUBE_CSProject_AppendSource(&project, "src/Windows/EditorWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Windows/HierarchyWindow.cs");
    CUBE_CSProject_AppendSource(&project, "src/Windows/SceneDefsWindow.cs");

    CUBE_CSProject_AppendReference(&project, "../IcarianEngine/IcarianCS/build/IcarianCS.dll");

    return project;
}

#ifdef __cplusplus
}  
#endif

#endif 