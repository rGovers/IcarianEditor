#define CUBE_IMPLEMENTATION
#define CUBE_PRINT_COMMANDS
#include "CUBE/CUBE.h"

#include "IcarianEngine/BuildBase.h"

#include "IcarianEngine/deps/BuildDependencies.h"
#include "IcarianEngine/FlareBase/BuildFlareBase.h"
#include "IcarianEngine/IcarianCS/BuildIcarianCS.h"
#include "IcarianEngine/IcarianNative/BuildIcarianNative.h"

CBBOOL BuildPlatform(const CUBE_Path* a_enginePath, e_TargetPlatform a_platform)
{
    CBUINT32 lineCount;
    CUBE_String* lines;

    CBBOOL ret;

    CBUINT32 dependencyProjectCount;
    DependencyProject* dependencyProjects;

    CUBE_CProject flareBaseProject;
    CUBE_CProject icarianNativeProject;

    e_CUBE_CProjectCompiler compiler;

    switch (a_platform)
    {
    case TargetPlatform_Windows:
    {
        compiler = CUBE_CProjectCompiler_MinGW;

        break;
    }
    case TargetPlatform_Linux:
    {
        compiler = CUBE_CProjectCompiler_GCC;

        break;
    }
    }

    lineCount = 0;
    lines = CBNULL;

    switch (a_platform)
    {
    case TargetPlatform_Windows:
    {
        PrintHeader("Building Windows Export");

        break;
    }
    case TargetPlatform_Linux:
    {
        PrintHeader("Building Linux Export");

        break;
    }
    }

    dependencyProjects = BuildDependencies(&dependencyProjectCount, a_platform, BuildConfiguration_Release);
    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

        CUBE_Path workingDirectory = CUBE_Path_CombineC(a_enginePath, dependencyProjects[i].WorkingDirectory);
        CUBE_String workingDirectoryStr = CUBE_Path_ToString(&workingDirectory);

        ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, compiler, workingDirectoryStr.Data, CBNULL, &lines, &lineCount);

        CUBE_String_Destroy(&workingDirectoryStr);
        CUBE_Path_Destroy(&workingDirectory);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return CBFALSE;
        }

        printf("Compiled %s\n", dependencyProjects[i].Project.Name.Data);

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    flareBaseProject = BuildFlareBaseProject(CBTRUE, a_platform, BuildConfiguration_Release);

    printf("Compiling FlareBase...\n");

    ret = CUBE_CProject_Compile(&flareBaseProject, compiler, "IcarianEngine/FlareBase", CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&flareBaseProject);

    if (!ret)
    {
        printf("Failed to compile FlareBase\n");

        return CBFALSE;
    }
    printf("Compiled FlareBase\n");

    dependencyProjects = BuildIcarianNativeDependencies(&dependencyProjectCount, a_platform, BuildConfiguration_Release);
    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

        CUBE_Path workingDirectory = CUBE_Path_CombineC(a_enginePath, dependencyProjects[i].WorkingDirectory);
        CUBE_String workingDirectoryStr = CUBE_Path_ToString(&workingDirectory);

        ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, compiler, workingDirectoryStr.Data, CBNULL, &lines, &lineCount);

        CUBE_String_Destroy(&workingDirectoryStr);
        CUBE_Path_Destroy(&workingDirectory);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return CBFALSE;
        }

        printf("Compiled %s\n", dependencyProjects[i].Project.Name.Data);

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    printf("Compiling IcarianNative...\n");

    icarianNativeProject = BuildIcarianNativeProject(a_platform, BuildConfiguration_Release, CBFALSE, CBFALSE);

    ret = CUBE_CProject_Compile(&icarianNativeProject, compiler, "IcarianEngine/IcarianNative", CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&icarianNativeProject);

    if (!ret)
    {
        printf("Failed to compile IcarianNative\n");

        return CBFALSE;
    }

    printf("Compiled IcarianNative\n");

    return CBTRUE;
}

