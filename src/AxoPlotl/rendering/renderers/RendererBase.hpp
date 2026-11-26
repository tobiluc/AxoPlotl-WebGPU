#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <webgpu/webgpu.hpp>
#include "AxoPlotl/typedefs/glm.hpp"
#include <AxoPlotl/rendering/ColorMap.hpp>
#include <AxoPlotl/core/picking.hpp>
#include <AxoPlotl/utils/ViewSize.hpp>

namespace AxoPlotl::internal
{

struct PipelineState
{
    wgpu::Device device_;
    wgpu::RenderPipeline pipeline_;
    wgpu::BindGroupLayout bind_group_layout_;

    inline void set_device(wgpu::Device _device) {
        if (device_ != _device) {
            device_ = _device;
            save_release(pipeline_);
            save_release(bind_group_layout_);
        }
    }
};

class RendererBase
{
public:
    using Position = Vec4; // ensure 16 byte alignment with 4f vector

    RendererBase(uint32_t _max_draw_calls = 1)
        : max_draw_calls_(std::max(_max_draw_calls, 1u))
    {}

    virtual void cleanup() = 0;

protected:
    using Pad4 = uint32_t[1];
    using Pad8 = uint32_t[2];
    using Pad12 = uint32_t[3];

    bool enabled_ = true;
    bool initialized_ = false;

    /// You can use a Renderer several times
    /// per frame (with different Uniforms)
    /// by modyfying max_draw_calls_. uniform_stride
    /// must be initialized in init.
    /// Do not exceed max draw calls (i.e. call render on
    /// the same Renderer more
    /// than max_draw_calls many times in the same frame tick)!
    uint32_t uniform_stride_ = 0;
    const uint32_t max_draw_calls_;
    uint32_t draw_index_ = 0;

    constexpr void set_next_draw_index() {
        draw_index_ = (draw_index_ + 1u) % max_draw_calls_;
    }

    template<typename UniformT>
    void compute_uniform_stride(const wgpu::Device& _device) {
        wgpu::Limits limits;
        _device.getLimits(&limits);
        const uint32_t alignment = limits.minUniformBufferOffsetAlignment;
        uniform_stride_ = (sizeof(UniformT) + alignment - 1) / alignment * alignment;
    }

public:
    constexpr void reset_draw_index() {
        draw_index_ = 0u;
    }

public:

    bool& enabled() {return enabled_;}

    virtual void set_default_uniforms() = 0;
};
}
