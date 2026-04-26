#include "commons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) valueRange : vec2<f32>,
    @align(16) objectId:u32
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var colorMap : texture_2d<f32>;
@group(0) @binding(2) var colorMapSampler : sampler;

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) value : f32
};

@vertex
fn vs_main(
    @location(0) position: vec3<f32>,
    @location(1) value: f32
) -> VSOut {
    var out : VSOut;

    out.position = ubo.mvp * vec4<f32>(position * value, 1.0);
    out.value = value;

    return out;
}

@fragment
fn fs_main(in : VSOut) -> FragmentOutput
{
    let t:f32 = clamp((in.value - ubo.valueRange.x) / (ubo.valueRange.y - ubo.valueRange.x), 0.0, 1.0);
    var out:FragmentOutput;
    out.color = textureSample(colorMap, colorMapSampler, vec2<f32>(t,0.5));
    out.pick = vec4<u32>(ubo.objectId, 2, bitcast<u32>(in.value), bitcast<u32>(in.position.z));
    return out;
}
