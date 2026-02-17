#pragma once

#include "AxoPlotl/objects/BaseObject.hpp"

namespace AxoPlotl
{

class VolumeMeshObject : public BaseObject
{
public:
    VolumeMeshObject(VolumeMeshRenderer::Context _render_context, int _id) :
        BaseObject(_render_context, _id, "Volume Mesh Object")
    {
        VolumeMeshRenderer::StaticData data;
        data.positions_ = {
            Vec4f(0,0,0,1),Vec4f(1,0,0,1),Vec4f(0,1,0,1),Vec4f(0,0,1,1)
        };
        data.vertex_draw_indices_ = {0,1,2,3};
        data.edge_draw_indices_.push_back({.vertex_index_=0,.edge_index_=0});
        data.edge_draw_indices_.push_back({.vertex_index_=1,.edge_index_=0});
        data.edge_draw_indices_.push_back({.vertex_index_=0,.edge_index_=1});
        data.edge_draw_indices_.push_back({.vertex_index_=2,.edge_index_=1});
        data.edge_draw_indices_.push_back({.vertex_index_=0,.edge_index_=2});
        data.edge_draw_indices_.push_back({.vertex_index_=3,.edge_index_=2});
        renderer_.init(data);
    }
};

}
