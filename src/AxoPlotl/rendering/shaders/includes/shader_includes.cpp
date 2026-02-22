#include "shader_includes.hpp"
#include <string>
#include <unordered_map>

inline static std::unordered_map<std::string,const char*> shader_includes =
{

{"VolumeMeshCommons.wgsl", R"(

    struct Property {
        value : vec4<f32>
    };

    alias Mode = u32;
    const MODE_COLOR:Mode = 0;
    const MODE_SCALAR:Mode = 1;
    const MODE_VEC3:Mode = 2;

    struct ClipBox {
        @align(16) min: vec3<f32>,
        @align(16) max: vec3<f32>,
        @align(16) enabled: i32
    };

    fn isOutsideClipBox(pos:vec3<f32>, clipBox:ClipBox) -> bool {
        return (clipBox.enabled>0 && ((pos.x < clipBox.min.x)
    || (pos.x > clipBox.max.x) || (pos.y < clipBox.min.y)
    || (pos.y > clipBox.max.y) || (pos.z < clipBox.min.z)
    || (pos.z > clipBox.max.z)));
    };

    fn clippedPosition() -> vec4<f32> {
        return vec4<f32>(0,0,2,1);
    };

    fn isOutsideRange(val:f32, range:vec2<f32>) -> bool {
        return val < range.x || val > range.y;
    };

    fn getFragmentColorFromPropertyValue(
        value:vec4<f32>,
        mode:Mode,
        valueFilter:vec2<f32>,
        colorMap : texture_2d<f32>,
        colorMapSampler: sampler
    ) -> vec4<f32> {
        // Interpret Value based on Property Mode
        if (mode == MODE_COLOR) {
            return value;
        } else if (mode == MODE_SCALAR) {
            let t = clamp((value.x-valueFilter.x)/(valueFilter.y-valueFilter.x), 0.0, 1.0);
            return textureSample(colorMap, colorMapSampler, vec2<f32>(t,0.5));
        } else if (mode == MODE_VEC3) {
            return vec4<f32>(normalize(value.xyz), 1.0);
        }
        return vec4<f32>(1,0.9,0.9,0);
    }

)"},

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
