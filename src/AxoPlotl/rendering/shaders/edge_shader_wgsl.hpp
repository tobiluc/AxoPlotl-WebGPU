#pragma once

namespace AxoPlotl
{

inline const char* edge_shader_wgsl = R"(

alias Mode = u32;
const MODE_COLOR:Mode = 0;
const MODE_SCALAR:Mode = 1;
const MODE_VEC3:Mode = 2;

struct EdgeProperty {
    color : vec4<f32>
};

struct Uniforms {
    mvp : mat4x4<f32>,
    mode: Mode
};

@group(0) @binding(0)
var<uniform> ubo : Uniforms;

@group(0) @binding(1)
var<storage, read> positions : array<vec3<f32>>;

@group(0) @binding(3)
var<storage, read> edge_props : array<EdgeProperty>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec4<f32>,
};

@vertex
fn vs_main(
    @location(0) vertex_index : u32,
    @location(1) edge_index : u32
) -> V2F {

    var out : V2F;

    let pos = positions[vertex_index];
    let color = edge_props[edge_index].color;

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);
    out.color = color;

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32> {
    return in.color;
}

)";

}
