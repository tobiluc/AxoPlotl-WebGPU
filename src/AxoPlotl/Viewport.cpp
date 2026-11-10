
#include "Viewport.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace AxoPlotl
{

Viewport::~Viewport()
{
    release();
}

void Viewport::release()
{
    // Release views before their parent textures.
    if (color_view_) {
        color_view_.release();
    }

    if (depth_view_) {
        depth_view_.release();
    }

    if (picking_view_) {
        picking_view_.release();
    }

    if (color_texture_) {
        color_texture_.release();
    }

    if (depth_texture_) {
        depth_texture_.release();
    }

    if (picking_texture_) {
        picking_texture_.release();
    }

    width_ = 0;
    height_ = 0;
}

std::optional<glm::vec2> Viewport::mouse_to_local() const
{
    const ImVec2 mouse = ImGui::GetMousePos();

    if (image_size_.x > 0.0f && image_size_.y > 0.0f) {
        // Mouse position relative to the displayed viewport image.
        const float local_x = mouse.x - image_min_.x;
        const float local_y = mouse.y - image_min_.y;

        // Convert from ImGui coordinates to texture pixel coordinates.
        const float scale_x = width() / image_size_.x;
        const float scale_y = height() / image_size_.y;

        const float x = local_x * scale_x;
        const float y = local_y * scale_y;

        // Check bounds
        if (x >= 0.0f && y >= 0.0f && x < width() && y < height()) {
            return glm::vec2(x, y);
        }
    }
    return std::nullopt;
}

void Viewport::resize(
    wgpu::Device& device,
    uint32_t width,
    uint32_t height,
    wgpu::TextureFormat color_format,
    wgpu::TextureFormat depth_format)
{
    width = std::max(width, 1u);
    height = std::max(height, 1u);

    if (width == width_ && height == height_ && valid()) {
        return;
    }

    release();

    width_ = width;
    height_ = height;

    //----------------------------------
    // Color texture
    //----------------------------------

    wgpu::TextureDescriptor color_desc{};

    color_desc.label = wgpu::StringView("Viewport Color");
    color_desc.dimension = wgpu::TextureDimension::_2D;
    color_desc.size = {width, height, 1};
    color_desc.format = color_format;
    color_desc.mipLevelCount = 1;
    color_desc.sampleCount = 1;

    color_desc.usage =
        wgpu::TextureUsage::RenderAttachment |
        wgpu::TextureUsage::TextureBinding;

    color_texture_ = device.createTexture(color_desc);

    wgpu::TextureViewDescriptor color_view_desc{};

    color_view_desc.format = color_format;
    color_view_desc.dimension = wgpu::TextureViewDimension::_2D;
    color_view_desc.baseMipLevel = 0;
    color_view_desc.mipLevelCount = 1;
    color_view_desc.baseArrayLayer = 0;
    color_view_desc.arrayLayerCount = 1;
    color_view_desc.aspect = wgpu::TextureAspect::All;

    color_view_ = color_texture_.createView(color_view_desc);

    //----------------------------------
    // Depth texture
    //----------------------------------

    wgpu::TextureDescriptor depth_desc{};

    depth_desc.label = wgpu::StringView("Viewport Depth");
    depth_desc.dimension = wgpu::TextureDimension::_2D;
    depth_desc.size = {width, height, 1};
    depth_desc.format = depth_format;
    depth_desc.mipLevelCount = 1;
    depth_desc.sampleCount = 1;
    depth_desc.usage = wgpu::TextureUsage::RenderAttachment;

    depth_texture_ = device.createTexture(depth_desc);

    wgpu::TextureViewDescriptor depth_view_desc{};

    depth_view_desc.format = depth_format;
    depth_view_desc.dimension = wgpu::TextureViewDimension::_2D;
    depth_view_desc.baseMipLevel = 0;
    depth_view_desc.mipLevelCount = 1;
    depth_view_desc.baseArrayLayer = 0;
    depth_view_desc.arrayLayerCount = 1;
    depth_view_desc.aspect = wgpu::TextureAspect::DepthOnly;

    depth_view_ = depth_texture_.createView(depth_view_desc);

    // ----------------------------------
    // Picking texture
    // ----------------------------------

    wgpu::TextureDescriptor picking_desc{};

    picking_desc.label = wgpu::StringView("Viewport Picking");
    picking_desc.dimension = wgpu::TextureDimension::_2D;
    picking_desc.size = {width, height, 1};
    picking_desc.format = wgpu::TextureFormat::RGBA32Uint;
    picking_desc.mipLevelCount = 1;
    picking_desc.sampleCount = 1;

    // TextureBinding is used if we want to render the texture
    // using draw_texture
    picking_desc.usage =
        wgpu::TextureUsage::RenderAttachment |
        wgpu::TextureUsage::CopySrc |
        wgpu::TextureUsage::TextureBinding;

    picking_texture_ = device.createTexture(picking_desc);
    picking_view_ = picking_texture_.createView();
}

bool Viewport::render_ui(
    wgpu::Device& device,
    wgpu::TextureFormat color_format,
    wgpu::TextureFormat depth_format)
{
    visible_ = false;
    hovered_ = false;
    focused_ = false;
    right_clicked_ = false;
    left_clicked_ = false;

    if (!ImGui::Begin("Viewport")) {
        ImGui::End();
        return false;
    }

    const ImVec2 available = ImGui::GetContentRegionAvail();

    if (available.x <= 1.0f || available.y <= 1.0f) {
        ImGui::End();
        return false;
    }

    // Convert ImGui coordinates to framebuffer pixels.
    const ImGuiIO& io = ImGui::GetIO();

    const uint32_t width = static_cast<uint32_t>(
        std::round(available.x * io.DisplayFramebufferScale.x));

    const uint32_t height = static_cast<uint32_t>(
        std::round(available.y * io.DisplayFramebufferScale.y));

    // Resize before displaying the texture.
    resize(device, width, height, color_format, depth_format);

    // Dawn C++ handle -> raw WebGPU handle.
    const WGPUTextureView raw_view =
        static_cast<WGPUTextureView>(color_view_);

    const ImTextureID texture_id =
        (ImTextureID)(intptr_t)raw_view;

    ImGui::Image(
        ImTextureRef(texture_id),
        available,
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 1.0f)
        );

    image_min_ = ImGui::GetItemRectMin();
    image_size_ = ImGui::GetItemRectSize();

    hovered_ = ImGui::IsItemHovered();
    focused_ = ImGui::IsWindowFocused();

    right_clicked_ =
        hovered_ &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right);

    left_clicked_ =
        hovered_ &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    if (hovered_) {
        const ImVec2 mouse = io.MousePos;

        mouse_x_ =
            (mouse.x - image_min_.x) *
            static_cast<float>(width_) / image_size_.x;

        mouse_y_ =
            (mouse.y - image_min_.y) *
            static_cast<float>(height_) / image_size_.y;
    }

    visible_ = true;

    ImGui::End();

    return true;
}

}
