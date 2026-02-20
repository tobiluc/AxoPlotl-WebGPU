#pragma once

namespace AxoPlotl
{

inline const char* face_shader_wgsl = R"(

alias Mode = u32;
const MODE_COLOR:Mode = 0;
const MODE_SCALAR:Mode = 1;
const MODE_VEC3:Mode = 2;

struct FaceProperty {
    value : vec4<f32>
};

struct Uniforms {
    mvp: mat4x4<f32>,
    mode: Mode
};

@group(0) @binding(0)
var<uniform> ubo : Uniforms;

@group(0) @binding(1)
var<storage, read> positions : array<vec3<f32>>;

@group(0) @binding(4)
var<storage, read> face_props : array<FaceProperty>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec4<f32>,
};

@vertex
fn vs_main(
    @location(0) vertex_index : u32,
    @location(1) face_index : u32
) -> V2F {

    var out : V2F;

    let pos = positions[vertex_index];
    let value = face_props[face_index].value;

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);
    if (ubo.mode == MODE_COLOR) {
        out.color = value;
    } else if (ubo.mode == MODE_SCALAR) {
        out.color = vec4<f32>(1,0,0,1);
    } else if (ubo.mode == MODE_VEC3) {
        out.color = vec4<f32>(normalize(value.xyz),1.0);
    }

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32> {
    return in.color;
}

)";

}
