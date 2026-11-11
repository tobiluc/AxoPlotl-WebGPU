#include "MyGpuContext.hpp"

namespace AxoPlotl::internal
{

wgpu::TextureFormat RenderContext::depth_format = wgpu::TextureFormat::Depth24Plus;
wgpu::TextureFormat RenderContext::picking_format = wgpu::TextureFormat::RGBA32Uint;

}
