#pragma once

#include "AxoPlotl/rendering/ColorMap.hpp"

namespace AxoPlotl
{

class Application;

inline constexpr void destroy_buffer(wgpu::Buffer& _buffer) {
    if (_buffer) {
        _buffer.destroy();
        _buffer.release();
        _buffer = nullptr;
    }
}

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

class PropertyRendererBase
{
public:
    using Position = Vec4f; // ensure 16 byte alignment with 4f vector

    struct alignas(16) ClipBox {
        alignas(16) Vec3f min_;
        alignas(16) Vec3f max_;
        alignas(16) int32_t enabled_ = false;

        inline void set(const Vec3f& _min, const Vec3f& _max) {
            enabled_ = true;
            min_ = _min;
            max_ = _max;
        }
    };
    static_assert(sizeof(ClipBox)==3*16);

    virtual void clear() = 0;

protected:
    using Pad4 = uint32_t[1];
    using Pad8 = uint32_t[2];
    using Pad12 = uint32_t[3];

    bool enabled_ = true;
    Application* app_;

    static_assert(sizeof(ClipBox)%16==0);

public:
    struct Property
    {
        enum class Type : uint32_t {
            COLOR = 0u,
            SCALAR = 1u,
            VEC3 = 2u,
        };
        using Data = Vec4f;
        struct Filter {
            Vec2f range_ = {0,0};
        };
    };

    virtual void render(
        const Vec4f& _viewport,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) = 0;

    inline bool& enabled() {
        return enabled_;
    }
};
}
