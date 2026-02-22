#pragma once

#include <string>

namespace AxoPlotl
{

inline const std::string face_shader_wgsl = R"(
#include "VolumeMeshCommons.wgsl"
#include "VolumeMeshInputs.wgsl"

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) value : vec4<f32>,
};

@vertex
fn vs_main(
    @location(0) vertex_index : u32,
    @location(1) face_index : u32
) -> V2F {

    var out : V2F;

    let pos = positions[vertex_index];

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);

    let value = faceProps[face_index].value;
    if (isOutsideClipBox(pos, ubo.clipBox)
|| (ubo.faceMode==1u && isOutsideRange(value.x, ubo.faceValueFilter))) {
        out.position = clippedPosition();
    }
    out.value = value;

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32> {
    return getFragmentColorFromPropertyValue(
in.value, ubo.faceMode, ubo.faceValueFilter, colorMap, colorSampler);
}

)";

}
