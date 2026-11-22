#include "commons.wgsl"
#include "line_as_quad.wgsl"

struct Uniforms {
    @align(16) view : mat4x4<f32>,
    @align(16) modelView : mat4x4<f32>,
    @align(16) projection : mat4x4<f32>,
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) lineWidth: f32,
    @align(16) clipBox: ClipBox,
    @align(16) objectId:u32,
    @align(16) ambient: vec4<f32>,
    @align(16) depthOffset: f32,
    @align(16) dirLight: DirectionalLight,
    @align(16) halfedgeScale: f32,
    @align(16) propFilter: PropFilterOptions
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> halfedgeProps : array<PropValue>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) @interpolate(flat) value : vec4<f32>,
    @location(1) quadCorner: vec2<f32>,
    @location(2) @interpolate(flat) quadSize: vec2<f32>,
    @location(3) @interpolate(flat) halfedgeHandle: u32,
    @location(4) @interpolate(flat) edgeDirView: vec3<f32>
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32,
    @location(0) vh0 : u32,
    @location(1) vh1 : u32,
    @location(2) heh : u32
) -> V2F
{
    var out : V2F;

    let p0View = ubo.modelView * vec4<f32>(positions[vh0], 1.0);
    let p1View = ubo.modelView * vec4<f32>(positions[vh1], 1.0);
    out.edgeDirView = normalize(p1View.xyz / p1View.w - p0View.xyz / p0View.w);

    let p0 = positions[vh0];
    let p1 = p0+(positions[vh1]-p0)*ubo.halfedgeScale;

    let quadCorner = getLineQuadCorner(
        ubo.mvp * vec4<f32>(p0, 1.0),
        ubo.mvp * vec4<f32>(p1, 1.0),
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
    let value = halfedgeProps[heh];
    let pos = mix(positions[vh0], p0, p1);
    if (isOutsideClipBox(pos, ubo.clipBox)
        || !propertyValueMatchesFilter(value, ubo.propFilter))
    {
        out.position = clippedPosition();
    }
    out.value = value;
    out.halfedgeHandle = heh;

    return out;
}

@fragment
fn fs_main(in:V2F) -> FragOutColPickDepth
{
    var out : FragOutColPickDepth;

    if (doDiscardRoundedLineQuadCornerFragment(
            in.quadCorner, in.quadSize)) {
        discard;
    }

    // Base property color
    out.color = getFragmentColorFromPropertyValue(
        in.value, ubo.propFilter, colorMap, colorSampler
        );

    // Compute Normal in View Space
    let T = normalize(in.edgeDirView);
    let viewDir = vec3<f32>(0.0, 0.0, 1.0);
    let S = normalize(cross(viewDir, T));
    let F = normalize(cross(T, S));
    let y = clamp(in.quadCorner.y / (in.quadSize.y * 0.5), -1.0, 1.0);
    let z = sqrt(max(0.0, 1.0 - y * y));
    let normal = normalize(y * S + z * F);

    // We transform the light direction to view space
    var dirLightView = ubo.dirLight;
    dirLightView.dir = normalize(
        (ubo.view * vec4<f32>(dirLightView.dir, 0.0)).xyz
        );
    let lighting = getLightContribution(dirLightView, normal);

    // Final color
    out.color = vec4<f32>(
        out.color.rgb * (lighting + ubo.ambient.rgb)*0.5f,
        out.color.a
        );

    // Depth correction for 3d imposter
    let depthCorrection = getImposterDepthCorrection(
        in.position.w,
        ubo.lineWidth * 0.5 * z * dot(F, viewDir),
        ubo.projection,
        ubo.viewportSize.y
        );
    let depth = in.position.z + depthCorrection;
    out.depth = depth;

    out.pick = vec4<u32>(ubo.objectId, ENTITY_TYPE_HALFEDGE, in.halfedgeHandle, bitcast<u32>(depth));
    return out;
}
