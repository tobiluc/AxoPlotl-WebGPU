#pragma once

namespace AxoPlotl
{

inline const char* vertex_shader_wgsl = R"(

alias Mode = u32;
const MODE_COLOR:Mode = 0;
const MODE_SCALAR:Mode = 1;
const MODE_VEC3:Mode = 2;

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) mode:Mode,
    @align(16) viewportSize: vec2<f32>,
    @align(16) pointSize: f32
};

@group(0) @binding(0)
var<uniform> ubo : Uniforms;

@group(0) @binding(1)
var<storage, read> positions : array<vec3<f32>>;

struct VertexProperty {
    color : vec4<f32>
};

@group(0) @binding(2)
var<storage, read> vertexProps : array<VertexProperty>;

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec4<f32>,
    @location(1) corner : vec2<f32>
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32
) -> VSOut {
    var out : VSOut;
    let pos = positions[iid];
    let color = vertexProps[iid].color;
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
    out.color = color;

    return out;
}

@fragment
fn fs_main(in : VSOut) -> @location(0) vec4<f32> {
    if (length(in.corner) > 1.0) {discard;} //round
    return in.color;
}

)";

}
