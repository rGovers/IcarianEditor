#ifndef INCLUDED_HEADER_BUILDICARIANEDITOR
#define INCLUDED_HEADER_BUILDICARIANEDITOR

#include "CUBE/CUBE.h"

#include "../IcarianEngine/BuildBase.h"

#ifdef __cplusplus
extern "C" {
#endif

const static char* IcarianEditorShaderBasePaths[] = 
{
    "shaders/Gizmo.frag",
    "shaders/Gizmo.vert",
    "shaders/Grid.frag",
    "shaders/Grid.vert"
};

const static CBUINT32 IcarianEditorShaderBasePathCount = sizeof(IcarianEditorShaderBasePaths) / sizeof(*IcarianEditorShaderBasePaths);

const static char* TemplateBasePaths[] = 
{
    "templates/About.xml",
    "templates/AssemblyControl.cs",
    "templates/DefTable.cs",
    "templates/RigidBody.cs",
    "templates/Scene.iscene",
    "templates/Scriptable.cs",
    "templates/TriggerBody.cs"
};

const static CBUINT32 TemplateBasePathCount = sizeof(TemplateBasePaths) / sizeof(*TemplateBasePaths);

CBBOOL WriteIcarianEditorShadersToHeader(const char* a_workingPath)
{
    CUBE_Path workingPath = CUBE_Path_CreateC(a_workingPath);

    CUBE_Path shaderPaths[IcarianEditorShaderBasePathCount];

    for (CBUINT32 i = 0; i < IcarianEditorShaderBasePathCount; ++i)
    {
        shaderPaths[i] = CUBE_Path_CombineC(&workingPath, IcarianEditorShaderBasePaths[i]);
    }

    CUBE_Path outPath = CUBE_Path_CombineC(&workingPath, "include/Shaders.h");
    CUBE_String outPathStr = CUBE_Path_ToString(&outPath);

    const CBBOOL ret = ShadersToHeader(shaderPaths, IcarianEditorShaderBasePathCount, outPathStr.Data);

    CUBE_String_Destroy(&outPathStr);
    CUBE_Path_Destroy(&outPath);

    for (CBUINT32 i = 0; i < IcarianEditorShaderBasePathCount; ++i)
    {
        CUBE_Path_Destroy(&shaderPaths[i]);
    }

    CUBE_Path_Destroy(&workingPath);

    return ret;
}

CBBOOL WriteTemplateToHeader(const char* a_workingPath)
{
    CUBE_Path workingPath = CUBE_Path_CreateC(a_workingPath);

    CUBE_Path templatePaths[TemplateBasePathCount];

    for (CBUINT32 i = 0; i < TemplateBasePathCount; ++i)
    {
        templatePaths[i] = CUBE_Path_CombineC(&workingPath, TemplateBasePaths[i]);
    }

    CUBE_Path outPath = CUBE_Path_CombineC(&workingPath, "include/Templates.h");
    CUBE_String outPathStr = CUBE_Path_ToString(&outPath);

    const CBBOOL ret = TemplatesToHeader(templatePaths, TemplateBasePathCount, outPathStr.Data);

    CUBE_String_Destroy(&outPathStr);
    CUBE_Path_Destroy(&outPath);

    for (CBUINT32 i = 0; i < TemplateBasePathCount; ++i)
    {
        CUBE_Path_Destroy(&templatePaths[i]);
    }

    CUBE_Path_Destroy(&workingPath);

    return ret;
}

CUBE_CProject BuildIcarianEditorProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianEditor");
    project.Target = CUBE_CProjectTarget_Exe;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendDefine(&project, "ICARIANEDITOR_VERSION_MAJOR=0");
    CUBE_CProject_AppendDefine(&project, "ICARIANEDITOR_VERSION_MINOR=1");

    CUBE_CProject_AppendIncludePath(&project, "include");
    CUBE_CProject_AppendIncludePath(&project, "../EditorInterop");
    CUBE_CProject_AppendIncludePath(&project, "../IcarianEngine/EngineInterop");
    CUBE_CProject_AppendIncludePath(&project, "../IcarianEngine/FlareBase/include");
    CUBE_CProject_AppendIncludePath(&project, "../IcarianEngine/deps/CUBE/include");
    CUBE_CProject_AppendIncludePath(&project, "../IcarianEngine/deps/flare-glfw/include");
    CUBE_CProject_AppendIncludePath(&project, "../IcarianEngine/deps/flare-glm");
    CUBE_CProject_AppendIncludePath(&project, "../IcarianEngine/deps/flare-stb");
    CUBE_CProject_AppendIncludePath(&project, "../IcarianEngine/deps/flare-tinyxml2");
    CUBE_CProject_AppendIncludePath(&project, "lib/glad/include");
    CUBE_CProject_AppendIncludePath(&project, "lib/imgui");
    CUBE_CProject_AppendIncludePath(&project, "lib/imgui/backends");
    CUBE_CProject_AppendIncludePath(&project, "lib/flare-ImGuizmo");
    CUBE_CProject_AppendIncludePath(&project, "lib/implot");

    CUBE_CProject_AppendSource(&project, "../IcarianEngine/deps/flare-tinyxml2/tinyxml2.cpp");
    CUBE_CProject_AppendSource(&project, "lib/glad/src/glad.c");
    CUBE_CProject_AppendSource(&project, "lib/imgui/imgui.cpp");
    CUBE_CProject_AppendSource(&project, "lib/imgui/imgui_draw.cpp");
    CUBE_CProject_AppendSource(&project, "lib/imgui/imgui_tables.cpp");
    CUBE_CProject_AppendSource(&project, "lib/imgui/imgui_widgets.cpp");
    CUBE_CProject_AppendSource(&project, "lib/imgui/backends/imgui_impl_glfw.cpp");
    CUBE_CProject_AppendSource(&project, "lib/imgui/backends/imgui_impl_opengl3.cpp");
    CUBE_CProject_AppendSource(&project, "lib/flare-ImGuizmo/ImGuizmo.cpp");
    CUBE_CProject_AppendSource(&project, "lib/implot/implot.cpp");
    CUBE_CProject_AppendSource(&project, "lib/implot/implot_items.cpp");

    // Used for style editor window
    // CUBE_CProject_AppendSource(&project, "lib/imgui/imgui_demo.cpp");
    
    CUBE_CProject_AppendSource(&project, "src/Application.cpp");
    CUBE_CProject_AppendSource(&project, "src/AppMain.cpp");
    CUBE_CProject_AppendSource(&project, "src/AssetBrowserWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/AssetLibrary.cpp");
    CUBE_CProject_AppendSource(&project, "src/BuildLoadingTask.cpp");
    CUBE_CProject_AppendSource(&project, "src/BuildProjectModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/ConfirmModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/ConsoleWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/ControlWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/CopyBuildLibraryLoadingTask.cpp");
    CUBE_CProject_AppendSource(&project, "src/CreateAssemblyControlModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/CreateDefTableModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/CreateFileModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/CreateProjectModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/CreateRigidBodyScriptModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/CreateScriptableModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/CreateTriggerBodyScriptModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/Datastore.cpp");
    CUBE_CProject_AppendSource(&project, "src/EditorConfig.cpp");
    CUBE_CProject_AppendSource(&project, "src/EditorConfigModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/EditorWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/ErrorModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/FileDialog.cpp");
    CUBE_CProject_AppendSource(&project, "src/FileHandler.cpp");
    CUBE_CProject_AppendSource(&project, "src/FlareImGui.cpp");
    CUBE_CProject_AppendSource(&project, "src/GameWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/GenerateConfigLoadingTask.cpp");
    CUBE_CProject_AppendSource(&project, "src/Gizmos.cpp");
    CUBE_CProject_AppendSource(&project, "src/GUI.cpp");
    CUBE_CProject_AppendSource(&project, "src/HierarchyWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/IO.cpp");
    CUBE_CProject_AppendSource(&project, "src/LoadingModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/Logger.cpp");
    CUBE_CProject_AppendSource(&project, "src/main.cpp");
    CUBE_CProject_AppendSource(&project, "src/Modal.cpp");
    CUBE_CProject_AppendSource(&project, "src/Model.cpp");
    CUBE_CProject_AppendSource(&project, "src/MonoProjectGenerator.cpp");
    CUBE_CProject_AppendSource(&project, "src/OpenProjectModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/PixelShader.cpp");
    CUBE_CProject_AppendSource(&project, "src/ProcessManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/ProfilerData.cpp");
    CUBE_CProject_AppendSource(&project, "src/ProfilerWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/Project.cpp");
    CUBE_CProject_AppendSource(&project, "src/PropertiesWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/RenamePathModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/RenderCommand.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeModal.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeStorage.cpp");
    CUBE_CProject_AppendSource(&project, "src/SceneDefsWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/SerializeAssetsLoadingTask.cpp");
    CUBE_CProject_AppendSource(&project, "src/ShaderGenerator.cpp");
    CUBE_CProject_AppendSource(&project, "src/ShaderProgram.cpp");
    CUBE_CProject_AppendSource(&project, "src/ShaderStorage.cpp");
    CUBE_CProject_AppendSource(&project, "src/ShaderStorageObject.cpp");
    CUBE_CProject_AppendSource(&project, "src/TemplateBuilder.cpp");
    CUBE_CProject_AppendSource(&project, "src/Texture.cpp");
    CUBE_CProject_AppendSource(&project, "src/UniformBuffer.cpp");
    CUBE_CProject_AppendSource(&project, "src/VertexShader.cpp");
    CUBE_CProject_AppendSource(&project, "src/Window.cpp");
    CUBE_CProject_AppendSource(&project, "src/Workspace.cpp");

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendDefine(&project, "WIN32");

        CUBE_CProject_AppendSystemIncludePath(&project, "../IcarianEngine/deps/Mono/Windows/include");

        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/FlareBase/build/FlareBase.lib");

        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/flare-glfw/build/GLFW.lib");
        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/miniz/build/miniz.lib");
        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/Mono/Windows/lib/mono-2.0-sgen.lib");
        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/Mono/Windows/lib/MonoPosixHelper.lib");
        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/OpenFBX/build/OpenFBXLibDeflate.lib");

        CUBE_CProject_AppendReference(&project, "gdi32");
        CUBE_CProject_AppendReference(&project, "wsock32");
        CUBE_CProject_AppendReference(&project, "ws2_32");

        // Magic string to get std library to link with MinGW
        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        break;
    }
    case TargetPlatform_Linux:
    {
        CUBE_CProject_AppendSystemIncludePath(&project, "../IcarianEngine/deps/Mono/Linux/include/mono-2.0");

        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/FlareBase/build/libFlareBase.a");

        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/flare-glfw/build/libGLFW.a");
        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/miniz/build/libminiz.a");
        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/Mono/Linux/lib/libmonosgen-2.0.a");
        CUBE_CProject_AppendLibrary(&project, "../IcarianEngine/deps/OpenFBX/build/libOpenFBXLibDeflate.a");

        CUBE_CProject_AppendReference(&project, "z");

        CUBE_CProject_AppendReference(&project, "stdc++");
        CUBE_CProject_AppendReference(&project, "m");

        break;
    }
    }

    return project;
}

#ifdef __cplusplus
}
#endif

#endif