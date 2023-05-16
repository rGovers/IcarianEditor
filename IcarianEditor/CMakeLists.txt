cmake_minimum_required (VERSION 3.0)

project (IcarianEditor VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

set (ICARIANEDITOR_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set (ICARIANEDITOR_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})

configure_file(
    "${PROJECT_SOURCE_DIR}/IcarianEditorConfig.h.in"
    "${PROJECT_SOURCE_DIR}/include/IcarianEditorConfig.h"
)

add_compile_definitions(GLM_FORCE_QUAT_DATA_XYZW)
add_compile_definitions(GLM_FORCE_RADIANS)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules")

include("FileToHeader")

FileToHeader(
    SOURCE_FILE "templates/AssemblyControl.cs"
    HEADER_FILE "include/Templates/AssemblyControlCS.h" 
    VARIABLE_NAME "AssemblyControlTemplateCS"
)
FileToHeader(
    SOURCE_FILE "templates/about.xml"
    HEADER_FILE "include/Templates/About.h"
    VARIABLE_NAME "AboutTemplate"
)

FileToHeader(
    SOURCE_FILE "shaders/grid.vert"
    HEADER_FILE "include/Shaders/GridVertex.h"
    VARIABLE_NAME "GridVertex"
)
FileToHeader(
    SOURCE_FILE "shaders/grid.frag"
    HEADER_FILE "include/Shaders/GridPixel.h"
    VARIABLE_NAME "GridPixel"
)
FileToHeader(
    SOURCE_FILE "shaders/gizmo.vert"
    HEADER_FILE "include/Shaders/GizmoVertex.h"
    VARIABLE_NAME "GizmoVertex"
)
FileToHeader(
    SOURCE_FILE "shaders/gizmo.frag"
    HEADER_FILE "include/Shaders/GizmoPixel.h"
    VARIABLE_NAME "GizmoPixel"
)

if (MINGW)
    include_directories("${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-mono/crossbuild/include/mono-2.0/")
    link_directories("${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-mono/crossbuild/lib/")
else()
    include_directories("${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-mono/build/include/mono-2.0/")
    link_directories("${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-mono/build/lib/")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include/"
                    "${PROJECT_SOURCE_DIR}/../IcarianEngine/FlareBase/include/"
                    "${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-glfw/include/"
                    "${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-glm/"
                    "${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-stb/"
                    "${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-tinyxml2/"
                    "${PROJECT_SOURCE_DIR}/lib/glad/include/"
                    "${PROJECT_SOURCE_DIR}/lib/imgui/"
                    "${PROJECT_SOURCE_DIR}/lib/imgui/backends/"
                    "${PROJECT_SOURCE_DIR}/lib/ImGuizmo/"
                    "${PROJECT_SOURCE_DIR}/lib/implot/")

file(GLOB SOURCES 
    "src/*.cpp" 
    "../IcarianEngine/deps/flare-tinyxml2/tinyxml2.cpp"
    "lib/glad/src/*.c" 
    "lib/imgui/*.cpp" 
    "lib/imgui/backends/imgui_impl_opengl3.cpp" 
    "lib/imgui/backends/imgui_impl_glfw.cpp" 
    "lib/ImGuizmo/ImGuizmo.cpp"
    "lib/implot/implot.cpp" 
    "lib/implot/implot_items.cpp")

if (NOT TARGET GLFW)
    add_subdirectory("${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/flare-glfw" "build/glfw")
endif()
if (NOT TARGET FlareBase)
    add_subdirectory("${PROJECT_SOURCE_DIR}/../IcarianEngine/FlareBase" "build/FlareBase")
endif()

link_directories("${PROJECT_SOURCE_DIR}/../IcarianEngine/deps/mono/build/lib/")

add_executable(IcarianEditor ${SOURCES})
target_link_libraries(IcarianEditor FlareBase glfw ${GLFW_LIBRARIES})

if (WIN32)
    target_link_libraries(IcarianEditor wsock32 ws2_32)

    if (MSVC)
        target_link_libraries(IcarianEditor mono-2.0-sgen)
    else()
        target_link_libraries(IcarianEditor monosgen-2.0)

        target_link_libraries(IcarianEditor -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
    endif()
    set_property(TARGET IcarianEditor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
else()
    target_link_libraries(IcarianEditor mono-2.0)
endif()
