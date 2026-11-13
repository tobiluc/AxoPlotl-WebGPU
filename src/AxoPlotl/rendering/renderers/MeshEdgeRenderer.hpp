#pragma once
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include "AxoPlotl/rendering/ColorMap.hpp"
#include <AxoPlotl/rendering/renderers/PropertyRendererBase.hpp>
#include "AxoPlotl/rendering/MyGpuContext.hpp"

namespace AxoPlotl::internal
{

class ColoredEdgeRenderer : public PropertyRendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f viewport_size_;
        alignas(16) float line_width_ = 8.0f;
        alignas(16) ClipBox clip_box_;
        alignas(16) Prop::Mode type_ = Prop::Mode::COLOR;;
        alignas(16) Prop::Filter value_filter_;
        alignas(16) uint32_t object_id_ = 0;
        alignas(16) Vec4f ambient_ = Vec4f(0.6,0.6,0.6,1);
        alignas(16) float depth_offset_ = 0.00002;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,type_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,line_width_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

public:
    float& line_width() {
        return uniforms_.line_width_;
    };

    Prop::Mode& property_mode() override {
        return uniforms_.type_;
    }

    Prop::Filter& property_filter() override {
        return uniforms_.value_filter_;
    }

    ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    Vec4f& ambient() {
        return uniforms_.ambient_;
    }

    // Value used in the shader as a
    // negative depth bias. A higher value
    // renders the fragment on top.
    float& depth_offset() {
        return uniforms_.depth_offset_;
    }

    ColoredEdgeRenderer(uint32_t _max_draw_calls = 1)
        : PropertyRendererBase(_max_draw_calls)
    {}

    ~ColoredEdgeRenderer()
    {
        clear();
    }

    void clear() override;

    struct EdgeInstance {
        uint32_t vh0_; // used to lookup position
        uint32_t vh1_; // used to lookup position
        uint32_t eh_; // points to EdgePropertyData
    };

    void init(uint32_t _object_id,
              const GpuContext& _gpu_context,
              const RenderContext& _render_context,
              wgpu::Buffer _position_buffer,
              const std::vector<std::pair<uint32_t,uint32_t>>& _edges);

    void update_property_data(const std::vector<Prop::Data>& _data);

    void render(
        const ViewSize& _size,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;

    void configure_like(const ColoredEdgeRenderer& _r) {
        uniforms_ = _r.uniforms_;
        enabled_ = _r.enabled_;
    }

private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_ = 0;
    size_t n_edges_ = 0;

    void create_buffers(const std::vector<std::pair<uint32_t,uint32_t>>& _edges);

    void create_bind_group_layout();

    void create_bind_group();

    void create_pipeline(const GpuContext& _gpu_context,
                         const RenderContext& _render_context);

    wgpu::Buffer edge_index_buffer_;
    wgpu::Buffer property_buffer_;
    wgpu::Buffer position_buffer_;
    wgpu::Buffer uniform_buffer_;

    static PipelineState pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
