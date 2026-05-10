#pragma once

#include <AxoPlotl/typedefs/glm.hpp>
#include <webgpu/webgpu.hpp>
#include <cstdint>
#include <iostream>

namespace AxoPlotl
{

struct PickConfig
{
};

struct PickResult
{
    uint32_t object_id_ = 0;

    uint32_t entity_type_ = 1;
    uint32_t entity_index_ = 2;

    float function_value_ = 0;

    Vec3f position_;

    friend inline std::ostream& operator<<(std::ostream& _os, const PickResult& _p) {
        return _os << _p.object_id_ << "/"<< _p.entity_type_ << "/"<< _p.entity_index_ << "/"
                   << _p.position_[0] << " "<< _p.position_[1] << " "<< _p.position_[2] ;
    }
};

}
