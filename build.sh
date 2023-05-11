#!/bin/bash

echo "------------------------"
echo
echo "Building Engine"
echo
echo "------------------------"
echo
if [ ! -d "bin/build" ]; then
    mkdir bin/build
fi
cd bin/build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 -DGENERATE_CONFIG=ON -DENABLE_TRACE=ON -DENABLE_PROFILER=ON ../../IcarianEngine/IcarianNative/
make -j6
echo
echo "------------------------"
echo
echo "Building Editor CS"
echo
echo "------------------------"
echo
../../IcarianEngine/deps/flare-mono/build/bin/xbuild ../../IcarianEditorCS/IcarianEditorCS.csproj /p:Configuration=Release
cd ..
mv build/IcarianNative IcarianNative
mv ../IcarianEditorCS/bin/IcarianCS.exe IcarianCS.dll
mv ../IcarianEditorCS/bin/IcarianEditorCS.exe IcarianEditorCS.dll
cp -r ../IcarianEngine/deps/flare-mono/build/lib .
cp -r ../IcarianEngine/deps/flare-mono/build/etc .
echo "------------------------"
echo
echo "Building Editor"
echo
echo "------------------------"
echo
cd ..
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 ../IcarianEditor
echo
make -j6
