# AxoPlotl (WebGPU)

This is the currently developed WebGPU version of
[AxoPlotl](https://github.com/tobiluc/AxoPlotl).

### Features
- TODO

### Examples
```cpp
#define WEBGPU_CPP_IMPLEMENTATION
#include <AxoPlotl/AxoPlotl.hpp>

int main()
{
    // Always call init() first!
    if (AxoPlotl::app().init()) {
        std::cerr << "ERROR: Failed to initialize AxoPlotl" << std::endl;
        std::exit(1);
    }
    
    // You can add custom UI code via the inspector callback.
    // This is shown in a side panel
    // There are some predefined "tools" that should be used here
    // like the DataContolTool
    AxoPlotl::app().inspector_callback() = {[&]() {
        ImGui::Text("Hello World");
        AxoPlotl::DataControlTool::render_ui();
    }};
  
    // Add an OpenVolumeMesh from a file
    // The scene holds an object collection that is automatically
    // rendered. In this case, we want to visualize an existing property
    auto obj = AxoPlotl::app().scene().add_mesh("coolmesh.ovmb");
    obj->visualize_property("v_weights");

    // Show the application window. While running, you can still add new meshes, visualize properties,
    // and more. If the window is closed, the program continues and the app is reset can be used again.
    AxoPlotl::app().run();
    
    // Visualize another mesh in a new window
    // This time we only show the scene and no inspector.
    // Note that this and other UI settings can also be set in the window
    // under Settings -> UI
    AxoPlotl::app().inspectore_enabled() = false;
  
    obj = AxoPlotl::app().scene().add_mesh("anothercoolmesh.obj");
  
    AxoPlotl::app().run();
    
    return 0;
}
```

### Wishlist
- [x] Picking: Render scene to a picking texture. Each pixel of the texture stores [object id, entity type, entity index,  ?]. Then we can click somewhere to for example show all properties of the clicked entity, zoom closer to it, etc. 
- [x] Histogram: Handle special case when all properties are either NaN or Inf
- [x] Histogram: Handle special case when all properties have same values. In general #distinct values should be >= #buckets. Also, make buckets a partition i.e. intervals [a,b) instead of [a,b].
- [x] Histogram: Handle NaN & Infinity values separately.
- [ ] Transparency. Allow rendering of meshes with transparency to better inspect them.
For example, for property visualization, colors would be in RGBA to make cell
more transparent, the larger/smaller a property value is.
- [x] Visualization of Halfedges and Halffaces and their properties.
- [x] Visualization of vector properties as lines (see VectorRenderer).
- [x] Visualization of matrix properties as frames/cubes
- [x] Highlight selected entity
- [ ] Warning Popup when input mesh contains no vertices or any position with NaN or INF.
- [ ] Handle BIIIGGG Meshes where normally we exceed the webgpu buffer limit.

### Bugs
- [x] When enabling fullscreen, the screen starts flickering before turning pink. (No visible error messages) => Solved by updating dawn (see below)
- [x] Application crashes when loading a mesh with no vertices (size of position buffer = 0)
- [x] When selecting a face/cell scalar property,
it is visualized as expected, but when
then also enabling vertices/edges rendering
it seems that the face/cell render mode
is MODE_COLOR (Somehow wrong mode in buffer?). In some cases, only
the vertices are rendered instead of both entitites.
- [x] Z-Fighting between Edges/Faces (correct quad vertex order seems to have fixed this too)
- [x] Edges are thinner in the middle (wrong vertex order in quad?)
- [ ] Z-Fighting between Faces/Cells for large scale meshes - precision error?


### Backend Version Update

Since updating to a newer version of xcode-select (command line tools), there have been some issues.
The Apple clang update from 15.0.0 (clang-1500.1.0.2.5) to 17.0.0 (clang-1700.0.13.5)
lead to an error in the dawn distribution
caused by some const pointer in std::allocator which is not allowed.
To fix it I needed to update dawn (6536) to (7069). 
Additionally, there are a few manual changes necessary
in the fetched dependencies listed below.
The original version of AxoPlotl for apple clang 15 is found under the clang15 tag.

1. Replace the file "external/imgui/backends/imgui_impl_webgpu.cpp" with "files/imgui_impl_webgpu.cpp".
2. (seems no longer necessary) Replace the file "external/glfw3webgpu/glfw3webgpu.c" with "files/glfw3webgpu.c".


These modified files are within AxoPlotl/files.
(18.04.2026)


### Building

MacOS
```
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

.vscode/settings.json
```
{
    "clangd.path": "/opt/homebrew/opt/llvm/bin/clangd",
    "editor.inlayHints.enabled": "off",

    "cmake.buildDirectory": "${workspaceFolder}/build/debug",
    "cmake.buildType": "Debug",
    "cmake.generator": "Ninja",

    "cmake.debugConfig": {
        "args": []
    }
}
```