int main(int a_argc, char** a_argv)
{
    CUBE_CSProject icarianCSProject;

    CBUINT32 lineCount;
    CUBE_String* lines;

    CBBOOL ret;

    CUBE_Path icarianEnginePath;

    lineCount = 0;
    lines = CBNULL;

    icarianEnginePath = CUBE_Path_CreateC("IcarianEngine");

    PrintHeader("Building Universal Export");

    printf("Compiling IcarianCS...\n");

    icarianCSProject = BuildIcarianCSProject(CBTRUE);

    ret = CUBE_CSProject_Compile(&icarianCSProject, "IcarianEngine/IcarianCS", "../deps/Mono/Linux/bin/csc", &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CSProject_Destroy(&icarianCSProject);

    printf("Compiled IcarianCS\n");

    if (!ret)
    {
        printf("Failed to compile IcarianCS\n");

        return 1;
    }

    if (!BuildPlatform(&icarianEnginePath, TargetPlatform_Windows))
    {
        printf("Failed to build Windows Export\n");

        return 1;
    }

    PrintHeader("Creating Windows BuildFiles");

    CUBE_IO_CreateDirectoryC("build");
    CUBE_IO_CreateDirectoryC("build/BuildFiles");
    CUBE_IO_CreateDirectoryC("build/BuildFiles/Windows");
    CUBE_IO_CreateDirectoryC("build/BuildFiles/Windows/bin");
    CUBE_IO_CreateDirectoryC("build/BuildFiles/Windows/lib");

    CUBE_IO_CopyFileC("IcarianEngine/IcarianNative/build/IcarianNative.exe", "build/BuildFiles/Windows/bin/renameexe");
    CUBE_IO_CopyFileC("IcarianEngine/IcarianCS/build/IcarianCS.exe", "build/BuildFiles/Windows/bin/IcarianCS.dll");
    CUBE_IO_CopyFileC("IcarianEngine/deps/Mono/Windows/bin/mono-2.0-sgen.dll", "build/BuildFiles/Windows/bin/mono-2.0-sgen.dll");
    CUBE_IO_CopyFileC("IcarianEngine/deps/Mono/Windows/bin/MonoPosixHelper.dll", "build/BuildFiles/Windows/bin/MonoPosixHelper.dll");

    CUBE_IO_CopyFileC("IcarianEngine/IcarianCS/build/IcarianCS.exe", "build/BuildFiles/Windows/lib/IcarianCS.dll");

    CUBE_IO_CHMODC("build/BuildFiles/Windows/bin/renameexe.exe", 0755);

    CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Windows/lib/mono/", "build/BuildFiles/Windows/bin/lib/mono/", CBTRUE);
    CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Windows/etc/", "build/BuildFiles/Windows/bin/etc/", CBTRUE);

    if (!BuildPlatform(&icarianEnginePath, TargetPlatform_Linux))
    {
        printf("Failed to build Linux Export\n");

        return 1;
    }

    PrintHeader("Creating Linux BuildFiles");

    CUBE_IO_CreateDirectoryC("build/BuildFiles/Linux");
    CUBE_IO_CreateDirectoryC("build/BuildFiles/Linux/bin");
    CUBE_IO_CreateDirectoryC("build/BuildFiles/Linux/lib");

    CUBE_IO_CopyFileC("IcarianEngine/IcarianNative/build/IcarianNative", "build/BuildFiles/Linux/bin/renameexe");
    CUBE_IO_CopyFileC("IcarianEngine/IcarianCS/build/IcarianCS.exe", "build/BuildFiles/Linux/bin/IcarianCS.dll");

    CUBE_IO_CopyFileC("IcarianEngine/IcarianCS/build/IcarianCS.exe", "build/BuildFiles/Linux/lib/IcarianCS.dll");

    CUBE_IO_CHMODC("build/BuildFiles/Linux/bin/renameexe", 0755);

    CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/lib/mono/", "build/BuildFiles/Linux/bin/lib/mono/", CBTRUE);
    CUBE_IO_CopyDirectoryC("IcarianEngine/deps/Mono/Linux/etc/", "build/BuildFiles/Linux/bin/etc/", CBTRUE);

    CUBE_Path_Destroy(&icarianEnginePath);

    return 0;
}