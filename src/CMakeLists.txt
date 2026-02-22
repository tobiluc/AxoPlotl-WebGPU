# todo: allow plugins with external dependencies

add_library(AxoPlotlCore
    #todo: actual plugin system
    ../plugins/DataControl/DataControlPlugin.cpp
    ../plugins/Debug/DebugPlugin.cpp

    AxoPlotl/Application.cpp
    AxoPlotl/IO/file_access.cpp
    AxoPlotl/rendering/VolumeMeshRenderer.cpp
    AxoPlotl/Scene.cpp
    AxoPlotl/objects/VolumeMeshObject.cpp
    AxoPlotl/input/Mouse.cpp
    AxoPlotl/Camera.cpp
    AxoPlotl/objects/BaseObject.cpp
    AxoPlotl/rendering/detail/create_static_render_data.cpp
    AxoPlotl/utils/fps.cpp
    AxoPlotl/rendering/detail/redraw.cpp
    AxoPlotl/rendering/ColorMap.cpp
    AxoPlotl/properties/property_calculations.cpp
    AxoPlotl/rendering/shaders/includes/shader_includes.cpp
    AxoPlotl/PluginBase.cpp
)

target_link_libraries(AxoPlotlCore PUBLIC
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

add_executable(AxoPlotlApp
    main.mm
)

target_link_libraries(AxoPlotlApp
    PRIVATE AxoPlotlCore
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
