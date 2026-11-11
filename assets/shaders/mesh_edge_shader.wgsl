#include "commons.wgsl"

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

    //-------------------------------
    // An Edge instance is rendered
    // as a thin quad with width
    // given in screen space
    //-------------------------------

    // Define the quad corners: (t, side)
    // t: 0.0 = start of edge, 1.0 = end of edge
    // side: -1.0 = left, 1.0 = right

    // For Triangle Strip (0, 1, 2, 3):
    // 0: Start-Left, 1: Start-Right, 2: End-Left, 3: End-Right
    var configs = array<vec2<f32>, 4>(
        vec2<f32>(0.0, -1.0), // 0
        vec2<f32>(0.0,  1.0), // 1
        vec2<f32>(1.0, -1.0), // 2
        vec2<f32>(1.0,  1.0)  // 3
    );

    let config = configs[vid];
    let t = config.x;
    let side = config.y;

    let clip0 = ubo.mvp * vec4<f32>(positions[vh0], 1.0);
    let clip1 = ubo.mvp * vec4<f32>(positions[vh1], 1.0);

    // Convert NDC to pixel coordinates.
    // Y is inverted because screen coordinates usually grow downward.
    let ndc0 = clip0.xy / clip0.w;
    let ndc1 = clip1.xy / clip1.w;

    // Convert to pixel coordinates
    let screen0 = ndc0 * ubo.viewportSize * 0.5;
    let screen1 = ndc1 * ubo.viewportSize * 0.5;

    let edgeVector = screen1 - screen0;
    let edgeLength = length(edgeVector);

    let edgeDir = edgeVector / max(edgeLength, 0.0001);
    let perpDir = vec2<f32>(-edgeDir.y, edgeDir.x);

    // Half-width in pixels
    let offsetPixels = perpDir * (ubo.lineWidth * 0.5 * side);

    // Convert back to NDC
    let offsetNdc = offsetPixels * 2.0 / ubo.viewportSize;

    var outPos = mix(clip0, clip1, t);

    // Apply offset in clip space.
    outPos.x += offsetNdc.x * outPos.w;
    outPos.y += offsetNdc.y * outPos.w;

    // Subtract Depth Offset to render on top
    // of faces
    outPos.z -= ubo.depthOffset;

    out.position = outPos;

    out.quadSize = vec2<f32>(edgeLength, ubo.lineWidth);
    out.quadCorner = vec2<f32>(
        (config.x - 0.5) * edgeLength,
        0.5 * config.y * ubo.lineWidth
        );

    // Property and Clipping
    let value = props[eh];
    let pos = mix(positions[vh0], positions[vh1], t);
    if (isOutsideClipBox(pos, ubo.clipBox)
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

    // Fake 3D
    let y = in.quadCorner.y / (in.quadSize.y * 0.5);
    let shading = 0.78 + 0.22 * sqrt(max(0.0, 1.0 - y * y));
    let highlight = 0.10 * exp(-pow((y + 0.45) * 12.0, 2.0));
    let shadow = 0.12 * smoothstep(0.1, 1.0, y);
    let finalShading = shading + highlight - shadow;

    out.color = getFragmentColorFromPropertyValue(
        in.value, ubo.valueType, ubo.valueFilter, colorMap, colorSampler
        ) * ubo.ambient * finalShading;
    out.pick = vec4<u32>(ubo.objectId, 1, in.edgeHandle, bitcast<u32>(in.position.z));
    return out;
}
