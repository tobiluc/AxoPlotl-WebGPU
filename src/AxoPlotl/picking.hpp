#pragma once

#include <webgpu/webgpu.hpp>
#include <cstdint>
#include <iostream>

namespace AxoPlotl
{

struct PickConfig
{
    bool enable_vertex_picking_ = true;
    bool enable_edge_picking_ = true;
    bool enable_face_picking_ = true;
    bool enable_cell_picking_ = true;
};

struct PickResult
{
    uint32_t object_id_ = 0;
    uint32_t type_ = 1;
    uint32_t index_ = 2;
    uint32_t p3 = 3;
    friend inline std::ostream& operator<<(std::ostream& _os, const PickResult& _p) {
        return _os << _p.object_id_ << "/"<< _p.type_ << "/"<< _p.index_ << "/"<< _p.p3;
    }
};

}
