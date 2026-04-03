#include "commons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) pointSize: f32,
    @align(16) clipBox: ClipBox,
    @align(16) valueType:ValueType,
    @align(16) valueFilter: vec2<f32>,
    @align(16) objectId:u32
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<PropertyValue>;

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) @interpolate(flat) value : vec4<f32>,
    @location(1) corner : vec2<f32>,
    @location(2) @interpolate(flat) vertexHandle: u32
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32
) -> VSOut {
    var out : VSOut;

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

    let value = props[iid];

    if (isOutsideClipBox(pos, ubo.clipBox)
        || isOutsideValueFilter(value, ubo.valueType, ubo.valueFilter)) {
        out.position = clippedPosition();
    }
    out.value = value;
    out.vertexHandle = iid;

    return out;
}

@fragment
fn fs_main(in : VSOut) -> FragmentOutput
{
    var out: FragmentOutput;
    out.color = getFragmentColorFromPropertyValue(
        in.value, ubo.valueType, ubo.valueFilter, colorMap, colorSampler
        );
    if (length(in.corner) > 1.0) {discard;} //round
    out.pick = vec4<u32>(ubo.objectId, 0, in.vertexHandle, bitcast<u32>(in.position.z));
    return out;
}
