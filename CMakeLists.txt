cmake_minimum_required(VERSION 3.0)

set(OUTPUT_BIN ON)

add_subdirectory("IcarianEditor/")
add_subdirectory("IcarianEngine/")

project(IcarianEditorCS VERSION 0.1 LANGUAGES CSharp)

include(CSharpUtilities)

file(GLOB_RECURSE SOURCES "IcarianEditorCS/src/*.cs")

add_executable(IcarianEditorCS ${SOURCES})

set_target_properties(IcarianEditorCS PROPERTIES VS_DOTNET_REFERENCES "System.Xml")
set_target_properties(IcarianEditorCS PROPERTIES VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6")
set_target_properties(IcarianEditorCS PROPERTIES WIN32_EXECUTABLE FALSE)
set_target_properties(IcarianEditorCS
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/"
)

add_custom_command(TARGET IcarianEditorCS
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/bin/IcarianEditorCS.exe"
        "${CMAKE_SOURCE_DIR}/bin/IcarianEditorCS.dll")

target_link_libraries(IcarianEditorCS IcarianCS)
