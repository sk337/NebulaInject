cmake_minimum_required(VERSION 3.18)
project(phantom LANGUAGES C CXX)

# Allow the user to override PHANTOM_SOURCE_FILES externally (e.g. -DSOURCES=...)
if (NOT DEFINED PHANTOM_SOURCE_FILES)
    file(GLOB_RECURSE PHANTOM_SOURCE_FILES src/*.*)
endif()

set(IMGUI_SOURCE_FILES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
    vendor/imgui/imgui_impl_sdl.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
)

# === find deps ===
find_package(JNI REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
# Find SDL2: support both FindSDL2 and pkg-config variants
find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(SDL2_PKG REQUIRED sdl2)
        set(SDL2_FOUND TRUE)
        set(SDL2_INCLUDE_DIRS ${SDL2_PKG_INCLUDE_DIRS})
        set(SDL2_LIBRARIES ${SDL2_PKG_LIBRARIES})
    endif()
endif()

# Fallback: if find_package(SDL2) found target, provide include/lib names
if (TARGET SDL2::SDL2)
    set(SDL2_TARGET SDL2::SDL2)
endif()

# Try to find jawt/jvm libraries (use JAVA_HOME if available)
if (DEFINED ENV{JAVA_HOME} AND NOT DEFINED JAVA_HOME)
    set(JAVA_HOME $ENV{JAVA_HOME})
endif()
if (DEFINED JAVA_HOME)
    find_library(JAWT_LIBRARY NAMES jawt PATHS
        ${JAVA_HOME}/lib
        ${JAVA_HOME}/lib/amd64
        ${JAVA_HOME}/lib/server
        NO_DEFAULT_PATH)
    find_library(JVM_LIBRARY NAMES jvm PATHS
        ${JAVA_HOME}/lib
        ${JAVA_HOME}/lib/amd64
        ${JAVA_HOME}/lib/server
        NO_DEFAULT_PATH)
endif()
# If not found above, try system find
if (NOT JAWT_LIBRARY)
    find_library(JAWT_LIBRARY NAMES jawt)
endif()
if (NOT JVM_LIBRARY)
    find_library(JVM_LIBRARY NAMES jvm)
endif()

# === Create target ===
add_library(phantom SHARED ${PHANTOM_SOURCE_FILES} ${IMGUI_SOURCE_FILES})


set_target_properties(phantom PROPERTIES
    BUILD_RPATH "${JAVA_HOME}/jre/lib/amd64:${JAVA_HOME}/jre/lib/amd64/xawt"
    INSTALL_RPATH "${JAVA_HOME}/jre/lib/amd64:${JAVA_HOME}/jre/lib/amd64/xawt"
)

# === Includes ===
target_include_directories(phantom PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}    # may be empty on some systems
    ${X11_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/src/mapping/impl
)

# If SDL2 target exists, link to it; otherwise use SDL2_LIBRARIES variable
if (SDL2_TARGET)
    target_link_libraries(phantom PRIVATE ${SDL2_TARGET})
elseif (SDL2_LIBRARIES)
    target_link_libraries(phantom PRIVATE ${SDL2_LIBRARIES})
else()
    message(WARNING "SDL2 was not found by CMake. Your target may be missing SDL2 symbols.")
endif()

# Link OpenGL. Prefer modern imported target if available.
if (TARGET OpenGL::GL)
    target_link_libraries(phantom PRIVATE OpenGL::GL)
elseif (OPENGL_gl_LIBRARY)
    target_link_libraries(phantom PRIVATE ${OPENGL_gl_LIBRARY})
elseif (OPENGL_LIBRARIES)
    target_link_libraries(phantom PRIVATE ${OPENGL_LIBRARIES})
else()
    message(WARNING "OpenGL libraries not found.")
endif()

# Link common libs
target_link_libraries(phantom PRIVATE
    Threads::Threads
    ${CMAKE_DL_LIBS}
    ${X11_LIBRARIES}
    ${JNI_LIBRARIES}
)

# Link jawt/jvm if found
if (JAWT_LIBRARY)
    target_link_libraries(phantom PRIVATE ${JAWT_LIBRARY})
endif()
if (JVM_LIBRARY)
    target_link_libraries(phantom PRIVATE ${JVM_LIBRARY})
endif()

# Ensure PIC and C++ standard
set_target_properties(phantom PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
)

# === Injector CLI ===
add_executable(injector-cli
    injector-cli/main.cpp
    injector-cli/injector/elf.c
    injector-cli/injector/injector.c
    injector-cli/injector/ptrace.c
    injector-cli/injector/remote_call.c
    injector-cli/injector/util.c
)
# Link injector-cli minimal deps (pthread)
target_link_libraries(injector-cli PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

# === Helpful configure-time info ===
message(STATUS "PHANTOM_SOURCE_FILES: ${PHANTOM_SOURCE_FILES}")
message(STATUS "SDL2 include dirs: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
message(STATUS "OpenGL gl lib: ${OPENGL_gl_LIBRARY}")
message(STATUS "X11 include dir: ${X11_INCLUDE_DIR}")
message(STATUS "JNI include dirs: ${JNI_INCLUDE_DIRS}")
message(STATUS "JAWT library: ${JAWT_LIBRARY}")
message(STATUS "JVM library: ${JVM_LIBRARY}")
