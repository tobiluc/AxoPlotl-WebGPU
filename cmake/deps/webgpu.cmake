if (NOT EMSCRIPTEN)
    set(GLFW_BUILD_SHARED_LIBS ON)
    add_subdirectory("${EXTERNAL_DIR}/glfw")
else()
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

FetchContent_Declare(webgpu-dawn
    GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution.git
    GIT_TAG dawn-7069
    SOURCE_DIR "${EXTERNAL_DIR}/webgpu-dawn"
)
FetchContent_MakeAvailable(webgpu-dawn)

# Create a "dummy" alias to satisfy hardcoded dependencies
if(TARGET webgpu AND NOT TARGET webgpu_dawn)
    add_library(webgpu_dawn INTERFACE)
    target_link_libraries(webgpu_dawn INTERFACE webgpu)
endif()

# https://developer.chrome.com/blog/new-in-webgpu-130?hl=de
# glfw3webgpu has some old names. change in glfw3webgpu.c:
# WGPUSurfaceDescriptorFromMetalLayer ->	WGPUSurfaceSourceMetalLayer
# WGPUSType_SurfaceDescriptorFromMetalLayer -> WGPUSType_SurfaceSourceMetalLayer
# L90: surfaceDescriptor.label = NULL; -> surfaceDescriptor.label = (WGPUStringView){0};

# Change in imgui/backends/imgui_impl_webgpu.cpp
# Remove using WGPUProgrammableStageDescriptor = WGPUComputeState;
# Replace with
# #ifndef WGPUComputeState
# typedef WGPUProgrammableStageDescriptor WGPUComputeState;
# #endif
# Replace
# { nullptr, WGPUVertexFormat_Float32x2, (uint64_t)offsetof(ImDrawVert, pos), 0 },
#         { nullptr, WGPUVertexFormat_Float32x2, (uint64_t)offsetof(ImDrawVert, uv),  1 },
#         { nullptr, WGPUVertexFormat_Unorm8x4,  (uint64_t)offsetof(ImDrawVert, col), 2 },
# with
# { .format = WGPUVertexFormat_Float32x2, .offset = (uint64_t)offsetof(ImDrawVert, pos), .shaderLocation = 0 },
# { .format = WGPUVertexFormat_Float32x2, .offset = (uint64_t)offsetof(ImDrawVert, uv), .shaderLocation = 1 },
# { .format = WGPUVertexFormat_Float32x2, .offset = (uint64_t)offsetof(ImDrawVert, col), .shaderLocation = 2 },
# Line 998: Comment out (// case WGPUDeviceLostReason_CallbackCancelled: return "CallbackCancelled";)

add_subdirectory("${EXTERNAL_DIR}/glfw3webgpu")
