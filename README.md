# AxoPlotl (WebGPU)

This is the currently developed WebGPU version of
[AxoPlotl](https://github.com/tobiluc/AxoPlotl).

### Features
- TODO

### Examples
```cpp
#define WEBGPU_CPP_IMPLEMENTATION
#include <AxoPlotl/Application.hpp>

int main()
{
    AxoPlotl::Application app;
    app.init();
  
    auto obj = app.scene().add_mesh("coolmesh.ovmb");
    obj->visualize_property("v_weights");

    app.run(); // blocking
  
    obj = app.scene().add_mesh("anothercoolmesh.obj");
  
    app.run();
    
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
- [ ] Visualization of Halfedges and Halffaces and their properties.
- [x] Visualization of vector properties as lines (see VectorRenderer).
- [ ] Visualization of matrix properties as frames/cubes
- [ ] Warning Popup when input mesh contains no vertices or any position with NaN or INF.

### Bugs
- [ ] When enabling fullscreen, the screen starts flickering before turning pink. (No visible error messages)
- [ ] For some model, when selecting the computed dihedral angle property, the range filter seems posessed by a ghost (could not reproduce).  
- [x] Application crashes when loading a mesh with no vertices (size of position buffer = 0)
- [x] When selecting a face/cell scalar property,
it is visualized as expected, but when
then also enabling vertices/edges rendering
it seems that the face/cell render mode
is MODE_COLOR (Somehow wrong mode in buffer?). In some cases, only
the vertices are rendered instead of both entitites.
- [x] Z-Fighting between Edges/Faces (correct quad vertex order seems to have fixed this too)
- [x] Edges are thinner in the middle (wrong vertex order in quad?)
