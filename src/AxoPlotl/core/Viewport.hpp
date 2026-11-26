
#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <webgpu/webgpu.hpp>
#include <imgui.h>
#include <AxoPlotl/utils/ViewSize.hpp>
#include <AxoPlotl/typedefs/glm.hpp>

namespace AxoPlotl::internal
{

class Viewport
{
    friend class Application;
public:
    Viewport(Application* _app);
    ~Viewport();

    Viewport(const Viewport&) = delete;
    Viewport& operator=(const Viewport&) = delete;

    bool draw_ui(wgpu::Device _device,
                wgpu::TextureFormat _color_format);

    void resize(
        wgpu::Device _device,
        wgpu::TextureFormat _color_format,
        const ViewSize& _size);

    const wgpu::TextureView& color_view() const { return resources_.color_view_; }
    const wgpu::TextureView& depth_view() const { return resources_.depth_view_; }
    const wgpu::TextureView& picking_view() const { return resources_.picking_view_; }

    const wgpu::Texture& picking_texture() const { return resources_.picking_texture_; }
    const wgpu::Texture& color_texture() const {return resources_.color_texture_;}

    const wgpu::TextureFormat& color_format() const {return resources_.color_format_;}

    const ViewSize& size() const {return size_;}

    std::optional<glm::vec2> mouse_to_local() const;

    bool valid() const {
        return size_.width > 0 && size_.height > 0 &&
               resources_.color_view_ && resources_.depth_view_ && resources_.picking_view_;
    }

    bool visible() const { return visible_; }
    bool hovered() const { return hovered_; }
    bool focused() const { return focused_; }

    bool right_clicked() const { return right_clicked_; }
    bool left_clicked() const { return left_clicked_; }

    float mouse_x() const { return mouse_x_; }
    float mouse_y() const { return mouse_y_; }

    ImVec2 image_min() const { return image_min_; }

    // Not the same as width/height as it might be scaled
    ImVec2 image_size() const { return image_size_; }

private:
    Application* app_;

    struct Resources
    {
        wgpu::Texture color_texture_;
        wgpu::TextureView color_view_;
        wgpu::TextureFormat color_format_ = wgpu::TextureFormat::Undefined;

        wgpu::Texture depth_texture_;
        wgpu::TextureView depth_view_;

        wgpu::Texture picking_texture_;
        wgpu::TextureView picking_view_;

        void cleanup();

    } resources_;

    ViewSize size_ = {0,0};

    ImVec2 image_min_{};
    ImVec2 image_size_{};

    float mouse_x_ = 0.0f;
    float mouse_y_ = 0.0f;

    bool visible_ = false;
    bool hovered_ = false;
    bool focused_ = false;
    bool right_clicked_ = false;
    bool left_clicked_ = false;

    void cleanup();

    void deferred_cleanup();
};

}
