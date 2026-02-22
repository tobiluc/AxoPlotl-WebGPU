#pragma once

#include "AxoPlotl/rendering/ColorMap.hpp"

namespace AxoPlotl
{

class Application;

class RendererBase
{
public:
    using Position = Vec4f;

    struct alignas(16) ClipBox {
        alignas(16) Vec3f min_;
        alignas(16) Vec3f max_;
        alignas(16) int32_t enabled_ = false;
    };

protected:
    using Pad4 = uint32_t[1];
    using Pad8 = uint32_t[2];
    using Pad12 = uint32_t[3];


    bool enabled_ = true;
    ColorMap property_color_map_;
    Application* app_;

    static_assert(sizeof(ClipBox)%16==0);

public:
    struct Property
    {
        enum class Mode : uint32_t {
            COLOR = 0u,
            SCALAR = 1u,
            VEC3 = 2u,
        };
        struct Data {
            Vec4f value_ = {0,0,0,1};
        };
        struct Filter {
            Vec2f range_ = {0,0};
        };
    };

    virtual ClipBox& clip_box() = 0;

    virtual void render(
        const Vec4f& _viewport,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) = 0;

    inline bool& enabled() {
        return enabled_;
    }
};
}
