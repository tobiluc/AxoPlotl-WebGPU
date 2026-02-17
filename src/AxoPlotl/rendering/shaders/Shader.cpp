// #include "Shader.hpp"

// namespace AxoPlotl
// {

// wgpu::ShaderModule create_shader_module(wgpu::Device _device,
//     const char* _src)
// {
//     // Specify the WGSL part of the shader module descriptor
//     wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
//     wgslDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
//     wgslDesc.code = _src;

//     wgpu::ShaderModuleDescriptor desc = {};
// #ifdef WEBGPU_BACKEND_WGPU
//     desc.hintCount = 0;
//     desc.hints = nullptr;
// #endif

//     // Connect the chain
//     desc.nextInChain = &wgslDesc.chain;
//     desc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
//     desc.label = "Cool Shader";

//     return _device.createShaderModule(desc);
// }

// }
