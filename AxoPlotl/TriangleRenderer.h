#pragma once

#include "webgpu/webgpu.hpp"
#include <vector>

namespace AxoPlotl
{

class TriangleRenderer
{
public:
    TriangleRenderer();

    ~TriangleRenderer();

    void initialize(wgpu::Device& device, wgpu::Queue& queue, const std::vector<float>& vertexData);

    void draw();

private:


};

}
