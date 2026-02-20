#pragma once

namespace AxoPlotl
{

inline const char* vertex_shader_wgsl = R"(

alias Mode = u32;
const MODE_COLOR:Mode = 0;
const MODE_SCALAR:Mode = 1;
const MODE_VEC3:Mode = 2;

struct Uniforms {
    mvp : mat4x4<f32>,
    mode:Mode
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
};

@vertex
fn vs_main(@builtin(vertex_index) vid : u32) -> VSOut {
    var out : VSOut;
    let pos = positions[vid];
    let color = vertexProps[vid].color;

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);
    out.color = color;

    return out;
}

@fragment
fn fs_main(in : VSOut) -> @location(0) vec4<f32> {
    return in.color;
}

)";

}
