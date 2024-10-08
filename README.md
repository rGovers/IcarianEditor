# IcarianEditor

![image](resources/Icarian_Logo_White.svg)

The editor for Icarian Engine.

## Prerequisites
* Vulkan SDK (Validation Layers in Debug not required in Release)
* Python3 (glslang)
### Linux
* GCC, Clang or Zig (GCC primary)
* Wayland
* X11
* XOrg Client
* Pulse
* Jack
* ALSA
### Windows
* MingGW

## Building

Building is done via CUBE.

The project can be built by running the following.
```
./build.sh
```

For extra options refer to --help on setup.sh.

Output is in the build folder.

Windows builds are done via cross compilation with MinGW.
MSVC is currently not supported.

### Export
In order to get editor exports you need to build the export files. Editor exports are not included in the main build process as they can take a very long time to compile on some systems as it requires compiling the engine for all platforms. Note that the editor will crash if you try to export builds without building them at this stage.

They can be generated with by running the following
```
./export.sh
```