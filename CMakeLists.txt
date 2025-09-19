cmake_minimum_required(VERSION 3.18)
project(phantom)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS_RELEASE -O3)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra")

set(IMGUI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)

# Dependencies
find_package(JNI REQUIRED)
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

# Explicitly find JVM + JAWT
find_library(JVM_LIBRARY jvm
    HINTS ${JAVA_HOME}/lib/server)

find_library(JAWT_LIBRARY jawt
    HINTS ${JAVA_HOME}/lib)

include_directories(
    ${JNI_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${X11_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    src/mapping/impl
)

if (NOT DEFINED SOURCES)
    file(GLOB_RECURSE PHANTOM_SOURCE_FILES src/*.*)
endif()

source_group("Source Files" FILES ${PHANTOM_SOURCE_FILES})

set(IMGUI_SOURCE_FILES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
    vendor/imgui/imgui_impl_sdl.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
)

# --- Define target FIRST ---
add_library(phantom SHARED ${IMGUI_SOURCE_FILES} ${PHANTOM_SOURCE_FILES})

# --- Then link against its deps ---
target_link_libraries(phantom
    ${JNI_LIBRARIES}
    ${JVM_LIBRARY}
    ${JAWT_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${SDL2_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${OPENGL_gl_LIBRARY}
    OpenGL::GL
    ${X11_LIBRARIES}
)
