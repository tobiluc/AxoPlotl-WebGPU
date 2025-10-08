cmake_minimum_required(VERSION 3.16)

project(AxoPlotl-WebGPU LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AxoPlotl-WebGPU main.cpp
    AxoPlotl/Application.h
    AxoPlotl/Application.cpp
    AxoPlotl/utils/web_gpu_utils.h
    AxoPlotl/utils/web_gpu_utils.cpp
    AxoPlotl/IO/file_access.h
    AxoPlotl/IO/file_access.cpp
    AxoPlotl/TriangleRenderer.h AxoPlotl/TriangleRenderer.cpp
    AxoPlotl/rendering/Shader.h
    AxoPlotl/rendering/Shader.cpp


    AxoPlotl/rendering/Pipeline.h AxoPlotl/rendering/Pipeline.cpp
    AxoPlotl/rendering/Buffer.h
    AxoPlotl/rendering/Buffer.cpp
    AxoPlotl/rendering/Renderer.h
    AxoPlotl/rendering/Renderer.cpp
)

if (NOT EMSCRIPTEN)
    add_subdirectory(glfw)
else()
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

add_subdirectory(webgpu-dawn)

add_subdirectory(glfw3webgpu)

set_target_properties(AxoPlotl-WebGPU PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR OFF
)

if (XCODE)
    set_target_properties(AxoPlotl-WebGPU PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()

if (EMSCRIPTEN)
    set_target_properties(AxoPlotl-WebGPU PROPERTIES SUFFIX ".html")
    # Enable the use of emscripten_sleep()
    target_link_options(App PRIVATE -sASYNCIFY)
endif()

include(GNUInstallDirs)
install(TARGETS AxoPlotl-WebGPU
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#=========================
# Init Fetch Content
#=========================
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

#=========================
# GLM
#=========================
if (NOT TARGET glm)
    message(STATUS "Fetching glm")
    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
        SOURCE_DIR "${EXTERNAL_DIR}/glm"
    )
    FetchContent_MakeAvailable(glm)
endif()

target_link_libraries(AxoPlotl-WebGPU PRIVATE webgpu glfw glfw3webgpu glm)
target_copy_webgpu_binaries(AxoPlotl-WebGPU)
