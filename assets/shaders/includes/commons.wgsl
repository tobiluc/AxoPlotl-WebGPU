#include "light.wgsl"
#include "imposter.wgsl"
#include "properties.wgsl"
#include "clipping.wgsl"
#include "bits.wgsl"

const COLOR_NAN:vec4<f32> = vec4<f32>(0,0,0,1);
const COLOR_INF:vec4<f32> = vec4<f32>(1,0,0,1);

alias EntityType = u32;
const ENTITY_TYPE_VERTEX:EntityType = 0;
const ENTITY_TYPE_HALFEDGE:EntityType = 1;
const ENTITY_TYPE_HALFFACE:EntityType = 2;
const ENTITY_TYPE_CELL:EntityType = 3;
const ENTITY_TYPE_VECTOR:EntityType = 6;

// Exponent is all 1s and Mantissa is NOT zero
fn isNan(val: f32) -> bool {
    let bits = bitcast<u32>(val);
    let exponent = (bits >> 23u) & 0xffu;
    let mantissa = bits & 0x7fffffu;
    return (exponent == 0xffu) && (mantissa != 0u);
}

fn isInf(val: f32) -> bool {
    let bits = bitcast<u32>(val);
    let magnitude = bits & 0x7fffffffu;
    return magnitude == 0x7f800000u;
}

fn colorSelected(_col:vec4<f32>) -> vec4<f32>
{
    return vec4<f32>(
        mix(_col.rgb, vec3<f32>(1.0, 0.5, 0.0), 0.4),
        _col.a
    );
};

fn isOutsideRange(val:f32, range:vec2<f32>) -> bool {
    return val < range.x || val > range.y;
};

struct Entity {
    objectId:u32,
    entityType:EntityType,
    entityIndex:u32
};

fn isSameEntity(a: Entity, b: Entity) -> bool {
    return a.objectId == b.objectId &&
           a.entityType == b.entityType &&
           a.entityIndex == b.entityIndex;
};

struct FragOutColPick {
    @location(0) color : vec4<f32>,
    @location(1) pick : vec4<u32> // picking
};

struct FragOutColPickDepth {
    @location(0) color : vec4<f32>,
    @location(1) pick : vec4<u32>,
    @builtin(frag_depth) depth: f32
};

