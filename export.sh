#!/bin/bash

cd IcarianEngine

if [ -d "../build/BuildFiles" ]; then
    rm -rf ../build/BuildFiles
fi

if [ -d "build" ]; then
    rm -rf build
fi

source "build.sh" "$@" --platform=windows -R
if [ $? -ne 0 ]; then
    echo "Windows export failed"
    exit 1
fi

mkdir -p ../build/BuildFiles/Windows/bin
mkdir -p ../build/BuildFiles/Windows/lib

cp -r build/. ../build/BuildFiles/Windows/bin
cp build/IcarianCS.dll ../build/BuildFiles/Windows/lib/IcarianCS.dll

if [ -d "build" ]; then
    rm -rf build
fi

source "build.sh" "$@" --platform=linux -R
if [ $? -ne 0 ]; then
    echo "Linux export failed"
    exit 1
fi

mkdir -p ../build/BuildFiles/Linux/bin
mkdir -p ../build/BuildFiles/Linux/lib

cp -r build/. ../build/BuildFiles/Linux/bin
cp build/IcarianCS.dll ../build/BuildFiles/Linux/lib/IcarianCS.dll

if ! command -v podman 2>&1 > /dev/null 
then
    echo "No Podman skipping Linux Steam version"
else
    podman pull registry.gitlab.steamos.cloud/steamrt/sniper/sdk:latest

    if [ $? -ne 0 ]; then
        echo "Steam Sniper runtime fetch failed"
        exit 1
    fi

    if [ -d "build" ]; then
        rm -rf build
    fi

    podman run \
        --rm \
        --init \
        -e HOME="$HOME" \
        -h "$(hostname)" \
        -v /tmp:/tmp \
        -v $PWD:/IcarianEngine/:z \
        -it \
        -w /IcarianEngine \
        registry.gitlab.steamos.cloud/steamrt/sniper/sdk:latest \
        /bin/bash /IcarianEngine/build.sh "$@" --platform=linuxsteam -R --rebuild

    if [ $? -ne 0 ]; then
        echo "Linux Steam export failed"
        exit 1
    fi

    mkdir -p "../build/BuildFiles/LinuxSteam/bin"
    mkdir -p "../build/BuildFiles/LinuxSteam/lib"

    cp -r build/. "../build/BuildFiles/LinuxSteam/bin"
    cp build/IcarianCS.dll "../build/BuildFiles/LinuxSteam/lib/IcarianCS.dll"
fi

echo "-------------------------------------------"
echo
echo "Export Complete"
echo
echo "-------------------------------------------"