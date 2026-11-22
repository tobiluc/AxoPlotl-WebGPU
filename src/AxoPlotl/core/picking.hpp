#pragma once

#include "AxoPlotl/rendering/wgpu_commons.hpp"
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/typedefs/glm.hpp>
#include <webgpu/webgpu.hpp>
#include <cstdint>
#include <iostream>

namespace AxoPlotl::internal
{

// struct PickConfig
// {
// };

// struct PickResult
// {
//     uint32_t object_id_ = 0;

//     uint32_t entity_type_ = 1;
//     uint32_t entity_index_ = 2;

//     float function_value_ = 0;

//     Vec3f position_;
//     Vec3f ndc_;

//     friend inline std::ostream& operator<<(std::ostream& _os, const PickResult& _p) {
//         return _os << _p.object_id_ << "/"<< _p.entity_type_ << "/"<< _p.entity_index_ << "/"
//                    << _p.position_[0] << " "<< _p.position_[1] << " "<< _p.position_[2] ;
//     }
// };

/**
 * Reads from the Viewports Picking Texture
 * to provide a Picking Result at a given position.
 */
class Picker
{
public:
    enum class EntityType : uint32_t {
        VERTEX = 0,
        HALFEDGE = 1,
        HALFFACE = 2,
        CELL = 3,
        VECTOR = 6
    };

    struct Settings {
    } settings;

    struct Pixel
    {
        uint32_t object_id_ = 0;

        EntityType entity_type_;
        uint32_t entity_index_ = 2;

        float function_value_ = 0;

        Vec3 position_;
        Vec3 ndc_;
    };

private:
    wgpu::Buffer buffer_;
    wgpu::Device device_;
    Pixel pixel_;

public:
    Picker() {};

    ~Picker() {
        destroy_buffer(buffer_);
    }

    void init(wgpu::Device _device);

    Picker::Pixel request_pixel(
        const Viewport& _viewport,
        const Camera& _camera,
        float _x, float _y);

    const Picker::Pixel& pixel() const {return pixel_;}
};

}
