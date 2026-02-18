if (NOT TARGET imgui)
    # https://github.com/ocornut/imgui/archive/refs/tags/v1.89.8.zip
    set(IMGUI_DIR "${EXTERNAL_DIR}/imgui")
    add_library(imgui STATIC
        ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
    )
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    #target_compile_definitions(ImGui PUBLIC IMGUI_IMPL_WEBGPU_BACKEND_DAWN)
    #target_compile_definitions(ImGui PUBLIC IMGUI_IMPL_WEBGPU_BACKEND_WGPU)
    target_link_libraries(imgui PUBLIC webgpu webgpu_dawn glfw)
endif()

if (NOT TARGET ImGuiFileDialog)
    message(STATUS "Fetching ImGuiFileDialog")
    FetchContent_Declare(ImGuiFileDialog
        GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
        SOURCE_DIR "${EXTERNAL_DIR}/ImGuiFileDialog"
    )
    FetchContent_MakeAvailable(ImGuiFileDialog)
    target_link_libraries(ImGuiFileDialog PUBLIC imgui)
endif()
