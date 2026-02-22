#pragma once

#include <string>
namespace AxoPlotl
{

inline const std::string cell_shader_wgsl = R"(
#include "VolumeMeshCommons.wgsl"
#include "VolumeMeshInputs.wgsl"

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) value : vec4<f32>,
};

@vertex
fn vs_main(
    @location(0) vertex_index : u32,
    @location(1) cell_index : u32
) -> V2F {

    var out : V2F;

    // Scale around incenter
    let pos = cellIncenters[cell_index]
+ ubo.cellScale * (positions[vertex_index]-cellIncenters[cell_index]);
    out.position = ubo.mvp * vec4<f32>(pos, 1.0);

    let value = cellProps[cell_index].value;
    if (isOutsideClipBox(pos, ubo.clipBox)
|| (ubo.cellMode==1u && isOutsideRange(value.x, ubo.cellValueFilter))) {
        out.position = clippedPosition();
    }
    out.value = value;

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32> {
    return getFragmentColorFromPropertyValue(
in.value, ubo.cellMode, ubo.cellValueFilter, colorMap, colorSampler);
}

)";

}
