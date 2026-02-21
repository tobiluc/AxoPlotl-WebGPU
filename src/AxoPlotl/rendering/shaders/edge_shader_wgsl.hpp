#pragma once

#include <string>
namespace AxoPlotl
{

inline const std::string edge_shader_wgsl = R"(
#include "ShaderInput.wgsl"

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) value : vec4<f32>,
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32,
    @location(0) vh0 : u32,
    @location(1) vh1 : u32,
    @location(2) eh : u32
) -> V2F
{
    var out : V2F;

    //-------------------------------
    // An Edge instance is rendered
    // as a thin quad with width
    // given in screen space
    //-------------------------------

    // The two mesh vertex positions in clip space
    let clip0 = ubo.mvp * vec4<f32>(positions[vh0], 1.0);
    let clip1 = ubo.mvp * vec4<f32>(positions[vh1], 1.0);

    // Convert to NDC and get perpendicular offset
    let ndc0 = clip0.xy / clip0.w;
    let ndc1 = clip1.xy / clip1.w;
    let ndcEdgeDir = normalize(ndc1 - ndc0);
    let ndcPerpOffset = vec2<f32>(-ndcEdgeDir.y, ndcEdgeDir.x);

    // Get line wifth offset
    let ndcLwOffset = vec2<f32>(
        ubo.lineWidth / ubo.viewportSize.x,
        ubo.lineWidth / ubo.viewportSize.y
    );

    // Each edge is rendered as a Quad
    // p0 - offset, p0 + offset, p1 + offset, p1 - offset

    // Get interpolation t between edge vertices (for convenience)
    var t:f32 = 1.0;
    if (vid == 0u || vid == 1u) {t = 0.0;}

    // Apply offset in NDC
    var ndcPos = mix(ndc0, ndc1, t);
    if (vid == 0u || vid == 3u) {ndcPos -= ndcLwOffset * ndcPerpOffset;}
    else {ndcPos += ndcLwOffset * ndcPerpOffset;}

    // Convert back to clip space
    let clip = mix(clip0, clip1, t);
    out.position = vec4<f32>(ndcPos * clip.w, clip.z, clip.w);

    let value = edgeProps[eh].value;
    let pos = mix(positions[vh0], positions[vh1], t);
    if (isOutsideClipBox(pos, ubo.clipBox)
|| (ubo.mode==1u && isOutsideRange(value.x, ubo.valueFilter))) {
        out.position = clippedPosition();
    }
    out.value = value;

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32> {
    #include "FragmentReturnPropertyColor.wgsl"
}

)";

}
