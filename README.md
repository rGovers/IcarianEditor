# IcarianEditor

![image](resources/Icarian_Logo_White.svg)

The editor for Icarian Engine.

## Prerequisites
* Vulkan SDK (Validation Layers in Debug not required in Release)
* Python3 (glslang)
### Linux
* GCC, Clang or Zig (GCC primary compiler)
* Wayland
* X11
* XOrg Client
* Pulse
* Jack
* ALSA
### Linux Steam Export Target
* podman
### Windows Export Target
* MinGW

## Building

Building is done via CUBE.

Building can be done by running the following scripts.
``` bash
# Main editor build
./build.sh
# Platform export targets
./export.sh
# Remote debugging support
./remoteBuild.sh
```

For extra options refer to --help on build.sh.

Output is in the build folder.

## Windows
Windows builds are currently not supported for the Editor due to Windowsisms in WIN32 eating a lot of development time. May revist at a later time.