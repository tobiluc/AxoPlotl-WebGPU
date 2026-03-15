#include "commons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) clipBox: ClipBox,
    @align(16) valueType:ValueType,
    @align(16) valueFilter: vec2<f32>
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<PropertyValue>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) @interpolate(flat) value : vec4<f32>,
};

@vertex
fn vs_main(
    @location(0) vertex_index : u32,
    @location(1) face_index : u32
) -> V2F {

    var out : V2F;

    let pos = positions[vertex_index];

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);

    let value = props[face_index];
    if (isOutsideClipBox(pos, ubo.clipBox)
        || isOutsideValueFilter(value, ubo.valueType, ubo.valueFilter)) {
        out.position = clippedPosition();
    }
    out.value = value;

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32> {
    let fragColor = getFragmentColorFromPropertyValue(
        in.value, ubo.valueType, ubo.valueFilter, colorMap, colorSampler
        );
    return fragColor;
}
