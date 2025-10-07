#pragma once
#include "Buffer.h"
#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

namespace AxoPlotl
{

class Renderer
{

public:
    Renderer();

    ~Renderer();

    bool init(GLFWwindow* window);

    void render();

private:
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Surface surface;
    std::unique_ptr<wgpu::ErrorCallback> uncapturedErrorCallbackHandle;

    wgpu::TextureFormat colorFormat = wgpu::TextureFormat::Undefined;
    wgpu::RenderPipeline pipeline;

    wgpu::Buffer vertexBuffer;
    uint32_t vertexCount = 0;
    wgpu::Buffer indexBuffer;
    uint32_t indexCount = 0;
};

}
