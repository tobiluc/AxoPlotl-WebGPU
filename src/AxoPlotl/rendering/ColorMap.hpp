#pragma once

#include "webgpu/webgpu.hpp"
#include <AxoPlotl/typedefs/glm.hpp>
#include <string>
#include <vector>
#include <arm_neon.h>

namespace AxoPlotl
{

class ColorMap {
public:
    using f16 = float16_t;

    ColorMap() {}

    ~ColorMap() {
        destroy();
    }

    void create(wgpu::Device _device);

    inline void destroy() {
        if (texture_) {
            texture_.destroy();
            texture_.release();
            view_.release();
            sampler_.release();
        }
    }

    void set_gradient(const std::vector<Vec3f>& _colors);

    void set_gradient(const Vec3f& _a, const Vec3f& _b);

    void set_viridis();

    void set_magma();

    void set_inferno();

    void set_plasma();

    void set_rd_bu();

    void set_coolwarm();

    void update(const std::vector<f16>& _data);

    std::string name_ = "";

    wgpu::Texture texture_;
    wgpu::TextureView view_;
    wgpu::Sampler sampler_;
    wgpu::Device device_;
};

}
