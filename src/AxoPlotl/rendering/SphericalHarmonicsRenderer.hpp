#pragma once
#include "AxoPlotl/rendering/RendererBase.hpp"

namespace AxoPlotl
{

class SHRenderer : public RendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f value_range_ = {-1,1};
        alignas(16) uint32_t object_id_ = 0;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

    struct Vertex
    {
        Vec3f position_;
        float value_;
    };

    ColorMap color_map_;

public:
    SHRenderer() {}

    ~SHRenderer() {clear();}

    inline ColorMap& color_map() {
        return color_map_;
    }

    void clear() override;

    void init(uint32_t _object_id, Application* _app, size_t _resolution);

    void update(const std::function<float32_t(const Vec3f&)>& _f);

    void render(
        const Vec4f& _viewport,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;
private:
    uint32_t object_id_ = UINT32_MAX;
    size_t resolution_;
    size_t n_indices_;

    void create_buffers();

    void create_bind_group_layout();

    void create_bind_group();

    void create_pipeline();

    std::vector<Vertex> vertices_;

    wgpu::Buffer vertex_buffer_;
    wgpu::Buffer uniform_buffer_;
    wgpu::Buffer index_buffer_;

    static PipelineState pipeline_state_;
    wgpu::BindGroup bind_group_;
};

using SphericalHarmonicsRenderer = SHRenderer;

}
