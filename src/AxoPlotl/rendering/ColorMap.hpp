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
    using f16x3 = glm::vec<3,f16>;

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

    void set_gradient(const std::vector<f16x3>& _colors);

    void set_gradient(const f16x3& _a, const f16x3& _b);

    void set_single_color(const f16x3& _color);

    void set_viridis();

    void set_magma();

    void set_plasma();

    void set_rd_bu();

    void set_coolwarm();

    void set_rainbow();

    void update(const std::vector<f16>& _data);

    std::string name_ = "";

    wgpu::Texture texture_;
    wgpu::TextureView view_;
    wgpu::Sampler sampler_;
    wgpu::Device device_;
};

}
