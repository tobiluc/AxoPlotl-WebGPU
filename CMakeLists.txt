cmake_minimum_required(VERSION 3.16)

project(AxoPlotl-WebGPU LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

add_executable("${PROJECT_NAME}"
    main.cpp
    src/AxoPlotl/Application.hpp
    src/AxoPlotl/Application.cpp
    src/AxoPlotl/utils/web_gpu_utils.h
    src/AxoPlotl/utils/web_gpu_utils.cpp
    src/AxoPlotl/IO/file_access.h
    src/AxoPlotl/IO/file_access.cpp
    src/AxoPlotl/TriangleRenderer.h
    src/AxoPlotl/TriangleRenderer.cpp


    src/AxoPlotl/rendering/TrianglePipeline.h
    src/AxoPlotl/rendering/TrianglePipeline.cpp
    src/AxoPlotl/rendering/Buffer.h
    src/AxoPlotl/rendering/Buffer.cpp
    src/AxoPlotl/rendering/Renderer.h
    src/AxoPlotl/rendering/Renderer.cpp
    src/AxoPlotl/geometry/mesh_adapter.h
    src/AxoPlotl/geometry/mesh_adapter.cpp
    src/AxoPlotl/rendering/VolumeMeshRenderer.hpp
    src/AxoPlotl/typedefs/glm.hpp
    src/AxoPlotl/rendering/VolumeMeshRenderer.cpp
    src/AxoPlotl/rendering/shaders/vertex_shader_wgsl.hpp
    src/AxoPlotl/rendering/shaders/edge_shader_wgsl.hpp

    src/AxoPlotl/rendering/shaders/Shader.cpp
    src/AxoPlotl/objects/BaseObject.hpp
    src/AxoPlotl/Scene.hpp
    src/AxoPlotl/Scene.cpp
    src/AxoPlotl/objects/VolumeMeshObject.hpp
    src/AxoPlotl/objects/VolumeMeshObject.cpp
    src/AxoPlotl/rendering/shaders/Shader.hpp
    src/AxoPlotl/typedefs/wgpu.hpp
)

#=========================
# Init Fetch Content
#=========================
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

#=========================
# WebGPU related
#=========================

if (NOT EMSCRIPTEN)
    add_subdirectory("${EXTERNAL_DIR}/glfw")
else()
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

add_subdirectory("${EXTERNAL_DIR}/webgpu-dawn")

add_subdirectory("${EXTERNAL_DIR}/glfw3webgpu")

# set_target_properties("${PROJECT_NAME}" PROPERTIES
#     CXX_STANDARD 20
#     CXX_STANDARD_REQUIRED ON
#     CXX_EXTENSIONS OFF
#     COMPILE_WARNING_AS_ERROR OFF
# )

if (XCODE)
    set_target_properties("${PROJECT_NAME}" PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()

if (EMSCRIPTEN)
    set_target_properties("${PROJECT_NAME}" PROPERTIES SUFFIX ".html")
    # Enable the use of emscripten_sleep()
    target_link_options(App PRIVATE -sASYNCIFY)
endif()

include(GNUInstallDirs)
install(TARGETS "${PROJECT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

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

#=========================
# OVM
#=========================
if (NOT TARGET OpenVolumeMesh::OpenVolumeMesh)
    message(STATUS "Fetching OpenVolumeMesh")
    FetchContent_Declare(ovm
        GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenVolumeMesh/OpenVolumeMesh
        GIT_TAG master
        SOURCE_DIR "${EXTERNAL_DIR}/OpenVolumeMesh"
        )
    FetchContent_MakeAvailable(ovm)
endif()

#=========================
# NLohmann Json
#=========================
if (NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

#=========================
# ToLoG
#=========================
if (NOT TARGET ToLoG)
    message(STATUS "Fetching ToLoG")
    FetchContent_Declare(ToLoG
        GIT_REPOSITORY https://github.com/tobiluc/ToLoG.git
        GIT_TAG main
        SOURCE_DIR "${EXTERNAL_DIR}/ToLoG"
    )
    set(TOLOG_BUILD_UNIT_TESTS FALSE)
    FetchContent_MakeAvailable(ToLoG)
endif()

#=========================
# Define what to include
#=========================
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
)

#=========================
# Link Libraries
#=========================
target_link_libraries("${PROJECT_NAME}" PRIVATE
    webgpu
    glfw
    glfw3webgpu
    glm
    OpenVolumeMesh::OpenVolumeMesh
    nlohmann_json::nlohmann_json
    ToLoG
)
target_copy_webgpu_binaries(AxoPlotl-WebGPU)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
