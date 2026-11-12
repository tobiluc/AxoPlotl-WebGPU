#pragma once
#include <AxoPlotl/rendering/renderers/PropertyRendererBase.hpp>
#include "AxoPlotl/core/Light.hpp"
#include "AxoPlotl/rendering/MyGpuContext.hpp"

namespace AxoPlotl::internal
{

class ColoredFaceRenderer : public PropertyRendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f viewport_size_;
        alignas(16) ClipBox clip_box_;
        alignas(16) Property::Type type_ = Property::Type::COLOR;
        alignas(16) Property::Filter value_filter_;
        alignas(16) uint32_t object_id_ = 0;
        alignas(16) Vec4f ambient_ = Vec4f(0.8,0.8,0.8,1);
        alignas(16) float depth_offset_ = 0.00001;
        alignas(16) Vec3f light_dir_ = {-1,-1,-1};
        alignas(16) DirectionalLight dir_light_ ;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,type_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

public:

    Property::Type& property_type() override {
        return uniforms_.type_;
    }

    Property::Filter& property_filter() override {
        return uniforms_.value_filter_;
    }

    ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    Vec4f& ambient() {
        return uniforms_.ambient_;
    }

    DirectionalLight& directional_light() {
        return uniforms_.dir_light_;
    }

    // Value used in the shader as a
    // negative depth bias. A higher value
    // renders the fragment on top.
    float& depth_offset() {
        return uniforms_.depth_offset_;
    }

    ColoredFaceRenderer() {}

    ~ColoredFaceRenderer()
    {
        clear();
    }

    void clear() override;

    void init(uint32_t _object_id,
              const GpuContext& _gpu_context,
              const RenderContext& _render_context,
              wgpu::Buffer _position_buffer,
              const std::vector<std::vector<uint32_t>>& _faces,
              const std::vector<glm::vec4>& _face_normals);

    void update_property_data(const std::vector<Property::Data>& _data);

    void render(
        const ViewSize& _size,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;

    void configure_like(const ColoredFaceRenderer& _r) {
        uniforms_ = _r.uniforms_;
        enabled_ = _r.enabled_;
    }
private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_ = 0;
    size_t n_faces_ = 0;
    size_t n_indices_ = 0;

    void create_buffers(
        const std::vector<std::vector<uint32_t>>& _faces,
        const std::vector<glm::vec4> &_face_normals);

    void create_bind_group_layout();

    void create_bind_group();

    void create_pipeline(const GpuContext& _gpu_context,
                         const RenderContext& _render_context);

    wgpu::Buffer face_index_buffer_;
    wgpu::Buffer property_buffer_;
    wgpu::Buffer position_buffer_;
    wgpu::Buffer uniform_buffer_;
    wgpu::Buffer normal_buffer_;

    static PipelineState pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
