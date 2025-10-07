#include "Shader.h"
#include "../IO/file_access.h"

namespace AxoPlotl
{

wgpu::ShaderModule loadShaderModule(wgpu::Device device, const std::filesystem::path& path)
{
    const char* code = read_file(path.c_str());
    if (!code) {
        std::cerr << "AxoPlotl: Failed to read shader file" << std::endl;
        return nullptr;
    }

    // Specify the WGSL part of the shader module descriptor
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    wgslDesc.code = code;

    wgpu::ShaderModuleDescriptor desc = {};
#ifdef WEBGPU_BACKEND_WGPU
    desc.hintCount = 0;
    desc.hints = nullptr;
#endif

    // Connect the chain
    desc.nextInChain = &wgslDesc.chain;
    desc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
    desc.label = path.c_str();

    return wgpuDeviceCreateShaderModule(device, &desc);
}

}
