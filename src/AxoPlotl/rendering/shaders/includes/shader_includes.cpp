#include "shader_includes.hpp"
#include <string>
#include <unordered_map>

inline static std::unordered_map<std::string,const char*> shader_includes =
{
{"ShaderInput.wgsl", R"(
    alias Mode = u32;
    const MODE_COLOR:Mode = 0;
    const MODE_SCALAR:Mode = 1;
    const MODE_VEC3:Mode = 2;

    struct ClipBox {
        @align(16) min: vec3<f32>,
        @align(16) max: vec3<f32>,
        @align(16) enabled: i32
    };

    struct Uniforms {
        @align(16) mvp : mat4x4<f32>,
        @align(16) mode:Mode,
        @align(16) viewportSize: vec2<f32>,
        @align(16) pointSize: f32,
        @align(16) lineWidth: f32,
        @align(16) cellScale: f32,
        @align(16) valueFilter: vec2<f32>,
        @align(16) clipBox: ClipBox
    };

    struct Property {
        value : vec4<f32>
    };

    @group(0) @binding(0) var<uniform> ubo : Uniforms;
    @group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
    @group(0) @binding(2) var colorMap : texture_2d<f32>;
    @group(0) @binding(3) var colorSampler : sampler;
    @group(0) @binding(4) var<storage, read> vertexProps : array<Property>;
    @group(0) @binding(5) var<storage, read> edgeProps : array<Property>;
    @group(0) @binding(6) var<storage, read> faceProps : array<Property>;
    @group(0) @binding(7) var<storage, read> cellProps : array<Property>;
    @group(0) @binding(8) var<storage, read> cellIncenters : array<vec3<f32>>;

    fn isOutsideClipBox(pos:vec3<f32>, clipBox:ClipBox) -> bool {
        return (ubo.clipBox.enabled>0 && ((pos.x < ubo.clipBox.min.x)
    || (pos.x > ubo.clipBox.max.x) || (pos.y < ubo.clipBox.min.y)
    || (pos.y > ubo.clipBox.max.y) || (pos.z < ubo.clipBox.min.z)
    || (pos.z > ubo.clipBox.max.z)));
    };

    fn clippedPosition() -> vec4<f32> {
        return vec4<f32>(0,0,2,1);
    };

    fn isOutsideRange(val:f32, range:vec2<f32>) -> bool {
        return val < range.x || val > range.y;
    };

)"},


{"FragmentReturnPropertyColor.wgsl", R"(
    // Interpret Value based on Property Mode
    if (ubo.mode == MODE_COLOR) {
        return in.value;
    } else if (ubo.mode == MODE_SCALAR) {
        let t = clamp((in.value.x-ubo.valueFilter.x)/(ubo.valueFilter.y-ubo.valueFilter.x), 0.0, 1.0);
        return textureSample(colorMap, colorSampler, vec2<f32>(t,0.5));
    } else if (ubo.mode == MODE_VEC3) {
        return vec4<f32>(normalize(in.value.xyz), 1.0);
    }
    return vec4<f32>(1,0.9,0.9,0);

)"}
};

std::string AxoPlotl::parse_shader_with_includes(const std::string& _source)
{
    std::string result = _source;
    size_t pos = 0;

    while ((pos = result.find("#include")) != std::string::npos)
    {
        // Find start and end quotes of include statement
        size_t quote_start = result.find('"', pos);
        size_t quote_end = result.find('"', quote_start+1);

        if (quote_start == std::string::npos
            || quote_end == std::string::npos
            || quote_start >= quote_end)
        {
            throw std::runtime_error("Malformed #include directive in shader:\n" + result);
        }

        std::string include_name = result.substr(quote_start+1, quote_end-quote_start-1);

        auto it = shader_includes.find(include_name);
        if (it == shader_includes.end()) {
            throw std::runtime_error("Unknown shader include: " + include_name);
        }

        // Replace the whole #include line with the included shader
        std::string before = result.substr(0, pos);
        std::string after = result.substr(quote_end + 1);

        result = before + it->second + "\n" + after;
    }
    return result;
}
