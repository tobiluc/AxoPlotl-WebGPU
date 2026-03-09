# AxoPlotl (WebGPU)

This is the currently developed WebGPU version of
[AxoPlotl](https://github.com/tobiluc/AxoPlotl).


### Wishlist
- [ ] Transparency. Allow rendering of meshes with transparency to better inspect them.
For example, for property visualization, colors would be in RGBA to make cell
more transparent, the larger/smaller a property value is.
- [ ] Visualization of Halfedges and Halffaces and their properties.
- [ ] Visualization of vector properties as lines.
- [ ] Visualization of matrix properties as frames/cubes

### Bugs
- [ ] For some model, when selecting the computed dihedral angle property, the range filter seems posedded by a ghost.  
- [x] When selecting a face/cell scalar property,
it is visualized as expected, but when
then also enabling vertices/edges rendering
it seems that the face/cell render mode
is MODE_COLOR (Somehow wrong mode in buffer?). In some cases, only
the vertices are rendered instead of both entitites.
- [ ] Z-Fighting between Edges/Faces. Edges should be "prioritised" when depth is equal.
