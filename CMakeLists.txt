cmake_minimum_required(VERSION 3.16)

project(AxoPlotl-WebGPU LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/plugins")

include(dependencies)

add_subdirectory(src)

include(plugin_helper)
include(Plugins)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
)

#target_copy_webgpu_binaries(AxoPlotl-WebGPU)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
