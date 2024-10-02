#define CUBE_IMPLEMENTATION
#define CUBE_PRINT_COMMANDS
#include "CUBE/CUBE.h"

#include "IcarianEngine/BuildBase.h"
#include "IcarianEngine/deps/BuildDependencies.h"
#include "IcarianEngine/IcarianCore/BuildIcarianCore.h"
#include "IcarianEngine/IcarianCS/BuildIcarianCS.h"
#include "IcarianEngine/IcarianModManager/BuildIcarianModManager.h"
#include "IcarianEngine/IcarianNative/BuildIcarianNative.h"

#include "IcarianEditor/BuildIcarianEditor.h"
#include "IcarianEditorCS/BuildIcarianEditorCS.h"

int main(int a_argc, char** a_argv)
{
    e_TargetPlatform targetPlatform;
    e_BuildConfiguration buildConfiguration;

    CUBE_CProject icarianCoreProject;
    CUBE_CSProject icarianCSProject;
    CUBE_CProject icarianNativeProject;
    CUBE_CProject icarianModManagerProject;
    CUBE_CSProject icarianEditorCSProject;
    CUBE_CProject icarianEditorProject;

    e_CUBE_CProjectCompiler compiler;

    CBUINT32 dependencyProjectCount;
    DependencyProject* dependencyProjects;

    CBUINT32 jobThreads;

    CBUINT32 lineCount;
    CUBE_String* lines;

    CBBOOL ret;

    CUBE_Path icarianEnginePath;

#ifdef _WIN32
    targetPlatform = TargetPlatform_Windows;
#else
    targetPlatform = TargetPlatform_Linux;
#endif

    buildConfiguration = BuildConfiguration_Debug;

    lineCount = 0;
    lines = CBNULL;

    jobThreads = 4;

    printf("IcarianEditor Build\n");
    printf("\n");

    // Dont need the first arg
    for (int i = 1; i < a_argc; ++i)
    {
        if (strncmp(a_argv[i], PlatformString, PlatformStringLen) == 0)
        {
            const char* platformStr = a_argv[i] + PlatformStringLen;
            while (*platformStr != '=' && *platformStr != '\0')
            {
                ++platformStr;
            }

            if (*platformStr == '\0')
            {
                printf("Invalid platform argument: %s\n", a_argv[i]);

                return 1;
            }

            ++platformStr;

            if (strcmp(platformStr, "windows") == 0)
            {
                targetPlatform = TargetPlatform_Windows;
            }
            else if (strcmp(platformStr, "linux") == 0)
            {
                targetPlatform = TargetPlatform_Linux;
            }
            else if (strcmp(platformStr, "linuxclang") == 0)
            {
                targetPlatform = TargetPlatform_LinuxClang;
            }
            else if (strcmp(platformStr, "linuxzig") == 0)
            {
                targetPlatform = TargetPlatform_LinuxZig;
            }
            else
            {
                printf("Unknown platform: %s\n", platformStr);

                return 1;
            }
        }
        else if (strncmp(a_argv[i], BuildConfigurationString, BuildConfigurationStringLen) == 0)
        {
            const char* buildConfigurationStr = a_argv[i] + BuildConfigurationStringLen;
            while (*buildConfigurationStr != '=' && *buildConfigurationStr != '\0')
            {
                ++buildConfigurationStr;
            }

            if (*buildConfigurationStr == '\0')
            {
                printf("Invalid build configuration argument: %s\n", a_argv[i]);

                return 1;
            }

            ++buildConfigurationStr;

            if (strcmp(buildConfigurationStr, "debug") == 0)
            {
                buildConfiguration = BuildConfiguration_Debug;
            }
            else if (strcmp(buildConfigurationStr, "releasewithdebug") == 0)
            {
                buildConfiguration = BuildConfiguration_ReleaseWithDebug;
            }
            else if (strcmp(buildConfigurationStr, "release") == 0)
            {
                buildConfiguration = BuildConfiguration_Release;
            }
            else 
            {
                printf("Unknown build configuration: %s\n", buildConfigurationStr);

                return 1;
            }
        }
        else if (strncmp(a_argv[i], CompileCommandsString, CompileCommandsStringLen) == 0)
        {
            // TODO: Clean this up so passing a working directory is not required
            CBUINT32 offset;
            CBUINT32 engineDependencyCount;
            DependencyProject* engineDependencies;

            const char* compileCommandsStr = a_argv[i] + CompileCommandsStringLen;
            while (*compileCommandsStr != '=' && *compileCommandsStr != '\0')
            {
                ++compileCommandsStr;
            }

            if (*compileCommandsStr == '\0')
            {
                printf("Invalid compile commands argument: %s\n", a_argv[i]);
            }

            ++compileCommandsStr;

            printf("Generating compile commands projects \n");
            dependencyProjects = BuildDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);
            engineDependencies = BuildIcarianNativeDependencies(&engineDependencyCount, targetPlatform, buildConfiguration);
            
            CBUINT32 finalCount = dependencyProjectCount + engineDependencyCount + 4;
            CUBE_CProject* projects = malloc(sizeof(CUBE_CProject) * finalCount);
            offset = 0;
            for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
            {
                CUBE_CProject proj = dependencyProjects[i].Project;
                CUBE_CProject_PrependPaths(&proj, dependencyProjects[i].WorkingDirectory, CBTRUE);
                CUBE_CProject_PrependPaths(&proj, "./IcarianEngine/", CBTRUE);
                projects[offset++] = proj;
            }

            free(dependencyProjects);

            icarianCoreProject = BuildIcarianCoreProject(CBTRUE, targetPlatform, buildConfiguration);
            CUBE_CProject_PrependPaths(&icarianCoreProject, "./IcarianEngine/IcarianCore/", CBTRUE);
            projects[offset++] = icarianCoreProject;

            for (CBUINT32 i = 0; i < engineDependencyCount; ++i)
            {
                CUBE_CProject proj = engineDependencies[i].Project;
                CUBE_CProject_PrependPaths(&proj, engineDependencies[i].WorkingDirectory, CBTRUE);
                CUBE_CProject_PrependPaths(&proj, "./IcarianEngine/", CBTRUE);
                projects[offset++] = proj;
            }

            free(engineDependencies);

            icarianNativeProject = BuildIcarianNativeProject(targetPlatform, buildConfiguration, CBTRUE, CBTRUE);
            CUBE_CProject_PrependPaths(&icarianNativeProject, "./IcarianEngine/IcarianNative/", CBTRUE);
            projects[offset++] = icarianNativeProject;

            icarianModManagerProject = BuildIcarianModManagerProject(targetPlatform, buildConfiguration);
            CUBE_CProject_PrependPaths(&icarianModManagerProject, "./IcarianEngine/IcarianModManager", CBTRUE);
            projects[offset++] = icarianModManagerProject;

            icarianEditorProject = BuildIcarianEditorProject(targetPlatform, buildConfiguration);
            CUBE_CProject_PrependPaths(&icarianEditorProject, "./IcarianEditor", CBTRUE);
            projects[offset++] = icarianEditorProject;

            printf("Generating compile commands \n");
            CUBE_String compCom = CUBE_CProject_GenerateMultiCompileCommands(projects, finalCount, CUBE_CProjectCompiler_GCC, compileCommandsStr);

            printf("Writing compile commands \n");
            CUBE_IO_WriteFileC("compile_commands.json", compCom.Data, compCom.Length);

            CUBE_String_Destroy(&compCom);

            for (CBUINT32 i = 0; i < finalCount; ++i)
            {
                CUBE_CProject_Destroy(&(projects[i]));
            }

            free(projects);

            printf("Done!\n");

            return 0;
        }
        else if (strncmp(a_argv[i], JobString, JobStringLen) == 0)
        {
            const char* jobCountStr = a_argv[i] + JobStringLen;
            while (*jobCountStr != '=' && *jobCountStr != 0)
            {
                ++jobCountStr;
            }

            if (*jobCountStr == 0)
            {
                printf("Invalid job count argument: %s\n", a_argv[i]);

                return 1;
            }

            ++jobCountStr;

            jobThreads = (CBUINT32)atoi(jobCountStr);
        }
        else if (strncmp(a_argv[i], HelpString, HelpStringLen) == 0)
        {
            PrintHelp();

            return 0;
        }
        else if (strcmp(a_argv[i], "-D") == 0)
        {
            buildConfiguration = BuildConfiguration_Debug;
        }
        else if (strcmp(a_argv[i], "-R") == 0)
        {
            buildConfiguration = BuildConfiguration_Release;
        }
        else
        {
            printf("Unknown argument: %s\n", a_argv[i]);
            printf("\n");

            PrintHelp();

            return 1;
        }
    }

    switch (targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        printf("Target Platform: Windows\n");

        compiler = CUBE_CProjectCompiler_MinGW;

        break;
    }
    case TargetPlatform_Linux:
    {
        printf("Target Platform: Linux\n");

        compiler = CUBE_CProjectCompiler_GCC;

        break;
    }
    case TargetPlatform_LinuxClang:
    {
        printf("Target Platform: Linux Clang\n");

        compiler = CUBE_CProjectCompiler_Clang;

        break;
    }
    case TargetPlatform_LinuxZig:
    {
        printf("Target Platform: Linux Zig\n");

        compiler = CUBE_CProjectCompiler_Zig;

        break;
    }
    }

    icarianEnginePath = CUBE_Path_CreateC("IcarianEngine");

    CUBE_CommandLine commandLine = { 0 };

    CUBE_String_AppendC(&commandLine.Path, "IcarianEngine/IcarianNative/lib/glslang");
    CUBE_String_AppendC(&commandLine.Command, "python3");

    CUBE_CommandLine_AppendArgumentC(&commandLine, "update_glslang_sources.py");

    int retCode = CUBE_CommandLine_Execute(&commandLine, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CommandLine_Destroy(&commandLine);

    if (retCode != 0)
    {
        printf("Failed to sync SPIRV-Tools\n");

        return 1;
    }

    PrintHeader("Building Dependencies");

    printf("Creating Dependencies projects...\n");

    dependencyProjects = BuildDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    printf("Compiling Dependencies...\n");
    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

        CUBE_Path workingDirectory = CUBE_Path_CombineC(&icarianEnginePath, dependencyProjects[i].WorkingDirectory);
        CUBE_String workingDirectoryStr = CUBE_Path_ToString(&workingDirectory);

        ret = CUBE_CProject_MultiCompile(&dependencyProjects[i].Project, compiler, workingDirectoryStr.Data, CBNULL, jobThreads, &lines, &lineCount);

        CUBE_String_Destroy(&workingDirectoryStr);
        CUBE_Path_Destroy(&workingDirectory);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return 1;
        }

        printf("Compiled %s\n", dependencyProjects[i].Project.Name.Data);

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    PrintHeader("Building IcarianCore");

    printf("Creating IcarianCore project...\n");
    icarianCoreProject = BuildIcarianCoreProject(CBTRUE, targetPlatform, buildConfiguration);

    printf("Compiling IcarianCore...\n");
    ret = CUBE_CProject_MultiCompile(&icarianCoreProject, compiler, "IcarianEngine/IcarianCore", CBNULL, jobThreads, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&icarianCoreProject);

    if (!ret)
    {
        printf("Failed to compile IcarianCore\n");

        return 1;
    }

    printf("IcarianCore Compiled!\n");

    PrintHeader("Building IcarianCS");
    printf("Writing imports to Header files...\n");
    if (!WriteIcarianCSImportsToHeader("IcarianEngine/IcarianCS"))
    {
        printf("Failed to write imports to header files\n");

        return 1;
    }

    printf("Creating IcarianCS project...\n");
    icarianCSProject = BuildIcarianCSProject(CBTRUE, CBFALSE);

    printf("Compiling IcarianCS...\n");
    ret = CUBE_CSProject_PreProcessCompile(&icarianCSProject, "IcarianEngine/IcarianCS", "../deps/Mono/Linux/bin/csc", compiler, CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CSProject_Destroy(&icarianCSProject);

    if (!ret)
    {
        printf("Failed to compile IcarianCS\n");

        return 1;
    }

    printf("IcarianCS Compiled!\n");

    PrintHeader("Building IcarianNative");

    printf("Writing shaders to Header files...\n");
    if (!WriteIcarianNativeShadersToHeader("IcarianEngine/IcarianNative"))
    {
        printf("Failed to write shaders to header files\n");

        return 1;
    }

    printf("Creating IcarianNative Dependencies projects...\n");
    dependencyProjects = BuildIcarianNativeDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    printf("Compiling IcarianNative Dependencies...\n");
    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

        CUBE_Path workingDirectory = CUBE_Path_CombineC(&icarianEnginePath, dependencyProjects[i].WorkingDirectory);
        CUBE_String workingDirectoryStr = CUBE_Path_ToString(&workingDirectory);

        ret = CUBE_CProject_MultiCompile(&dependencyProjects[i].Project, compiler, workingDirectoryStr.Data, CBNULL, jobThreads, &lines, &lineCount);

        CUBE_String_Destroy(&workingDirectoryStr);
        CUBE_Path_Destroy(&workingDirectory);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return 1;
        }

        printf("Compiled %s\n", dependencyProjects[i].Project.Name.Data);

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    printf("Creating IcarianNative project...\n");
    icarianNativeProject = BuildIcarianNativeProject(targetPlatform, buildConfiguration, CBTRUE, CBTRUE);

    printf("Compiling IcarianNative...\n");
    ret = CUBE_CProject_MultiCompile(&icarianNativeProject, compiler, "IcarianEngine/IcarianNative", CBNULL, jobThreads, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&icarianNativeProject);

    if (!ret)
    {
        printf("Failed to compile IcarianNative\n");

        return 1;
    }

    printf("IcarianNative Compiled!\n");

    PrintHeader("Building IcarianEditorCS");
    
    printf("Creating IcarianEditorCS project...\n");
    icarianEditorCSProject = BuildIcarianEditorCSProject(CBTRUE);

    printf("Compiling IcarianEditorCS...\n");
    ret = CUBE_CSProject_PreProcessCompile(&icarianEditorCSProject, "IcarianEditorCS", "../IcarianEngine/deps/Mono/Linux/bin/csc", compiler, CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CSProject_Destroy(&icarianEditorCSProject);

    if (!ret)
    {
        printf("Failed to compile IcarianEditorCS\n");

        return 1;
    }

    printf("IcarianEditorCS Compiled!\n");

    PrintHeader("Building IcarianEditor");

    printf("Writing shaders to header files...\n");
    if (!WriteIcarianEditorShadersToHeader("IcarianEditor"))
    {
        printf("Failed to write shaders to header files\n");

        return 1;
    }

    printf("Writing templates to header files...\n");
    if (!WriteTemplateToHeader("IcarianEditor"))
    {
        printf("Failed to write templates to header files\n");

        return 1;
    }

    printf("Creating IcarianEditor project...\n");
    icarianEditorProject = BuildIcarianEditorProject(targetPlatform, buildConfiguration);

    printf("Compiling IcarianEditor...\n");
    ret = CUBE_CProject_MultiCompile(&icarianEditorProject, compiler, "IcarianEditor", CBNULL, jobThreads, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&icarianEditorProject);

    if (!ret)
    {
        printf("Failed to compile IcarianEditor\n");

        return 1;
    }

    printf("IcarianEditor Compiled!\n");

    PrintHeader("Copying Files");

    CUBE_IO_CreateDirectoryC("build");

    CUBE_IO_CreateDirectoryC("build/lib");
    CUBE_IO_CreateDirectoryC("build/etc");
    CUBE_IO_CreateDirectoryC("build/bin");

    CUBE_IO_CopyFileC("IcarianEditorCS/build/IcarianEditorCS.dll", "build/IcarianEditorCS.dll");
    CUBE_IO_CopyFileC("IcarianEngine/IcarianCS/build/IcarianCS.dll", "build/IcarianCS.dll");

    CUBE_IO_CopyDirectoryC("bin", "build", CBTRUE);

    switch (targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_IO_CopyFileC("IcarianEditor/build/IcarianEditor.exe", "build/IcarianEditor.exe");
        CUBE_IO_CopyFileC("IcarianEngine/IcarianNative/build/IcarianNative.exe", "build/IcarianNative.exe");

        CUBE_IO_CopyFileC("IcarianEngine/deps/Mono/Windows/bin/mono-2.0-sgen.dll", "build/mono-2.0-sgen.dll");
        CUBE_IO_CopyFileC("IcarianEngine/deps/Mono/Windows/bin/MonoPosixHelper.dll", "build/MonoPosixHelper.dll");

        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Windows/lib", "build/lib", CBTRUE);
        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Windows/etc", "build/etc", CBTRUE);
        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Windows/bin", "build/bin", CBTRUE);

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_IO_CopyFileC("IcarianEditor/build/IcarianEditor", "build/IcarianEditor");
        CUBE_IO_CopyFileC("IcarianEngine/IcarianNative/build/IcarianNative", "build/IcarianNative");

        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/lib", "build/lib", CBTRUE);
        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/etc", "build/etc", CBTRUE);
        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/bin", "build/bin", CBTRUE);

        CUBE_IO_CHMODC("build/IcarianEditor", 0755);
        CUBE_IO_CHMODC("build/IcarianNative", 0755);

        CUBE_IO_CHMODC("build/bin/mono", 0755);
        CUBE_IO_CHMODC("build/bin/csc", 0755);

        break;
    }
    }

    printf("Done!\n");

    CUBE_Path_Destroy(&icarianEnginePath);

    return 0;
}