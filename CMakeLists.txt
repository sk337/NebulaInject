
cmake_minimum_required(VERSION 3.18)
project(phantom LANGUAGES C CXX)


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
add_library(phantom SHARED ${PHANTOM_SOURCE_FILES} ${IMGUI_SOURCE_FILES})

# --- Add include dirs (don’t force /SDL2 manually) ---
target_include_directories(phantom PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}   # ✅ just this is enough
    ${OPENGL_INCLUDE_DIR}
    ${X11_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    src/mapping/impl
)

# --- Then link against its deps ---
target_link_libraries(phantom
    ${JNI_LIBRARIES}
    ${JVM_LIBRARY}
    ${JAWT_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
    ${SDL2_LIBRARIES}      # ✅ make sure SDL2 is linked
    ${X11_LIBRARIES}
)

add_executable(injector-cli
    injector-cli/main.cpp
    injector-cli/injector/elf.c
    injector-cli/injector/injector.c
    injector-cli/injector/ptrace.c
    injector-cli/injector/remote_call.c
    injector-cli/injector/util.c
)
