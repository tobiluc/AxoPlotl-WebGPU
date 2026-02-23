# AxoPlotl (WebGPU)

This is the currently developed WebGPU version of
[AxoPlotl](https://github.com/tobiluc/AxoPlotl).


### Wishlist
- [ ] Plugin System. A user should be able to easily write a Plugin
extending from PluginBase. A custom plugin should be able
to have external dependencies and an instance of the plugin should
automatically be added to some PluginRegistry when the application
starts. Currently, we have to "hack" plugin files into the main CMakeLists which
should not be touched. This current system does not allow for external dependencies
on a plugin level.
- [ ] Transparency. Allow rendering of meshes with transparency to better inspect them.
For example, for property visualization, colors would be in RGBA to make cell
more transparent, the larger/smaller a property value is.
- [ ] Visualization of Halfedges and Halffaces and their properties.
- [ ] Visualization of vector properties as lines.
- [ ] Visualization of matrix properties as frames/cubes
- [ ] Scalar Filters: Color Maps should be initialized on some on_select function.

### Bugs
- [x] When selecting a face/cell scalar property,
it is visualized as expected, but when
then also enabling vertices/edges rendering
it seems that the face/cell render mode
is MODE_COLOR (Somehow wrong mode in buffer?). In some cases, only
the vertices are rendered instead of both entitites.
- [ ] Z-Fighting between Edges/Faces. Edges should be "prioritised" when depth is equal.
