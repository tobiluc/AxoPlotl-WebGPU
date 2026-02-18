# todo: allow plugins with external dependencies
# file(GLOB PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/AxoPlotl/plugins/*.cpp")
# message(STATUS "Found Plugins: ${PLUGIN_SOURCES}")

add_executable("${PROJECT_NAME}"
    main.cpp
    # ${PLUGIN_SOURCES}
    AxoPlotl/Application.hpp
    AxoPlotl/Application.cpp
    AxoPlotl/utils/web_gpu_utils.h
    AxoPlotl/utils/web_gpu_utils.cpp
    AxoPlotl/IO/file_access.h
    AxoPlotl/IO/file_access.cpp
    AxoPlotl/rendering/TrianglePipeline.h
    AxoPlotl/rendering/TrianglePipeline.cpp
    AxoPlotl/rendering/Renderer.h
    AxoPlotl/rendering/Renderer.cpp
    AxoPlotl/geometry/mesh_adapter.h
    AxoPlotl/geometry/mesh_adapter.cpp
    AxoPlotl/rendering/VolumeMeshRenderer.hpp
    AxoPlotl/typedefs/glm.hpp
    AxoPlotl/rendering/VolumeMeshRenderer.cpp
    AxoPlotl/rendering/shaders/vertex_shader_wgsl.hpp
    AxoPlotl/rendering/shaders/edge_shader_wgsl.hpp
    AxoPlotl/rendering/shaders/Shader.cpp
    AxoPlotl/objects/BaseObject.hpp
    AxoPlotl/Scene.hpp
    AxoPlotl/Scene.cpp
    AxoPlotl/objects/VolumeMeshObject.hpp
    AxoPlotl/objects/VolumeMeshObject.cpp
    AxoPlotl/rendering/shaders/Shader.hpp
    AxoPlotl/typedefs/wgpu.hpp
    AxoPlotl/input/Mouse.hpp
    AxoPlotl/input/Mouse.cpp
    AxoPlotl/Camera.hpp
    AxoPlotl/Camera.cpp
    AxoPlotl/typedefs/ToLoG.hpp
    AxoPlotl/objects/BaseObject.cpp
    AxoPlotl/rendering/shaders/face_shader_wgsl.hpp
    AxoPlotl/rendering/shaders/cell_shader_wgsl.hpp
    AxoPlotl/rendering/detail/create_static_render_data.hpp
    AxoPlotl/rendering/detail/create_static_render_data.cpp
    AxoPlotl/typedefs/ovm.hpp
    AxoPlotl/PluginBase.hpp
)

target_link_libraries("${PROJECT_NAME}" PRIVATE
    webgpu
    glfw
    glfw3webgpu
    glm
    OpenVolumeMesh::OpenVolumeMesh
    nlohmann_json::nlohmann_json
    ToLoG
    imgui
    ImGuiFileDialog
)

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
