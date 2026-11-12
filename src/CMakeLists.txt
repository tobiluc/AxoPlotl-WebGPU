# todo: allow plugins with external dependencies

add_library(AxoPlotlCore
    AxoPlotl/geometry/spherical_harmonics.cpp

    AxoPlotl/AxoPlotl.cpp

    AxoPlotl/core/Application.cpp
    AxoPlotl/core/Scene.cpp
    AxoPlotl/core/Viewport.cpp
    AxoPlotl/core/picking.cpp
    AxoPlotl/core/Camera.cpp

    AxoPlotl/core/objects/BaseObject.cpp
    AxoPlotl/core/objects/OpenVolumeMeshObject.cpp
    AxoPlotl/core/objects/SphericalHarmonicsObject.cpp
    AxoPlotl/core/objects/TetrahedronObject.cpp
    AxoPlotl/core/objects/SegmentObject.cpp

    AxoPlotl/input/Mouse.cpp

    AxoPlotl/rendering/create_static_render_data.cpp
    AxoPlotl/rendering/redraw.cpp
    AxoPlotl/gui/widgets/draw_histogram.cpp
    AxoPlotl/rendering/ColorMap.cpp
    AxoPlotl/properties/property_calculations.cpp
    AxoPlotl/properties/Histogram.cpp
    AxoPlotl/gui/themes.cpp

    AxoPlotl/rendering/renderers/MeshVertexRenderer.cpp
    AxoPlotl/rendering/renderers/MeshEdgeRenderer.cpp
    AxoPlotl/rendering/renderers/MeshFaceRenderer.cpp
    AxoPlotl/rendering/renderers/MeshCellRenderer.cpp
    AxoPlotl/rendering/renderers/Vector3Renderer.cpp
    AxoPlotl/rendering/renderers/Matrix3x3Renderer.cpp
    AxoPlotl/rendering/renderers/SphericalHarmonicsRenderer.cpp

    AxoPlotl/gui/widgets/input_slider.cpp
    AxoPlotl/gui/fonts.cpp

    AxoPlotl/gui/widgets/draw_range_sliders.cpp
    AxoPlotl/gui/widgets/draw_texture.cpp
    AxoPlotl/gui/widgets/draw_basic_renderer_settings.cpp

    AxoPlotl/gui/properties/property_settings.cpp
    AxoPlotl/gui/properties/PropertyRendererSettingsBool.cpp
    AxoPlotl/gui/properties/PropertyRendererSettingsVec3.cpp
    AxoPlotl/gui/properties/PropertyRendererSettingsMat3x3.cpp

    AxoPlotl/tools/PlottingTool.cpp
    AxoPlotl/tools/DataControlTool.cpp
    AxoPlotl/tools/DebugTool.cpp

    AxoPlotl/IO/rapidobj_to_ovm.cpp
    AxoPlotl/IO/happly_to_ovm.cpp
    AxoPlotl/IO/om_to_ovm.cpp
    AxoPlotl/IO/read_shader.cpp
    AxoPlotl/IO/file_access.cpp
    AxoPlotl/IO/texture_io.cpp

    AxoPlotl/rendering/MyGpuContext.cpp
    AxoPlotl/rendering/debug/PickingTextureVis.cpp
)

target_link_libraries(AxoPlotlCore PUBLIC
    webgpu
    glfw
    glfw3webgpu
    glm
    imgui
    ImGuiFileDialog
    nlohmann_json::nlohmann_json
    OpenMeshCore
    # OpenMeshTools
    OpenVolumeMesh::OpenVolumeMesh
    rapidobj::rapidobj
    ibex
    ToLoG
)

add_executable(AxoPlotlApp
    main.cpp
)

target_link_libraries(AxoPlotlApp PUBLIC
    AxoPlotlCore   
)

target_include_directories(AxoPlotlCore PUBLIC
    ${iconfontcppheaders_SOURCE_DIR}
    # "${EXTERNAL_DIR}/webgpu-dawn/include"
)
target_compile_definitions(AxoPlotlCore PUBLIC
AXOPLOTL_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts/"
AXOPLOTL_SHADERS_DIR="${CMAKE_SOURCE_DIR}/assets/shaders/"
)

if (XCODE)
    set_target_properties("${PROJECT_NAME}" PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()

if (EMSCRIPTEN)
    set_target_properties(AxoPlotlCore PROPERTIES SUFFIX ".html")
    # Enable the use of emscripten_sleep()
    target_link_options(App PRIVATE -sASYNCIFY)
endif()

include(GNUInstallDirs)
install(TARGETS AxoPlotlCore
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# target_include_directories(glfw3webgpu PUBLIC
#     ${WEBGPU_INCLUDE_DIR}
# )
