include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

include(deps/OpenVolumeMesh)
include(deps/glm)
include(deps/nlohmann_json)
include(deps/ToLoG)
include(deps/webgpu)
include(deps/imgui)

# add_plugin()
# add_plugin_with_deps(
#     plugins/MyFancyPlugin.cpp
#     INCLUDE_DIRS ${GLM_INCLUDE_DIRS}  # headers
#     LINK_LIBS imgui glfw              # libraries needed
# )
