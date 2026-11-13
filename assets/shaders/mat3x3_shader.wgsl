#include "commons.wgsl"
#include "line_as_quad.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) lineWidth: f32,
    @align(16) clipBox: ClipBox,
    @align(16) vecScale:f32,
    @align(16) objectId:u32,
    @align(16) drawSymmetric:u32,
    @align(16) normalize:u32,
    @align(16) axisColors : array<vec4<f32>, 3>
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var<storage, read> matrices : array<mat3x3<f32>>;

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec4<f32>,
    @location(1) quadCorner : vec2<f32>,
    @location(2) @interpolate(flat) quadSize : vec2<f32>,
    @location(3) @interpolate(flat) matrixHandle : u32,
                                                                                                                                                                           @location(4) @interpolate(flat) axis : u32
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32
) -> VSOut {
    var out : VSOut;

    let matrixHandle = iid / 3u;
    let axis = iid % 3u;
    let origin = positions[matrixHandle];
    var m = matrices[matrixHandle];
    if (ubo.normalize != 0u) {
        for (var i = 0u; i < 3u; i += 1u) {
            m[i] = normalize(m[i]);
        }
    }
    m *= ubo.vecScale;

    // Calculate line endpoints.
    var p0 = origin;
    var p1 = origin;

    if (ubo.drawSymmetric != 0u) {
        p0 -= m[axis];
    }

    p1 += m[axis];

    // Transform endpoints to clip space.
    let clip0 = ubo.mvp * vec4<f32>(p0, 1.0);
    let clip1 = ubo.mvp * vec4<f32>(p1, 1.0);

    // Generate screen-space quad.
    let quad = getLineQuadCorner(
        clip0,
        clip1,
        vid,
        ubo.viewportSize,
        ubo.lineWidth
        );

    out.position = quad.position;
    out.quadCorner = quad.quadCorner;
    out.quadSize = quad.quadSize;

    out.color = ubo.axisColors[axis];

    out.matrixHandle = matrixHandle;
    out.axis = axis;

    return out;
}

@fragment
fn fs_main(in : VSOut) -> FragmentOutput
{
    var out:FragmentOutput;
    out.color = in.color;
    return out;
}
