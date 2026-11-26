#pragma once

#include <algorithm>
#include <AxoPlotl/typedefs/glm.hpp>

namespace AxoPlotl
{
struct ViewSize
{
    uint32_t width = 1;
    uint32_t height = 1;
    float aspect_ratio() const {return static_cast<float>(width)/static_cast<float>(height);}
    ViewSize operator*(float _scale) const {
        return {
            .width=static_cast<uint32_t>(_scale*width),
            .height=static_cast<uint32_t>(_scale*height)
        };
    }
    bool operator==(const ViewSize& _size) const {
        return width==_size.width && height==_size.height;
    }
    ViewSize clamped(const ViewSize& _min = {1,1}, const ViewSize& _max = {UINT32_MAX,UINT32_MAX}) const {
        return ViewSize{
            .width = std::clamp(width, _min.width, _max.width),
            .height = std::clamp(height, _min.height, _max.height)
        };
    }
    void clamp(const ViewSize& _min = {1,1}, const ViewSize& _max = {UINT32_MAX,UINT32_MAX}) {
        *this = clamped(_min, _max);
    }
    float fwidth() const {return static_cast<float>(width);}
    float fheight() const {return static_cast<float>(height);}
    Vec2 to_vec2() const {return {fwidth(), fheight()};}
    void from_vec2(const Vec2& _v) {
        width = static_cast<uint32_t>(std::max(_v[0],1.0f));
        height = static_cast<uint32_t>(std::max(_v[1],1.0f));
    }
};
}