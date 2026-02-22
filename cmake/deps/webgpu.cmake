if (NOT EMSCRIPTEN)
    set(GLFW_BUILD_SHARED_LIBS ON)
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
