
struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) clipBox: ClipBox,
    @align(16) valueType:ValueType,
    @align(16) valueFilter: vec2<f32>,
    @align(16) objectId:u32,
    @align(16) entityType: u32,
    @align(16) ambient: vec4<f32>,
    @align(16) dirLight: DirectionalLight
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<PropertyValue>;
@group(0) @binding(5) var<storage, read> normals : array<vec3<f32>>;

struct V2F {
    @builtin(position) position : vec4<f32>,
    @location(0) @interpolate(flat) value : vec4<f32>,
    @location(1) @interpolate(flat) entity_index: u32,
    @location(2) @interpolate(flat) normal: vec3<f32>
};

@vertex
fn vs_main(
    @location(0) vertex_index : u32,
    @location(1) entity_index : u32
) -> V2F {

    var out : V2F;

    let pos = positions[vertex_index];
    out.position = ubo.mvp * vec4<f32>(pos, 1.0);
    out.normal = normals[vertex_index];

    let value = props[entity_index];
    if (isOutsideClipBox(pos, ubo.clipBox)
        || isOutsideValueFilter(value, ubo.valueType, ubo.valueFilter)) {
        out.position = clippedPosition();
    }
    out.value = value;
    out.entity_index = entity_index;

    return out;
}

@fragment
fn fs_main(in:V2F) -> FragmentOutput
{
    var out: FragmentOutput;

    out.color = getFragmentColorFromPropertyValue(
        in.value, ubo.valueType, ubo.valueFilter, colorMap, colorSampler
        ) * ubo.ambient;

    // Directional diffuse lighting
    let lighting = getLightContribution(ubo.dirLight, in.normal);

    out.color = vec4<f32>(
        out.color.rgb * (lighting + ubo.ambient.rgb),
        out.color.a
        );

    out.pick = vec4<u32>(ubo.objectId, ubo.entityType, in.entity_index, bitcast<u32>(in.position.z / in.position.w));
    return out;
}
