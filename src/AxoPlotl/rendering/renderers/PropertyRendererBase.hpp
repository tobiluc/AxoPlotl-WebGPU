#pragma once
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>

namespace AxoPlotl::internal
{

class PropertyRendererBase : public RendererBase
{
protected:
    ColorMap property_color_map_;

public:
    virtual Property::Type& property_type() = 0;

    virtual Property::Filter& property_filter() = 0;

    ColorMap& color_map() {
        return property_color_map_;
    }
};

}
