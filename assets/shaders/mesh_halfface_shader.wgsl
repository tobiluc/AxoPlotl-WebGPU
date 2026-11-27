#include "commons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) clipBox: ClipBox,
    @align(16) objectId:u32,
    @align(16) ambient: vec4<f32>,
    @align(16) depthOffset: f32,
    @align(16) dirLight: DirectionalLight,
    @align(16) selectedEntity: Entity,
    @align(16) propFilter: PropFilterOptions
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> halffaceProps : array<PropValue>;
@group(0) @binding(5) var<storage, read> halffaceNormals : array<vec3<f32>>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) @interpolate(flat) value : PropValue,
    @location(1) @interpolate(flat) halfFaceHandle: u32,
    @location(2) @interpolate(flat) normal: vec3<f32>,
    @location(3) @interpolate(flat) selected: u32
};

@vertex
fn vs_main(
    @location(0) vertex_index : u32,
    @location(1) halfface_index : u32
) -> V2F {

    var out : V2F;

    let pos = positions[vertex_index];

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);
    out.normal = halffaceNormals[halfface_index];

    // Subtract Depth Offset to render on top
    // of cells
    out.position.z -= ubo.depthOffset;

    let value = halffaceProps[halfface_index];
    if (isOutsideClipBox(pos, ubo.clipBox)
        || !propertyValueMatchesFilter(value, ubo.propFilter))
    {
        out.position = clippedPosition();
    }
    out.value = value;
    out.halfFaceHandle = halfface_index;

    var myEntity:Entity;
    myEntity.objectId = ubo.objectId;
    myEntity.entityType = ENTITY_TYPE_HALFFACE;
    myEntity.entityIndex = halfface_index;
    out.selected = select(0u, 1u, isSameEntity(myEntity, ubo.selectedEntity));

    return out;
}

@fragment
fn fs_main(in:V2F) -> FragOutColPick
{

    var out: FragOutColPick;

    out.color = getFragmentColorFromPropertyValue(
        in.value, ubo.propFilter, colorMap, colorSampler
        );

    let lighting = getLightContribution(ubo.dirLight, in.normal);

    out.color = vec4<f32>(
        out.color.rgb * (lighting + ubo.ambient.rgb)*0.5f,
        out.color.a
        );
    if (in.selected != 0u) {
        out.color = colorSelected(out.color);
    }

    out.pick = vec4<u32>(ubo.objectId, ENTITY_TYPE_HALFFACE, in.halfFaceHandle, bitcast<u32>(in.position.z));
    return out;
}
