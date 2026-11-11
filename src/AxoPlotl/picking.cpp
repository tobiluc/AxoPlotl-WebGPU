#include "picking.hpp"
#include "AxoPlotl/Camera.hpp"
#include <AxoPlotl/Viewport.hpp>

namespace AxoPlotl::internal
{

void Picker::init(wgpu::Device _device)
{
    device_ = _device;

    wgpu::BufferDescriptor pickBuffDesc{};
    pickBuffDesc.label = wgpu::StringView("Pixel Picking Buffer");
    pickBuffDesc.size = sizeof(Picker::Pixel);
    pickBuffDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    pickBuffDesc.mappedAtCreation = false;
    buffer_ = device_.createBuffer(pickBuffDesc);
}

Picker::Pixel Picker::request_pixel(
    const Viewport& _viewport,
    const Camera &_camera,
    float _x, float _y)
{
    wgpu::CommandEncoder encoder = device_.createCommandEncoder();

    // We copy FROM the picking texture
    wgpu::TexelCopyTextureInfo src{};
    src.texture = _viewport.picking_texture();// picking_texture_;
    src.origin = {static_cast<uint32_t>(_x), static_cast<uint32_t>(_y), 0};
    src.mipLevel = 0;
    src.aspect = wgpu::TextureAspect::All;

    // We copy TO the picking buffer
    wgpu::TexelCopyBufferInfo dst{};
    dst.buffer = buffer_;
    dst.layout.offset = 0;
    // this needs to be a multiple of 256
    // even if we care only about a 1x1x1x1 pixel
    dst.layout.bytesPerRow = 256;
    dst.layout.rowsPerImage = 1;

    // Copy the single pixel we clicked on (1 pixel is 1x1x1)
    wgpu::Extent3D copySize = {1,1,1};
    encoder.copyTextureToBuffer(src, dst, copySize);

    wgpu::CommandBuffer cb = encoder.finish();
    device_.getQueue().submit(1, &cb);
    cb.release();
    encoder.release();

    struct Context {
        bool ready;
        wgpu::Buffer buffer;
        Picker::Pixel pick_pixel;
        const Camera* camera;
        ViewSize scene_size;
    };

    auto on_buffer_mapped = [](
                                WGPUMapAsyncStatus status,
                                WGPUStringView message,
                                void* _user_data, void*)
    {
        Context* context = reinterpret_cast<Context*>(_user_data);
        context->ready = true;
        //std::cout << "Buffer mapped with status " << status << std::endl;
        if (status != WGPUMapAsyncStatus::WGPUMapAsyncStatus_Success) {return;}

        // Extract Info from Picking Result
        // and Map clicked position back to world space

        uint32_t* data = (uint32_t*)context->buffer.getConstMappedRange(0, sizeof(Picker::Pixel));

        // Index 0 stores the Object ID, so we know which
        // Objects we clicked. The corresponding object's
        // render_picking_ui function is called.
        context->pick_pixel.object_id_ = data[0];

        // For Meshes, we store the entity type and handle index
        // (Type 0:Vertex, 1:Edge, 2:Face, 3:Cell)
        context->pick_pixel.entity_type_ = data[1];
        context->pick_pixel.entity_index_ = data[2];

        // Entity Index (u32, for Meshes) and Function Value (f32, for Functions)
        // are both stored at index 2 of the pixel
        // TODO: Maybe we want different PickResult Objects instead?
        context->pick_pixel.function_value_ = std::bit_cast<glm::float32_t>(data[2]);

        // At Index 3 we store the fragment's depth value
        // Based on this, we can reconstruct the 3d position
        // in world space
        // (normalized devie coords (ndc) x and y value, we already
        // have, it's the normalized viewport position)
        float depth = std::bit_cast<float>(data[3]);
        context->pick_pixel.ndc_.z = depth;
        const auto& v = context->camera->get_view_matrix();
        const auto& p = context->camera->get_projection_matrix(context->scene_size);
        Vec4f pos = glm::inverse(p * v) * Vec4f(context->pick_pixel.ndc_, 1);
        pos /= pos.w;
        context->pick_pixel.position_[0] = pos[0];
        context->pick_pixel.position_[1] = pos[1];
        context->pick_pixel.position_[2] = pos[2];

        // unmap the memory
        context->buffer.unmap();
    };

    // The context contains everyhting that is required
    // for our picking computation in on_buffer_mapped.
    // Is it passed as userdata1
    Context context;
    context.ready = false;
    context.buffer = buffer_;
    context.camera = &_camera;
    context.scene_size = _viewport.size();

    // Get x and y components of Normalized Device Coordinates
    _x -= 0; // topleft of vieport is origin since docking rework
    _y -= 0;
    context.pick_pixel.ndc_ = {
        std::clamp((2.0*_x)/context.scene_size.width - 1.0, -1.0, 1.0),
        std::clamp(1.0-(2.0*_y)/context.scene_size.height, -1.0, 1.0),
        0
    };

    WGPUBufferMapCallbackInfo callback_info;
    callback_info.callback = on_buffer_mapped;
    callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
    callback_info.userdata1 = (void*)&context;
    callback_info.userdata2 = nullptr;
    callback_info.nextInChain = nullptr;

    wgpuBufferMapAsync(
        buffer_,
        wgpu::MapMode::Read,
        0, sizeof(Picker::Pixel),
        callback_info
        );
    while (!context.ready) {
        device_.poll(false, nullptr);
    }

    this->pixel_ = context.pick_pixel;
    return pixel_;
}

}
