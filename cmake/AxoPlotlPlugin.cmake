function(add_axoplotl_plugin plugin_src)

    # Extract Plugin Name and create library for it
    get_filename_component(plugin_name ${plugin_src} NAME_WE)
    set(PLUGIN_LIB lib${plugin_name})
    add_library(${PLUGIN_LIB} OBJECT ${plugin_src})

    # Optional arguments: include dirs, link libs
    set(options)
    set(oneValueArgs "")
    set(multiValueArgs INCLUDE_DIRS LINK_LIBS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Additional include directories
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${PLUGIN_LIB} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()

    # Additional Libraries
    if(ARG_LINK_LIBS)
        target_link_libraries(${PLUGIN_LIB} PRIVATE ${ARG_LINK_LIBS})
        # Propagate external dependencies
        target_link_libraries(AxoPlotlApp PRIVATE ${ARG_LINK_LIBS})
    endif()

    # Each Plugin should "see" our core AxoPlotl files
    target_include_directories(${PLUGIN_LIB} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

    # Each Plugin should be linked to certain libraries
    # target_link_libraries(${PLUGIN_LIB} PRIVATE
    #     webgpu
    #     glfw
    #     glfw3webgpu
    #     glm
    #     OpenVolumeMesh::OpenVolumeMesh
    #     nlohmann_json::nlohmann_json
    #     ToLoG
    #     imgui
    #     ImGuiFileDialog
    # )

    # Connect Plugin to Main App and link the Core library to the plugin
    target_sources(AxoPlotlApp PRIVATE $<TARGET_OBJECTS:${PLUGIN_LIB}>)
    target_link_libraries(${PLUGIN_LIB} PRIVATE AxoPlotlCore)

endfunction()

#set(PLUGINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
