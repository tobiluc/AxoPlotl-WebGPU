cmake_minimum_required(VERSION 3.16)

project(AxoPlotl LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include("${PROJECT_SOURCE_DIR}/cmake/dependencies.cmake")

add_subdirectory(src)

target_include_directories(AxoPlotlCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
)

#target_copy_webgpu_binaries(AxoPlotl-WebGPU)
target_compile_features(AxoPlotlCore PUBLIC cxx_std_20)
