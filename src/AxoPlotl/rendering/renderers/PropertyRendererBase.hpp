#pragma once
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>
#include <AxoPlotl/rendering/ShaderMirrors.hpp>

namespace AxoPlotl::internal
{

class PropertyRendererBase : public RendererBase
{
protected:
    ColorMap property_color_map_;

public:

    PropertyRendererBase(uint32_t _max_draw_calls = 1)
        : RendererBase(_max_draw_calls)
    {}

    virtual PropFilterOptions& property_filter_options() = 0;

    ColorMap& color_map() {
        return property_color_map_;
    }
};

}
