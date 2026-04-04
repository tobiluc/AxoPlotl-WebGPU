# todo: allow plugins with external dependencies

add_library(AxoPlotlCore
    AxoPlotl/Application.cpp
    AxoPlotl/IO/file_access.cpp

    AxoPlotl/Scene.cpp
    AxoPlotl/objects/OpenVolumeMeshObject.cpp
    AxoPlotl/input/Mouse.cpp
    AxoPlotl/Camera.cpp
    AxoPlotl/objects/BaseObject.cpp
    AxoPlotl/rendering/detail/create_static_render_data.cpp
    AxoPlotl/rendering/detail/redraw.cpp
    AxoPlotl/rendering/ColorMap.cpp
    AxoPlotl/properties/property_calculations.cpp
    AxoPlotl/gui/themes.cpp
    AxoPlotl/rendering/MeshVertexRenderer.cpp
    AxoPlotl/rendering/MeshEdgeRenderer.cpp
    AxoPlotl/rendering/MeshFaceRenderer.cpp
    AxoPlotl/rendering/MeshCellRenderer.cpp
    AxoPlotl/rendering/VectorRenderer.cpp
    AxoPlotl/gui/fonts.cpp

    AxoPlotl/tools/DataControlTool.cpp
    AxoPlotl/tools/DebugTool.cpp
    AxoPlotl/IO/rapidobj_to_ovm.cpp
    AxoPlotl/IO/happly_to_ovm.cpp
    AxoPlotl/IO/om_to_ovm.cpp
    AxoPlotl/IO/read_shader.cpp
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
    OpenMeshTools
    OpenVolumeMesh::OpenVolumeMesh
    rapidobj::rapidobj
)

add_executable(AxoPlotlApp
    main.mm
    AxoPlotl/tools/PlottingTool.cpp
)

target_link_libraries(AxoPlotlApp PUBLIC
    AxoPlotlCore
    ibex
)

target_include_directories(AxoPlotlCore PUBLIC
    ${iconfontcppheaders_SOURCE_DIR}
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
