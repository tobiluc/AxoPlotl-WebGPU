#pragma once
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>

namespace AxoPlotl::internal
{

class PropertyRendererBase : public RendererBase
{
protected:
    ColorMap property_color_map_;

public:
    struct Prop {
        enum class Mode : uint32_t {
            COLOR = 0u,
            SCALAR = 1u,
            VEC3 = 2u,
            MAT3x3 = 3u
        };
        struct Filter {
            Vec2f range_ = {0,0};
        };
        using Data = Vec4f;
    };

    PropertyRendererBase(uint32_t _max_draw_calls = 1)
        : RendererBase(_max_draw_calls)
    {}

    virtual Prop::Mode& property_mode() = 0;

    virtual Prop::Filter& property_filter() = 0;

    ColorMap& color_map() {
        return property_color_map_;
    }
};

}
