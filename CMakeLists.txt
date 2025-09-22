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

# --- Then link against its deps ---
target_link_libraries(phantom
    ${JNI_LIBRARIES}
    ${JVM_LIBRARY}
    ${JAWT_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
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
