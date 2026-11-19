#pragma once
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>

namespace AxoPlotl::internal
{

class PropertyRendererBase : public RendererBase
{
protected:
    ColorMap property_color_map_;

public:
    enum class PropValueMode : uint32_t
    {
        COLOR = 0u,
        SCALAR = 1u,
        VEC3 = 2u,
        MAT3x3 = 3u
    };
    enum class PropFilterMode : uint32_t
    {
        NONE = 0u,
        INSIDE_RANGE = 1u,
        IS_NAN = 2u,
        IS_INF = 3u
    };
    static_assert(sizeof(PropValueMode) == 4);
    static_assert(sizeof(PropFilterMode) == 4);
    struct PropFilterOptions
    {
        alignas(16) PropValueMode value_mode = PropValueMode::COLOR;
        alignas(16) PropFilterMode filter_mode = PropFilterMode::NONE;
        alignas(16) uint32_t invert = 0;
        alignas(16) Vec2 scalar_range = {-1,1};
        alignas(16) glm::vec<2,uint32_t> scalar_incl = {1,1};

        void set_default() {
            *this = PropFilterOptions();
        }
    };
    static_assert(sizeof(PropFilterOptions) == 80);
    static_assert(offsetof(PropFilterOptions, value_mode) == 0);
    static_assert(offsetof(PropFilterOptions, filter_mode) == 16);
    static_assert(offsetof(PropFilterOptions, invert) == 32);
    static_assert(offsetof(PropFilterOptions, scalar_range) == 48);
    static_assert(offsetof(PropFilterOptions, scalar_incl) == 64);
    static_assert(alignof(PropFilterOptions) == 16);

    struct Prop {
        // enum class Mode : uint32_t {
        //     COLOR = 0u,
        //     SCALAR = 1u,
        //     VEC3 = 2u,
        //     MAT3x3 = 3u
        // };
        // struct Filter {
        //     Vec2 range_ = {0,0};
        // };
        using Data = Vec4;
    };

    PropertyRendererBase(uint32_t _max_draw_calls = 1)
        : RendererBase(_max_draw_calls)
    {}

    virtual PropFilterOptions& property_filter_options() = 0;

    ColorMap& color_map() {
        return property_color_map_;
    }
};

}
