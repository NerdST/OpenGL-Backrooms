cmake_minimum_required(VERSION 3.10)
project(Project1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Libraries
add_library(glad STATIC include/glad/src/glad.c)
target_include_directories(glad PUBLIC include/glad/include)

# ImGui library
add_library(imgui STATIC
    include/imgui/imgui.cpp
    include/imgui/imgui_demo.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/backends/imgui_impl_glfw.cpp
    include/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC include/imgui include/imgui/backends)
target_link_libraries(imgui PUBLIC glad ${GLFW_LIBRARIES})
target_include_directories(imgui PUBLIC ${GLFW_INCLUDE_DIRS})

# Executable
add_executable(Project1 
    src/main.cpp 
    src/stb_image.cpp
    src/Mesh.cpp
    src/TextureManager.cpp
    src/Primitives.cpp
    src/MazeGenerator.cpp
    src/FrustumCuller.cpp
    src/OcclusionCuller.cpp
)
target_include_directories(Project1 PRIVATE include)

# Dependencies
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# Link libraries
target_link_libraries(Project1 PRIVATE 
    glad 
    imgui
    OpenGL::GL 
    ${GLFW_LIBRARIES}
    $<$<PLATFORM_ID:Linux>:dl>
)

target_include_directories(Project1 PRIVATE ${GLFW_INCLUDE_DIRS})

# Copy data files
add_custom_command(TARGET Project1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data
)

# Set working directory for debuggers
set_target_properties(Project1 PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Compiler warnings
target_compile_options(Project1 PRIVATE 
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

