#!/bin/bash

# I may reuse the build files but making a seperate build just incase
# Main reason I am doing this is I may add Video Encode down the line but probably not have it in the main build

cd IcarianEngine

if [ -d "../build/RemoteFiles" ]; then
    rm -rf ../build/RemoteFiles
fi

if [ -d "build" ]; then
    rm -rf build
fi

source "build.sh" "$@" --platform=windows --remote --disable-mod --enable-profiler -R
if [ $? -ne 0 ]; then
    echo "Windows remote build failed"
    exit 1
fi

mkdir -p ../build/RemoteFiles/Windows/bin
mkdir -p ../build/RemoteFiles/Windows/lib

cp -r build/. ../build/RemoteFiles/Windows/bin
cp build/IcarianCS.dll ../build/RemoteFiles/Windows/lib/IcarianCS.dll

if [ -d "build" ]; then
    rm -rf build
fi

source "build.sh" "$@" --platform=linux --remote --disable-mod --enable-profiler -R
if [ $? -ne 0 ]; then
    echo "Linux remote build failed"
    exit 1
fi

mkdir -p ../build/RemoteFiles/Linux/bin
mkdir -p ../build/RemoteFiles/Linux/lib

cp -r build/. ../build/RemoteFiles/Linux/bin
cp build/IcarianCS.dll ../build/RemoteFiles/Linux/lib/IcarianCS.dll

echo "-------------------------------------------"
echo
echo "Remote Files Complete"
echo
echo "-------------------------------------------"