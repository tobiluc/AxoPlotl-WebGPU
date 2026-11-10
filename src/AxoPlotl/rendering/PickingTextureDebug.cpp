// #include "PickingTextureDebug.hpp"

// namespace AxoPlotl
// {

// void PickingTextureDebug::init(wgpu::Device _device)
// {
//     renderer_.init(_device, 256, 256);
// }

// void PickingTextureDebug::resize(uint32_t _width, uint32_t _height)
// {
//     renderer_.resize(_width, _height);
// }

// void PickingTextureDebug::render(wgpu::CommandEncoder& _encoder,
//         wgpu::TextureView)
// {
//     // wgpu::CommandEncoder encoder = AxoPlotl::scene().app()->device_.createCommandEncoder();
//     renderer_.render(_encoder, view_, AxoPlotl::PickingTextureRenderer::Mode::EntityType);
//     // wgpu::CommandBuffer cb = encoder.finish();
//     // AxoPlotl::scene().app()->device_.getQueue().submit(1, &cb);
//     // cb.release();
//     // encoder.release();
// }

// void PickingTextureDebug::show()
// {
//     draw_texture("PickingTexture", renderer_.get_texture_view());
// }

// }
