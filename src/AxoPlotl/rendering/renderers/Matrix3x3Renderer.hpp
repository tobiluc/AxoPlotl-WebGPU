#pragma once
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>
#include "AxoPlotl/rendering/MyGpuContext.hpp"

namespace AxoPlotl::internal
{

class Matrix3x3Renderer : public RendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f viewport_size_;
        alignas(16) float line_width_ = 8.0f;
        alignas(16) ClipBox clip_box_;
        alignas(16) float vec_scale_ = 1.0f;
        alignas(16) uint32_t object_id_ = 0;
        alignas(16) uint32_t draw_symmetric_ = 1;
        alignas(16) uint32_t normalize = 0;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,vec_scale_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,line_width_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

public:
    float& line_width() {
        return uniforms_.line_width_;
    };

    float& vector_scale() {
        return uniforms_.vec_scale_;
    };

    uint32_t& draw_symmetric() {
        return uniforms_.draw_symmetric_;
    };

    uint32_t& normalize() {
        return uniforms_.normalize;
    };

    ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    Matrix3x3Renderer() {}

    ~Matrix3x3Renderer() {clear();}

    void clear() override;

    void init(uint32_t _object_id,
              const GpuContext& _gpu_context,
              const RenderContext& _render_context,
              wgpu::Buffer _position_buffer);

    void update_matrix_data(const std::vector<glm::mat3x4> &_data);

    void render(
        const ViewSize& _size,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;

    inline bool is_initialized() const {
        return initialized_;
    }

    void configure_like(const Matrix3x3Renderer& _r) {
        uniforms_ = _r.uniforms_;
        enabled_ = _r.enabled_;
    }

private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_ = 0;
    bool initialized_ = false;

    void create_buffers();

    void create_bind_group_layout();

    void create_bind_group();

    void create_pipeline(const GpuContext& _gpu_context,
                         const RenderContext& _render_context);

    wgpu::Buffer matrix_buffer_;
    wgpu::Buffer position_buffer_;
    wgpu::Buffer uniform_buffer_;

    static PipelineState pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
