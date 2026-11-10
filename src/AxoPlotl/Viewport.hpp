
#pragma once

#include "AxoPlotl/rendering/draw_texture.hpp"
#include <webgpu/webgpu.hpp>
#include <imgui.h>
#include <cstdint>
#include <AxoPlotl/typedefs/glm.hpp>

namespace AxoPlotl
{

class Viewport
{
public:
    Viewport() = default;
    ~Viewport();

    Viewport(const Viewport&) = delete;
    Viewport& operator=(const Viewport&) = delete;

    bool render_ui(
        wgpu::Device& device,
        wgpu::TextureFormat color_format,
        wgpu::TextureFormat depth_format
        );

    void resize(
        wgpu::Device& device,
        uint32_t width,
        uint32_t height,
        wgpu::TextureFormat color_format,
        wgpu::TextureFormat depth_format
        );

    void release();

    wgpu::TextureView& color_view() { return color_view_; }
    wgpu::TextureView& depth_view() { return depth_view_; }
    const wgpu::TextureView& picking_view() const { return picking_view_; }

    wgpu::Texture& picking_texture() { return picking_texture_; }

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }

    ViewSize size() const {
        return {.width = static_cast<float>(width_),
                .height = static_cast<float>(height_)};}

    std::optional<glm::vec2> mouse_to_local() const;

    bool valid() const {
        return width_ > 0 && height_ > 0 &&
               color_view_ && depth_view_ && picking_view_;
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
    wgpu::Texture color_texture_;
    wgpu::TextureView color_view_;

    wgpu::Texture depth_texture_;
    wgpu::TextureView depth_view_;

    wgpu::Texture picking_texture_;
    wgpu::TextureView picking_view_;

    uint32_t width_ = 0;
    uint32_t height_ = 0;

    ImVec2 image_min_{};
    ImVec2 image_size_{};

    float mouse_x_ = 0.0f;
    float mouse_y_ = 0.0f;

    bool visible_ = false;
    bool hovered_ = false;
    bool focused_ = false;
    bool right_clicked_ = false;
    bool left_clicked_ = false;
};

}
