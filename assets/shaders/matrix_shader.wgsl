#include "commons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) lineWidth: f32,
    @align(16) clipBox: ClipBox,
    @align(16) vecScale:f32,
    @align(16) objectId:u32,
    @align(16) drawSymmetric:u32
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var<storage, read> matrices : array<mat3x3<f32>>;

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec4<f32>
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32
) -> VSOut {
    var out : VSOut;

    var pos:vec3<f32> = positions[iid];

    let m = matrices[iid];

    // A Point Instance is rendered as a Line in some direction
    for (var i = 0u; i < 3u; i += 1u) {
        if (vid == 2u*i+0u && ubo.drawSymmetric != 0u) {
            pos -= ubo.vecScale * m[i];
        } else if (vid == 2u*i+1u) {
            pos += ubo.vecScale * m[i];
        }
    }

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);
    out.color = vec4<f32>(0,0,0,1);
    out.color[vid/2] = 1.0f;

    return out;
}

@fragment
fn fs_main(in : VSOut) -> FragmentOutput
{
    var out:FragmentOutput;
    out.color = in.color;
    return out;
}
