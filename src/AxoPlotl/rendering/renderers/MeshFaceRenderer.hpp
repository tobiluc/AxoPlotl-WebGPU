#pragma once
#include <AxoPlotl/rendering/renderers/PropertyRendererBase.hpp>
#include "AxoPlotl/rendering/MyGpuContext.hpp"
#include "AxoPlotl/rendering/MeshData.hpp"

namespace AxoPlotl::internal
{

class ColoredFaceRenderer : public PropertyRendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4 mvp_;
        alignas(16) Vec2 viewport_size_;
        alignas(16) ClipBox clip_box_;
        alignas(16) uint32_t object_id_ = 0;
        alignas(16) Vec4 ambient_ = {0.8,0.8,0.8,1};
        alignas(16) float depth_offset_ = 0.00001;
        alignas(16) DirectionalLight dir_light_ ;
        alignas(16) uint32_t flip_normals_ = 0;
        alignas(16) PropFilterOptions filter_options_;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,filter_options_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

public:

    PropFilterOptions& property_filter_options() override {
        return uniforms_.filter_options_;
    }

    ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    Vec4& ambient() {
        return uniforms_.ambient_;
    }

    DirectionalLight& directional_light() {
        return uniforms_.dir_light_;
    }

    uint32_t& flip_normals() {
        return uniforms_.flip_normals_;
    }

    // Value used in the shader as a
    // negative depth bias. A higher value
    // renders the fragment on top.
    float& depth_offset() {
        return uniforms_.depth_offset_;
    }

    ColoredFaceRenderer(uint32_t _max_draw_calls = 1)
        : PropertyRendererBase(_max_draw_calls)
    {}

    ~ColoredFaceRenderer()
    {
        clear();
    }

    void clear() override;

    void init(uint32_t _object_id,
              const GpuContext& _gpu_context,
              const RenderContext& _render_context,
              const VolMeshData& _data);

    void update_property_data(const std::vector<PropContainer>& _data);

    void render(
        const ViewSize& _size,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4& _mvp);

    void configure_like(const ColoredFaceRenderer& _r) {
        uniforms_ = _r.uniforms_;
        enabled_ = _r.enabled_;
    }

    void set_default_uniforms() override {
        uniforms_ = Uniforms();
    };

private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_ = 0;
    size_t n_faces_ = 0;
    size_t n_indices_ = 0;

    void create_buffers();

    void create_bind_group_layout();

    void create_bind_group();

    void create_pipeline(const GpuContext& _gpu_context,
                         const RenderContext& _render_context);

    wgpu::Buffer face_index_buffer_;
    wgpu::Buffer property_buffer_;
    wgpu::Buffer position_buffer_;
    wgpu::Buffer uniform_buffer_;
    wgpu::Buffer halfface_normal_buffer_;

    static PipelineState pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
