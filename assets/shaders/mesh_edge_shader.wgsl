#include "commons.wgsl"
#include "line_as_quad.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) lineWidth: f32,
    @align(16) clipBox: ClipBox,
    @align(16) valueType: ValueType,
    @align(16) valueFilter: vec2<f32>,
    @align(16) objectId:u32,
    @align(16) ambient: vec4<f32>,
    @align(16) depthOffset: f32
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<PropertyValue>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) @interpolate(flat) value : vec4<f32>,
    @location(1) quadCorner: vec2<f32>,
    @location(2) @interpolate(flat) quadSize: vec2<f32>,
    @location(3) @interpolate(flat) edgeHandle: u32
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

    let quadCorner = getLineQuadCorner(
        ubo.mvp * vec4<f32>(positions[vh0], 1.0),
        ubo.mvp * vec4<f32>(positions[vh1], 1.0),
        vid,
        ubo.viewportSize,
        ubo.lineWidth
    );

    // Subtract Depth Offset to render on top
    // of faces
    out.position = quadCorner.position;
    out.position.z -= ubo.depthOffset;

    out.quadSize = quadCorner.quadSize;
    out.quadCorner = quadCorner.quadCorner;

    // Property and Clipping
    let value = props[eh];
    // let pos = mix(positions[vh0], positions[vh1], t);
    if (isOutsideClipBox(out.position.xyz, ubo.clipBox)
        || isOutsideValueFilter(value, ubo.valueType, ubo.valueFilter)) {
        out.position = clippedPosition();
    }
    out.value = value;
    out.edgeHandle = eh;

    return out;
}

@fragment
fn fs_main(in:V2F) -> FragmentOutput
{
    var out : FragmentOutput;

    let shading = getLineQuadShading(in.quadCorner, in.quadSize);

    out.color = getFragmentColorFromPropertyValue(
        in.value, ubo.valueType, ubo.valueFilter, colorMap, colorSampler
        ) * ubo.ambient * shading;
    out.pick = vec4<u32>(ubo.objectId, 1, in.edgeHandle, bitcast<u32>(in.position.z / in.position.w));
    return out;
}
