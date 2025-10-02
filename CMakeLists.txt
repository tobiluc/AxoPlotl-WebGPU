cmake_minimum_required(VERSION 3.16)

project(AxoPlotl-WebGPU LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AxoPlotl-WebGPU main.cpp
    AxoPlotl/Application.h
    AxoPlotl/Application.cpp
    AxoPlotl/utils/web_gpu_utils.h
    AxoPlotl/utils/web_gpu_utils.cpp)

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

target_link_libraries(AxoPlotl-WebGPU PRIVATE webgpu glfw glfw3webgpu)
target_copy_webgpu_binaries(AxoPlotl-WebGPU)
