# IcarianEditor

![image](IcarianEngine/resources/Icarian_Logo_White.svg)

The editor for Icarian Engine.

## Prerequisites
* Vulkan SDK
### Linux
* GCC
### Windows
* MingGW

## Building

Building is done via CUBE.

The project can be built by running build.sh.

For extra options refer to --help on setup.sh.

Output is in the build folder.

Windows builds are done via cross compilation with MinGW.
MSVC is currently not supported.

### Export
In order to get editor exports you need to build the export files

They can be generated with ./export.sh