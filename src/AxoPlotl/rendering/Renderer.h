// #pragma once
// #include "Buffer.h"
// #include "TrianglePipeline.h"
// #include <webgpu/webgpu.hpp>
// #include <GLFW/glfw3.h>
// #include <glfw3webgpu.h>

// namespace AxoPlotl
// {

// class Renderer
// {

// public:
//     Renderer();

//     ~Renderer();

//     void release();

//     bool init(GLFWwindow* window);

//     void render();

//     void onWindowResize(float width, float height);

// private:
//     wgpu::Device device;
//     wgpu::Queue queue;
//     wgpu::Surface surface;
//     std::unique_ptr<wgpu::ErrorCallback> uncapturedErrorCallbackHandle;

//     void configureSurface(float width, float height);

//     wgpu::TextureFormat colorFormat = wgpu::TextureFormat::Undefined;

// };

// }
