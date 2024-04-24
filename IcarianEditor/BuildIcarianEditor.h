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

static CBBOOL WriteIcarianEditorShadersToHeader(const char* a_workingPath)
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

static CBBOOL WriteTemplateToHeader(const char* a_workingPath)
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

static CUBE_CProject BuildIcarianEditorProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
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

    CUBE_StackString commitHash = CUBE_Git_GetCommitHashShort();

    CUBE_String commitDefine = CUBE_String_CreateC("ICARIANEDITOR_COMMIT_HASH=");
    CUBE_String_AppendSS(&commitDefine, &commitHash);

    CUBE_CProject_AppendDefines(&project,
        "ICARIANEDITOR_VERSION_MAJOR=2024",
        "ICARIANEDITOR_VERSION_MINOR=0",
        "ICARIANEDITOR_VERSION_PATCH=0",
        commitDefine.Data,
        "ICARIANEDITOR_VERSION_TAG=DEV",

        // "GLM_FORCE_QUAT_DATA_XYZW",
        "GLM_FORCE_RADIANS",
        "KHRONOS_STATIC",
        "LIBKTX",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2"
    );

    CUBE_String_Destroy(&commitDefine);

    CUBE_CProject_AppendIncludePaths(&project, 
        "include",
        "../EditorInterop",
        "../IcarianEngine/EngineInterop",
        "../IcarianEngine/IcarianCore/include",
        "../IcarianEngine/deps/CUBE/include",
        "../IcarianEngine/deps/flare-glfw/include",
        "../IcarianEngine/deps/flare-glm",
        "../IcarianEngine/deps/flare-stb",
        "../IcarianEngine/deps/KTX-Software/include",
        "../IcarianEngine/deps/flare-tinyxml2",
        "lib/glad/include",
        "lib/imgui",
        "lib/imgui/backends",
        "lib/flare-ImGuizmo",
        "lib/implot"
    );

    CUBE_CProject_AppendSources(&project, 
        "../IcarianEngine/deps/flare-tinyxml2/tinyxml2.cpp",
        "lib/glad/src/glad.c",
        "lib/imgui/imgui.cpp",
        "lib/imgui/imgui_draw.cpp",
        "lib/imgui/imgui_tables.cpp",
        "lib/imgui/imgui_widgets.cpp",
        "lib/imgui/backends/imgui_impl_glfw.cpp",
        "lib/imgui/backends/imgui_impl_opengl3.cpp",
        "lib/flare-ImGuizmo/ImGuizmo.cpp",
        "lib/implot/implot.cpp",
        "lib/implot/implot_items.cpp",

        "src/Application.cpp",
        "src/AppMain.cpp",
        "src/AssetBrowserWindow.cpp",
        "src/AssetLibrary.cpp",
        "src/BuildLoadingTask.cpp",
        "src/BuildProjectModal.cpp",
        "src/ConfirmModal.cpp",
        "src/ConsoleWindow.cpp",
        "src/CopyBuildLibraryLoadingTask.cpp",
        "src/CreateAssemblyControlModal.cpp",
        "src/CreateDefTableModal.cpp",
        "src/CreateFileModal.cpp",
        "src/CreateProjectModal.cpp",
        "src/CreateRigidBodyScriptModal.cpp",
        "src/CreateScriptableModal.cpp",
        "src/CreateTriggerBodyScriptModal.cpp",
        "src/Datastore.cpp",
        "src/EditorConfig.cpp",
        "src/EditorConfigModal.cpp",
        "src/EditorWindow.cpp",
        "src/ErrorModal.cpp",
        "src/FileDialog.cpp",
        "src/FileDialogBlock.cpp",
        "src/FileHandler.cpp",
        "src/FlareImGui.cpp",
        "src/GameWindow.cpp",
        "src/GenerateConfigLoadingTask.cpp",
        "src/Gizmos.cpp",
        "src/GUI.cpp",
        "src/HierarchyWindow.cpp",
        "src/IO.cpp",
        "src/LoadingModal.cpp",
        "src/Logger.cpp",
        "src/main.cpp",
        "src/Modal.cpp",
        "src/Model.cpp",
        "src/MonoProjectGenerator.cpp",
        "src/OpenProjectModal.cpp",
        "src/PixelShader.cpp",
        "src/ProcessManager.cpp",
        "src/ProfilerData.cpp",
        "src/ProfilerWindow.cpp",
        "src/Project.cpp",
        "src/PropertiesWindow.cpp",
        "src/RenamePathModal.cpp",
        "src/RenderCommand.cpp",
        "src/RuntimeManager.cpp",
        "src/RuntimeModal.cpp",
        "src/RuntimeStorage.cpp",
        "src/SceneDefsWindow.cpp",
        "src/SerializeAssetsLoadingTask.cpp",
        "src/ShaderGenerator.cpp",
        "src/ShaderProgram.cpp",
        "src/ShaderStorage.cpp",
        "src/ShaderStorageObject.cpp",
        "src/TemplateBuilder.cpp",
        "src/Texture.cpp",
        "src/TimelineWindow.cpp",
        "src/UniformBuffer.cpp",
        "src/VertexShader.cpp",
        "src/Window.cpp",
        "src/Workspace.cpp"
    );

    // Used for style editor window
    // CUBE_CProject_AppendSource(&project, "lib/imgui/imgui_demo.cpp");

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
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-march=x86-64-v2");

        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-march=x86-64-v2");

        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );

        CUBE_CProject_AppendSystemIncludePath(&project, "../IcarianEngine/deps/Mono/Windows/include");

        CUBE_CProject_AppendLibraries(&project,
            "../IcarianEngine/IcarianCore/build/IcarianCore.lib",

            "../IcarianEngine/deps/flare-glfw/build/GLFW.lib",
            "../IcarianEngine/deps/miniz/build/miniz.lib",
            "../IcarianEngine/deps/KTX-Software/build/ktxc.lib",
            "../IcarianEngine/deps/KTX-Software/build/ktxcpp.lib",
            "../IcarianEngine/deps/KTX-Software/build/ktxglc.lib",
            "../IcarianEngine/deps/KTX-Software/build/ktxglcpp.lib",
            "../IcarianEngine/deps/Mono/Windows/lib/mono-2.0-sgen.lib",
            "../IcarianEngine/deps/Mono/Windows/lib/MonoPosixHelper.lib",
            "../IcarianEngine/deps/OpenFBX/build/OpenFBXLibDeflate.lib"
        );

        CUBE_CProject_AppendReference(&project, "gdi32");
        CUBE_CProject_AppendReference(&project, "wsock32");
        CUBE_CProject_AppendReference(&project, "ws2_32");

        // Magic string to get std library to link with MinGW
        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_CProject_AppendSystemIncludePath(&project, "../IcarianEngine/deps/Mono/Linux/include/mono-2.0");

        CUBE_CProject_AppendLibraries(&project, 
            "../IcarianEngine/IcarianCore/build/libIcarianCore.a",

            "../IcarianEngine/deps/flare-glfw/build/libGLFW.a",
            "../IcarianEngine/deps/miniz/build/libminiz.a",
            "../IcarianEngine/deps/KTX-Software/build/libktxc.a",
            "../IcarianEngine/deps/KTX-Software/build/libktxcpp.a",
            "../IcarianEngine/deps/KTX-Software/build/libktxglc.a",
            "../IcarianEngine/deps/KTX-Software/build/libktxglcpp.a",
            "../IcarianEngine/deps/Mono/Linux/lib/libmonosgen-2.0.a",
            "../IcarianEngine/deps/OpenFBX/build/libOpenFBXLibDeflate.a"
        );

        // Seems KTX fails loading GL functions with Nvidia GPUs so need to link manually
        // Only fixes on some distros seem to be waiting on a proper fix
        // Using clang also seems to break it for some reason
        // Known issue and am waiting on a fix
        // TODO: Remove when fixed
        CUBE_CProject_AppendReference(&project, "GL");
        CUBE_CProject_AppendReference(&project, "m");
        CUBE_CProject_AppendReference(&project, "stdc++");
        CUBE_CProject_AppendReference(&project, "z");

        break;
    }
    }

    return project;
}

#ifdef __cplusplus
}
#endif

#endif