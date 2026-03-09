#include "commons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) lineWidth: f32,
    @align(16) clipBox: ClipBox,
    @align(16) mode:Mode,
    @align(16) valueFilter: vec2<f32>
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<Property>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) value : vec4<f32>,
    @location(1) quadCorner: vec2<f32>,
    @location(2) quadSize: vec2<f32>
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

    // Project to screen to get the 2D direction
    let screen0 = (clip0.xy / clip0.w) * ubo.viewportSize;
    let screen1 = (clip1.xy / clip1.w) * ubo.viewportSize;
    let edgeDir = normalize(screen1 - screen0);
    let perpDir = vec2<f32>(-edgeDir.y, edgeDir.x);

    // Calculate final position
    var outPos = mix(clip0, clip1, t);

    // Apply thickness offset
    let offset = (perpDir * ubo.lineWidth * 0.5 * side) / ubo.viewportSize;
    outPos.x += offset.x * outPos.w * 2.0;
    outPos.y += offset.y * outPos.w * 2.0;

    out.position = outPos;

    // Tell the fragment where we are relative to the center of the quad
    out.quadSize = vec2<f32>(length(screen1-screen0), ubo.lineWidth);
    out.quadCorner = vec2<f32>((config.x-0.5)*out.quadSize.x,
                               0.5*config.y*out.quadSize.y);

    // Property and Clipping
    let value = props[eh].value;
    let pos = mix(positions[vh0], positions[vh1], t);
    if (isOutsideClipBox(pos, ubo.clipBox)
        || (ubo.mode==1u && isOutsideRange(value.x, ubo.valueFilter)))
    {
        out.position = clippedPosition();
    }
    out.value = value;

    return out;
}

@fragment
fn fs_main(in:V2F) -> @location(0) vec4<f32>
{
    // Round Edge Endpoints
    let d = abs(0.5*(in.quadSize.x-in.quadSize.y));
    let cx = min(max(in.quadCorner.x-d,0.0),in.quadCorner.x+d);
    let cy = in.quadCorner.y;
    if (cx*cx+cy*cy > 0.25*in.quadSize.y*in.quadSize.y)
    {discard;}

    return getFragmentColorFromPropertyValue(
in.value, ubo.mode, ubo.valueFilter, colorMap, colorSampler);
}
