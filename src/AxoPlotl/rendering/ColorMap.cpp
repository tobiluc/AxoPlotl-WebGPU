#include "ColorMap.hpp"
#include "imgui.h"

namespace AxoPlotl
{

static wgpu::Extent3D extent()
{
    return {128, 1, 1};
}
static inline int N = extent().width;

ColorMap::f16x3 ColorMap::sample_color(float _t) const
{
    if (colors_.size()==1) [[unlikely]] {return colors_.front();}

    f16 scaled = _t * (colors_.size() - 1);
    int idx0 = std::min(int(std::floor(scaled)), int(colors_.size() - 2));
    int idx1 = idx0 + 1;
    f16 local_t = scaled - idx0;

    const Vec3f &c0 = colors_.at(idx0);
    const Vec3f &c1 = colors_.at(idx1);

    f16 r = c0.r + local_t * (c1.r - c0.r);
    f16 g = c0.g + local_t * (c1.g - c0.g);
    f16 b = c0.b + local_t * (c1.b - c0.b);

    return {r, g, b};
}

uint32_t ColorMap::sample_color_packed(float _t) const
{
    f16x3 rgb = sample_color(_t);
    return IM_COL32(
        static_cast<int>(rgb[0] * 255.0f),
        static_cast<int>(rgb[1] * 255.0f),
        static_cast<int>(rgb[2] * 255.0f),
        255
    );
}

void ColorMap::set_gradient(const std::vector<f16x3> &_colors)
{
    colors_ = std::move(_colors);

    std::vector<f16> data;
    data.reserve(N * 4);

    for (int i = 0; i < N; ++i)
    {
        f16 t = f16(i) / f16(N - 1);
        f16x3 rgb = sample_color(t);
        data.push_back(rgb[0]);
        data.push_back(rgb[1]);
        data.push_back(rgb[2]);
        data.push_back(1.0f);
    }

    update(data);
}

void ColorMap::set_gradient(const f16x3& _a, const f16x3& _b)
{
    set_gradient({_a, _b});
}

void ColorMap::set_single_color(const f16x3& _color)
{
    set_gradient({_color});
}

void ColorMap::set_viridis()
{
    static const std::vector<f16x3> viridis = {
        {0.267004f, 0.004874f, 0.329415f},
        {0.282327f, 0.140926f, 0.457517f},
        {0.253935f, 0.265254f, 0.529983f},
        {0.206756f, 0.371758f, 0.553117f},
        {0.163625f, 0.471133f, 0.558148f},
        {0.127568f, 0.566949f, 0.550556f},
        {0.134692f, 0.658636f, 0.517649f},
        {0.266941f, 0.748751f, 0.440573f},
        {0.477504f, 0.821444f, 0.318195f},
        {0.741388f, 0.873449f, 0.149561f},
        {0.993248f, 0.906157f, 0.143936f}
    };
    set_gradient(viridis);
    name_ = "Viridis";
}

void ColorMap::set_plasma()
{
    static const std::vector<f16x3> plasma = {
        {0.050383f, 0.029803f, 0.527975f},
        {0.336627f, 0.063956f, 0.769953f},
        {0.648925f, 0.133936f, 0.781554f},
        {0.863001f, 0.386567f, 0.764393f},
        {0.975324f, 0.639465f, 0.519905f},
        {0.993248f, 0.906157f, 0.143936f}
    };
    set_gradient(plasma);
    name_ = "Plasma";
}

void ColorMap::set_magma()
{
    static const std::vector<f16x3> magma = {
        {0.001462f, 0.000466f, 0.013866f},
        {0.267004f, 0.004874f, 0.329415f},
        {0.477504f, 0.064254f, 0.557964f},
        {0.741388f, 0.214372f, 0.497832f},
        {0.965960f, 0.618919f, 0.319167f},
        {0.987053f, 0.991438f, 0.749504f}
    };
    set_gradient(magma);
    name_ = "Magma";
}

void ColorMap::set_rd_bu()
{
    static const std::vector<f16x3> rd_bu = {
        {0.403921f, 0.000000f, 0.121569f},
        {0.698039f, 0.094118f, 0.168627f},
        {0.870588f, 0.619608f, 0.592157f},
        {1.0f, 1.0f, 1.0f},
        {0.619608f, 0.792157f, 0.870588f},
        {0.098039f, 0.396078f, 0.686275f},
        {0.0f, 0.003922f, 0.258824f}
    };
    set_gradient(rd_bu);
    name_ = "RdBu";
}

void ColorMap::set_coolwarm()
{
    static const std::vector<f16x3> coolwarm = {
        {0.229805f, 0.298717f, 0.753683f}, // blue
        {0.541643f, 0.636480f, 0.864786f},
        {0.865003f, 0.865003f, 0.865003f}, // white / center
        {0.991248f, 0.697083f, 0.417556f},
        {0.956871f, 0.211055f, 0.131898f}  // red
    };
    set_gradient(coolwarm);
    name_ = "Coolwarm";
}

void ColorMap::set_rainbow()
{
    static const std::vector<f16x3> rainbow = {
        {1.0f, 0.0f, 0.0f}, // red
        {1.0f, 0.5f, 0.0f}, // orange
        {1.0f, 1.0f, 0.0f}, // yellow
        {0.5f, 1.0f, 0.0f}, // lime green
        {0.0f, 1.0f, 0.0f}, // green
        {0.0f, 1.0f, 0.5f}, // spring green
        {0.0f, 1.0f, 1.0f}, // cyan
        {0.0f, 0.5f, 1.0f}, // azure blue
        {0.0f, 0.0f, 1.0f}, // blue
        {0.5f, 0.0f, 1.0f}, // violet
        {1.0f, 0.0f, 1.0f}, // magenta
        {1.0f, 0.0f, 0.5f} // rose
    };
    set_gradient(rainbow);
    name_ = "Rainbow";
}

void ColorMap::create(wgpu::Device _device)
{
    device_ = _device;
    wgpu::TextureDescriptor desc{};
    desc.label = "Color Map";
    desc.size = extent();
    desc.dimension = wgpu::TextureDimension::_2D;
    desc.format = wgpu::TextureFormat::RGBA16Float;
    desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;
    desc.viewFormatCount = 0;
    desc.viewFormats = nullptr;
    texture_ = device_.createTexture(desc);

    wgpu::TextureViewDescriptor textureViewDesc;
    textureViewDesc.aspect = wgpu::TextureAspect::All;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    textureViewDesc.format = desc.format;
    view_ = texture_.createView(textureViewDesc);

    wgpu::SamplerDescriptor samplerDesc{};
    samplerDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
    samplerDesc.addressModeV = wgpu::AddressMode::ClampToEdge;
    samplerDesc.addressModeW = wgpu::AddressMode::ClampToEdge;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    samplerDesc.magFilter = wgpu::FilterMode::Linear;
    samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
    samplerDesc.maxAnisotropy = 1;
    sampler_ = device_.createSampler(samplerDesc);
}

void ColorMap::update(const std::vector<f16>& _data)
{
    static wgpu::ImageCopyTexture destination;
    destination.texture = texture_;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 };
    destination.aspect = wgpu::TextureAspect::All;

    static wgpu::TextureDataLayout layout_;
    layout_.rowsPerImage = 1lu;
    layout_.bytesPerRow = 4 * sizeof(f16) * extent().width;
    layout_.offset = 0;

    device_.getQueue().writeTexture(
        destination,
        _data.data(),
        sizeof(f16)*_data.size(),
        layout_,
        extent()
    );

    std::cout << "Update Color Map" << std::endl;
}


}
