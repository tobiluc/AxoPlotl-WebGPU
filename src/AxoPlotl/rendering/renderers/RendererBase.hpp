#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <webgpu/webgpu.hpp>
#include "AxoPlotl/typedefs/glm.hpp"
#include <AxoPlotl/rendering/ColorMap.hpp>

namespace AxoPlotl::internal
{

class Application;

struct PipelineState
{
    wgpu::Device device_;
    wgpu::RenderPipeline pipeline_;
    wgpu::BindGroupLayout bind_group_layout_;

    inline void set_device(wgpu::Device _device) {
        if (device_ != _device) {
            device_ = _device;
            if (pipeline_) {pipeline_.release(); pipeline_ = nullptr;}
            if (bind_group_layout_) {bind_group_layout_.release(); bind_group_layout_ = nullptr;}
        }
    }
};

class RendererBase
{
public:
    using Position = Vec4f; // ensure 16 byte alignment with 4f vector

    struct alignas(16) ClipBox {
        alignas(16) Vec3f min_;
        alignas(16) Vec3f max_;
        alignas(16) int32_t enabled_ = false;

        // void set(const Vec3f& _min, const Vec3f& _max) {
        //     enabled_ = true;
        //     min_ = _min;
        //     max_ = _max;
        // }
    };
    static_assert(sizeof(ClipBox)==3*16);
    static_assert(sizeof(ClipBox)%16==0);

    RendererBase(uint32_t _max_draw_calls = 1)
        : max_draw_calls_(std::max(_max_draw_calls, 1u))
    {}

    virtual void clear() = 0;

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

    virtual void render(
        const ViewSize& _size,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) = 0;

    bool& enabled() {return enabled_;}
};
}
