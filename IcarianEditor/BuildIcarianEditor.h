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
    "templates/Scene.iscene",
    "templates/AssemblyControl.cs",
    "templates/DefTable.cs",
    "templates/EmptyScript.cs",
    "templates/Component.cs",
    "templates/Scriptable.cs",
    "templates/Canvas.ui",
    "templates/VertexShader.fvert",
    "templates/SkinnedVertexShader.fvert",
    "templates/ShadowVertexShader.fvert",
    "templates/PixelShader.fpix",
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
        "ICARIANEDITOR_VERSION_MINOR=2",
        "ICARIANEDITOR_VERSION_PATCH=0",
        commitDefine.Data,
        "ICARIANEDITOR_VERSION_TAG=DEV",
        
        "GLM_FORCE_QUAT_DATA_XYZW",
        "GLM_FORCE_RADIANS",
        "KHRONOS_STATIC",
        "LIBKTX",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2"
    );

    CUBE_String_Destroy(&commitDefine);

    CUBE_CProject_AppendIncludePaths(&project, 
        "./include/",

        "../EditorInterop/",
        "../IcarianEngine/EngineInterop/",

        "../IcarianEngine/IcarianCore/include/",

        "../IcarianEngine/deps/assimp/include/",
        "../IcarianEngine/deps/gen/assimp/",
        "../IcarianEngine/deps/CUBE/include/",
        "../IcarianEngine/deps/glfw/include/",
        "../IcarianEngine/deps/flare-glm/",
        "../IcarianEngine/deps/stb/",
        "../IcarianEngine/deps/KTX-Software/include/",
        "../IcarianEngine/deps/flare-tinyxml2/",
        "../IcarianEngine/deps/imgui/",
        "../IcarianEngine/deps/glad/include/",
        "../IcarianEngine/deps/enet/include",

        "./lib/flare-ImGuizmo/",
        "./lib/IcarianRemoteProtocol/include/",
        "./lib/implot/"
    );

    CUBE_CProject_AppendSources(&project, 
        "../IcarianEngine/deps/flare-tinyxml2/tinyxml2.cpp",
        "../IcarianEngine/deps/imgui/imgui.cpp",
        "../IcarianEngine/deps/imgui/imgui_draw.cpp",
        "../IcarianEngine/deps/imgui/imgui_tables.cpp",
        "../IcarianEngine/deps/imgui/imgui_widgets.cpp",
        "../IcarianEngine/deps/imgui/backends/imgui_impl_glfw.cpp",
        "../IcarianEngine/deps/imgui/backends/imgui_impl_opengl3.cpp",
        "../IcarianEngine/deps/glad/src/glad.c",
        "./lib/flare-ImGuizmo/ImGuizmo.cpp",
        "./lib/implot/implot.cpp",
        "./lib/implot/implot_items.cpp",

        "./src/Application.cpp",
        "./src/AppMain.cpp",
        "./src/AssetBrowserWindow.cpp",
        "./src/AssetLibrary.cpp",
        "./src/BuildLoadingTask.cpp",
        "./src/BuildProjectModal.cpp",
        "./src/ConfirmModal.cpp",
        "./src/ConsoleWindow.cpp",
        "./src/CopyBuildLibraryLoadingTask.cpp",
        "./src/CreateAssemblyControlModal.cpp",
        "./src/CreateComponentModal.cpp",
        "./src/CreateDefTableModal.cpp",
        "./src/CreateEmptyScriptModal.cpp",
        "./src/CreateFileModal.cpp",
        "./src/CreateProjectModal.cpp",
        "./src/CreateScriptableModal.cpp",
        "./src/Datastore.cpp",
        "./src/EditorConfig.cpp",
        "./src/EditorConfigModal.cpp",
        "./src/EditorInputManager.cpp",
        "./src/EditorWindow.cpp",
        "./src/ErrorModal.cpp",
        "./src/FileDialog.cpp",
        "./src/FileDialogBlock.cpp",
        "./src/FileHandler.cpp",
        "./src/FlareImGui.cpp",
        "./src/GameWindow.cpp",
        "./src/GenerateConfigLoadingTask.cpp",
        "./src/GetAssetModal.cpp",
        "./src/Gizmos.cpp",
        "./src/GUI.cpp",
        "./src/HierarchyWindow.cpp",
        "./src/IO.cpp",
        "./src/LoadingModal.cpp",
        "./src/Logger.cpp",
        "./src/main.cpp",
        "./src/Modal.cpp",
        "./src/Model.cpp",
        "./src/MonoProjectGenerator.cpp",
        "./src/OpenProjectModal.cpp",
        "./src/PixelShader.cpp",
        "./src/ProjectConfigModal.cpp",
        "./src/ProcessManager.cpp",
        "./src/ProfilerData.cpp",
        "./src/ProfilerWindow.cpp",
        "./src/Project.cpp",
        "./src/PropertiesWindow.cpp",
        "./src/RemoteBuildLoadingTask.cpp",
        "./src/RenamePathModal.cpp",
        "./src/RenderCommand.cpp",
        "./src/RunRemoteLoadingTask.cpp",
        "./src/RuntimeManager.cpp",
        "./src/RuntimeModal.cpp",
        "./src/RuntimeStorage.cpp",
        "./src/SceneDefsWindow.cpp",
        "./src/SCPPipe.cpp",
        "./src/SerializeAssetsLoadingTask.cpp",
        "./src/Shader.cpp",
        "./src/ShaderProgram.cpp",
        "./src/ShaderStorage.cpp",
        "./src/ShaderStorageObject.cpp",
        "./src/SSHConnectedModal.cpp",
        "./src/SSHConnectModal.cpp",
        "./src/SSHPipe.cpp",
        "./src/SyncRemoteBuildLoadingTask.cpp",
        "./src/TemplateBuilder.cpp",
        "./src/Texture.cpp",
        "./src/TextureSampler.cpp",
        "./src/TimelineWindow.cpp",
        "./src/UniformBuffer.cpp",
        "./src/VertexShader.cpp",
        "./src/Window.cpp",
        "./src/Workspace.cpp"
    );

    // Used for style editor window
    // CUBE_CProject_AppendSource(&project, "../IcarianEngine/deps/imgui/imgui_demo.cpp");

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

        if (a_targetPlatform == TargetPlatform_LinuxZig)
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86_64_v2");
        }
        else
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86-64-v2");
        }

        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-fwhole-program");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        if (a_targetPlatform == TargetPlatform_LinuxZig)
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86_64_v2");
        }
        else
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86-64-v2");
        }

        CUBE_CProject_AppendCFlag(&project, "-O3");

        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-fwhole-program");

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

            "../IcarianEngine/deps/glfw/build/GLFW.lib",
            "../IcarianEngine/deps/miniz/build/miniz.lib",
            "../IcarianEngine/deps/KTX-Software/build/writec/ktxwritec.lib",
            "../IcarianEngine/deps/KTX-Software/build/writecpp/ktxwritecpp.lib",
            "../IcarianEngine/deps/Mono/Windows/lib/mono-2.0-sgen.lib",
            "../IcarianEngine/deps/Mono/Windows/lib/MonoPosixHelper.lib",
            "../IcarianEngine/deps/zlib/build/zlib.lib",
            "../IcarianEngine/deps/assimp/build/assimp.lib",
            "../IcarianEngine/deps/assimp/contrib/unzip/build/unzip.lib",
            "../IcarianEngine/deps/enet/build/enet.lib"
        );

        CUBE_CProject_AppendReference(&project, "gdi32");
        CUBE_CProject_AppendReference(&project, "wsock32");
        CUBE_CProject_AppendReference(&project, "ws2_32");
        CUBE_CProject_AppendReference(&project, "winmm");

        // Magic string to get std library to link with MinGW
        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    case TargetPlatform_LinuxSteam:
    {
        CUBE_CProject_AppendSystemIncludePath(&project, "../IcarianEngine/deps/Mono/Linux/include/mono-2.0");

        CUBE_CProject_AppendLibraries(&project, 
            "../IcarianEngine/IcarianCore/build/libIcarianCore.a",

            "../IcarianEngine/deps/glfw/build/libGLFW.a",
            "../IcarianEngine/deps/miniz/build/libminiz.a",
            "../IcarianEngine/deps/KTX-Software/build/writec/libktxwritec.a",
            "../IcarianEngine/deps/KTX-Software/build/writecpp/libktxwritecpp.a",
            "../IcarianEngine/deps/Mono/Linux/lib/libmonosgen-2.0.a",
            "../IcarianEngine/deps/zlib/build/libzlib.a",
            "../IcarianEngine/deps/assimp/build/libassimp.a",
            "../IcarianEngine/deps/assimp/contrib/unzip/build/libunzip.a",
            "../IcarianEngine/deps/enet/build/libenet.a"
        );

        CUBE_CProject_AppendReference(&project, "m");
        CUBE_CProject_AppendReference(&project, "stdc++");

        break;
    }
    }

    return project;
}

#ifdef __cplusplus
}
#endif

#endif