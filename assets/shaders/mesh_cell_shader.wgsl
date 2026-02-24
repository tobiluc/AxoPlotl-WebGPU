#include "commons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) cellScale:f32,
    @align(16) clipBox: ClipBox,
    @align(16) mode:Mode,
    @align(16) valueFilter: vec2<f32>
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<Property>;
@group(0) @binding(5) var<storage, read> cellCenters : array<vec3<f32>>;

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
    let pos = cellCenters[cell_index]
+ ubo.cellScale * (positions[vertex_index]-cellCenters[cell_index]);
    out.position = ubo.mvp * vec4<f32>(pos, 1.0);

    let value = props[cell_index].value;
    if (isOutsideClipBox(pos, ubo.clipBox)
|| (ubo.mode==1u && isOutsideRange(value.x, ubo.valueFilter))) {
        out.position = clippedPosition();
    }
    out.value = value;

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32> {
    return getFragmentColorFromPropertyValue(
in.value, ubo.mode, ubo.valueFilter, colorMap, colorSampler);
}
