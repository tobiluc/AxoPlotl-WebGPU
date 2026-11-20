#include "commons.wgsl"

struct Uniforms {
    @align(16) view : mat4x4<f32>,
    @align(16) projection : mat4x4<f32>,
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) pointSize: f32,
    @align(16) clipBox: ClipBox,
    @align(16) objectId:u32,
    @align(16) ambient: vec4<f32>,
    @align(16) depthOffset: f32,
    @align(16) dirLight: DirectionalLight,
    @align(16) propFilter: PropFilterOptions
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<PropValue>;

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

    // Subtract Depth Offset to render on top
    // of edges
    out.position.z -= ubo.depthOffset;

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
        || !propertyValueMatchesFilter(value, ubo.propFilter))
    {
        out.position = clippedPosition();
    }
    out.value = value;
    out.vertexHandle = iid;

    return out;
}

@fragment
fn fs_main(in : VSOut) -> FragOutColPickDepth
{
    // Render as Circle
    let dist2 = dot(in.corner, in.corner);
    if (dist2 > 1.0) {discard;}

    // Depth correction for 3d imposter
    let sphereZ = sqrt(1.0 - dist2);
    let depthCorrection = getImposterDepthCorrection(
        in.position.w,
        ubo.pointSize * sphereZ * 0.5,
        ubo.projection,
        ubo.viewportSize.y
        );
    let depth = in.position.z + depthCorrection;

    var out: FragOutColPickDepth;

    // Base color
    out.color = getFragmentColorFromPropertyValue(
        in.value, ubo.propFilter, colorMap, colorSampler
        );

    // Lighting - computed in view space
    var dirLightView = ubo.dirLight;
    dirLightView.dir = normalize(
        (ubo.view * vec4<f32>(dirLightView.dir, 0.0)).xyz
    );
    let normal = normalize(vec3<f32>(
        in.corner.x,
        in.corner.y,
        sphereZ
        ));
    let lighting = getLightContribution(dirLightView, normal);

    // Final fragment output
    out.color = vec4<f32>(
        out.color.rgb * (lighting + ubo.ambient.rgb)*0.5f,
        out.color.a
        );
    out.depth = depth;
    out.pick = vec4<u32>(ubo.objectId, ENTITY_TYPE_VERTEX, in.vertexHandle, bitcast<u32>(depth));

    return out;
}
