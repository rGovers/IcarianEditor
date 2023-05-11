#!/bin/bash

echo "------------------------"
echo
echo "Building Engine"
echo
echo "------------------------"
echo
mkdir bin/crossbuild
cd bin/crossbuild
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 -DGENERATE_CONFIG=ON -DENABLE_TRACE=ON -DENABLE_PROFILER=ON -DCMAKE_TOOLCHAIN_FILE=../../IcarianEngine/toolchains/WinCross.cmake ../../IcarianEngine/IcarianNative/
make -j6
echo
echo "------------------------"
echo
echo "Building Editor CS"
echo
echo "------------------------"
echo
../../IcarianEngine/deps/flare-mono/build/bin/xbuild ../../IcarianEditorBuildEngine/IcarianEditorBuildEngine.csproj /p:Configuration=Release
../../IcarianEngine/deps/flare-mono/build/bin/xbuild ../../IcarianEditorCS/IcarianEditorCS.csproj /p:Configuration=Release
cd ..
mv crossbuild/IcarianNative.exe IcarianNative.exe
mv ../IcarianEditorBuildEngine/bin/*.dll .
mv ../IcarianEditorCS/bin/IcarianCS.exe IcarianCS.dll
mv ../IcarianEditorCS/bin/IcarianEditorCS.dll IcarianEditorCS.dll
# cp -rf ../IcarianEngine/deps/flare-mono/build/lib .
cp -rf ../IcarianEngine/deps/flare-mono/crossbuild/lib .
cp -rf ../IcarianEngine/deps/flare-mono/crossbuild/etc .
# cp ../IcarianEngine/deps/flare-mono/crossbuild/bin/libmonosgen-2.0.dll libmonosgen-2.0.dll
# cp ../IcarianEngine/deps/flare-mono/crossbuild/bin/libMonoPosixHelper.dll libMonoPosixHelper.dll
echo "------------------------"
echo
echo "Building Editor"
echo
echo "------------------------"
echo
cd ..
mkdir crossbuild
cd crossbuild
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 -DCMAKE_TOOLCHAIN_FILE=../IcarianEngine/toolchains/WinCross.cmake ../IcarianEditor
echo
make -j6

cp ../IcarianEngine/deps/flare-mono/crossbuild/bin/libmonosgen-2.0.dll libmonosgen-2.0.dll