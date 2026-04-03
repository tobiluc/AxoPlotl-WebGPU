#pragma once
#include "AxoPlotl/rendering/RendererBase.hpp"

namespace AxoPlotl
{

class VectorRenderer : public PropertyRendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f viewport_size_;
        alignas(16) float line_width_ = 8.0f;
        alignas(16) ClipBox clip_box_;
        alignas(16) float vec_scale_ = 0.05f;
        alignas(16) uint32_t object_id_ = 0;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,vec_scale_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,line_width_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

public:
    inline float& line_width() {
        return uniforms_.line_width_;
    };

    inline float& vector_scale() {
        return uniforms_.vec_scale_;
    };

    inline ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    VectorRenderer() {}

    ~VectorRenderer()
    {
        vector_buffer_.destroy();
        vector_buffer_.release();
        uniform_buffer_.destroy();
        uniform_buffer_.release();
    }

    void init(uint32_t _object_id, Application* _app, wgpu::Buffer _position_buffer);

    void update_vector_data(const std::vector<Vec4f>& _data);

    void render(
        const Vec4f& _viewport,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;
private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_;

    void create_buffers();

    void create_bind_group_layout();

    void create_bind_group();

    void create_pipeline();

    wgpu::Buffer vector_buffer_;
    wgpu::Buffer position_buffer_;
    wgpu::Buffer uniform_buffer_;

    static PipelineState pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
