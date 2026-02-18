function(add_plugin plugin_src)
    # Create an OBJECT library for the plugin
    get_filename_component(plugin_name ${plugin_src} NAME_WE)
    add_library(${plugin_name}_obj OBJECT ${plugin_src})

    # Optional arguments: include dirs, link libs
    set(options)
    set(oneValueArgs "")
    set(multiValueArgs INCLUDE_DIRS LINK_LIBS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Additional include directories
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${plugin_name}_obj PRIVATE ${ARG_INCLUDE_DIRS})
    endif()

    # Additional Libraries
    if(ARG_LINK_LIBS)
        target_link_libraries(${plugin_name}_obj PRIVATE ${ARG_LINK_LIBS})
    endif()

    # Each Plugin should "see" our core AxoPlotl files
    target_include_directories(${plugin_name}_obj PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>)

    # Each Plugin should be linked to certain libraries
    target_link_libraries(${plugin_name}_obj PRIVATE imgui)

    # Add object files to main executable
    target_sources(${PROJECT_NAME} PRIVATE $<TARGET_OBJECTS:${plugin_name}_obj>)
endfunction()

set(PLUGINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
