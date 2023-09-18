#define CUBE_IMPLEMENTATION
#define CUBE_PRINT_COMMANDS
#include "CUBE/CUBE.h"

#include "IcarianEngine/BuildBase.h"
#include "IcarianEngine/deps/BuildDependencies.h"
#include "IcarianEngine/FlareBase/BuildFlareBase.h"
#include "IcarianEngine/IcarianCS/BuildIcarianCS.h"
#include "IcarianEngine/IcarianNative/BuildIcarianNative.h"

#include "IcarianEditor/BuildIcarianEditor.h"
#include "IcarianEditorCS/BuildIcarianEditorCS.h"

int main(int a_argc, char** a_argv)
{
    e_TargetPlatform targetPlatform;
    e_BuildConfiguration buildConfiguration;

    CUBE_CProject flareBaseProject;
    CUBE_CSProject icarianCSProject;
    CUBE_CProject icarianNativeProject;
    CUBE_CSProject icarianEditorCSProject;
    CUBE_CProject icarianEditorProject;

    e_CUBE_CProjectCompiler compiler;

    CBUINT32 dependencyProjectCount;
    DependencyProject* dependencyProjects;

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
    }

    icarianEnginePath = CUBE_Path_CreateC("IcarianEngine");

    // PrintHeader("Building Dependencies");

    // printf("Creating Dependencies projects...\n");

    // dependencyProjects = BuildDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    // printf("Compiling Dependencies...\n");
    // for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    // {
    //     printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

    //     CUBE_Path workingDirectory = CUBE_Path_CombineC(&icarianEnginePath, dependencyProjects[i].WorkingDirectory);
    //     CUBE_String workingDirectoryStr = CUBE_Path_ToString(&workingDirectory);

    //     ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, compiler, workingDirectoryStr.Data, CBNULL, &lines, &lineCount);

    //     CUBE_String_Destroy(&workingDirectoryStr);
    //     CUBE_Path_Destroy(&workingDirectory);

    //     FlushLines(&lines, &lineCount);

    //     if (!ret)
    //     {
    //         printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

    //         return 1;
    //     }

    //     printf("Compiled %s\n", dependencyProjects[i].Project.Name.Data);

    //     CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    // }

    // free(dependencyProjects);

    PrintHeader("Building FlareBase");

    printf("Creating FlareBase project...\n");
    flareBaseProject = BuildFlareBaseProject(CBTRUE, targetPlatform, buildConfiguration);

    printf("Compiling FlareBase...\n");
    ret = CUBE_CProject_Compile(&flareBaseProject, compiler, "IcarianEngine/FlareBase", CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&flareBaseProject);

    if (!ret)
    {
        printf("Failed to compile FlareBase\n");

        return 1;
    }

    printf("FlareBase Compiled!\n");

    PrintHeader("Building IcarianCS");

    printf("Creating IcarianCS project...\n");
    icarianCSProject = BuildIcarianCSProject(CBTRUE);

    printf("Compiling IcarianCS...\n");
    ret = CUBE_CSProject_Compile(&icarianCSProject, "IcarianEngine/IcarianCS", "../deps/Mono/Linux/bin/csc", &lines, &lineCount);

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

        ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, compiler, workingDirectoryStr.Data, CBNULL, &lines, &lineCount);

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
    icarianNativeProject = BuildIcarianNativeProject(targetPlatform, buildConfiguration);

    printf("Compiling IcarianNative...\n");
    ret = CUBE_CProject_Compile(&icarianNativeProject, compiler, "IcarianEngine/IcarianNative", CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

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
    ret = CUBE_CSProject_Compile(&icarianEditorCSProject, "IcarianEditorCS", "../IcarianEngine/deps/Mono/Linux/bin/csc", &lines, &lineCount);

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
    ret = CUBE_CProject_Compile(&icarianEditorProject, compiler, "IcarianEditor", CBNULL, &lines, &lineCount);

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

    switch (targetPlatform)
    {
    case TargetPlatform_Linux:
    {
        CUBE_IO_CopyFileC("IcarianEditor/build/IcarianEditor", "build/IcarianEditor");
        CUBE_IO_CopyFileC("IcarianEngine/IcarianNative/build/IcarianNative", "build/IcarianNative");

        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/lib", "build/lib", CBTRUE);
        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/etc", "build/etc", CBTRUE);
        CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/bin", "build/bin", CBTRUE);

        break;
    }
    }

    CUBE_IO_CopyFileC("IcarianEditorCS/build/IcarianEditorCS.dll", "build/IcarianEditorCS.dll");
    CUBE_IO_CopyFileC("IcarianEngine/IcarianCS/build/IcarianCS.exe", "build/IcarianCS.dll");

    CUBE_IO_CopyDirectoryC("bin", "build", CBTRUE);

    printf("Done!\n");

    CUBE_Path_Destroy(&icarianEnginePath);

    return 0;
}