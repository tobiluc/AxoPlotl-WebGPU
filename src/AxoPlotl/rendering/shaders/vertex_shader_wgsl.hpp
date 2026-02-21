#pragma once
#include <string>

namespace AxoPlotl
{

inline const std::string vertex_shader_wgsl = R"(
#include "commons.wgsl"

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) value : vec4<f32>,
    @location(1) corner : vec2<f32>
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32
) -> VSOut {
    var out : VSOut;

    let value = vertexProps[iid].value;

    let pos = positions[iid];
    out.position = ubo.mvp * vec4<f32>(pos, 1.0);

    // A Point Instance is rendered as a Quad
    out.corner = vec2<f32>(0.0);
    if (vid == 0u) {out.corner = vec2(-1.0, -1.0);}
    if (vid == 1u) {out.corner = vec2( 1.0, -1.0);}
    if (vid == 2u) {out.corner = vec2(-1.0,  1.0);}
    if (vid == 3u) {out.corner = vec2( 1.0,  1.0);}

    // Add Offset
    out.position += vec4<f32>(
        out.corner.x * (ubo.pointSize / ubo.viewportSize.x) * out.position.w,
        out.corner.y * (ubo.pointSize / ubo.viewportSize.y) * out.position.w,
        0.0,
        0.0
    );

    // Move out of clip space if property is not is visible scalar range
    if (ubo.mode == 1u &&
        (value.x < ubo.valueFilter.x || value.x > ubo.valueFilter.y)) {
        out.position = vec4<f32>(0,0,2,1);
    }

    out.value = value;
    return out;
}

@fragment
fn fs_main(in : VSOut) -> @location(0) vec4<f32>
{
    if (length(in.corner) > 1.0) {discard;} //round
    #include "frag_return_property_color.wgsl"
}

)";

}
