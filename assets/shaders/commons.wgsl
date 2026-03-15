
alias PropertyValue = vec4<f32>;

alias ValueType = u32;
const VALUE_TYPE_COLOR:ValueType = 0;
const VALUE_TYPE_SCALAR:ValueType = 1;
const VALUE_TYPE_VEC3:ValueType = 2;

const COLOR_NAN:vec4<f32> = vec4<f32>(0,0,0,1);
const COLOR_INF:vec4<f32> = vec4<f32>(1,0,0,1);

struct ClipBox {
    @align(16) min: vec3<f32>,
    @align(16) max: vec3<f32>,
    @align(16) enabled: i32
};

fn isOutsideClipBox(pos:vec3<f32>, clipBox:ClipBox) -> bool {
    return (clipBox.enabled>0 && ((pos.x < clipBox.min.x)
|| (pos.x > clipBox.max.x) || (pos.y < clipBox.min.y)
|| (pos.y > clipBox.max.y) || (pos.z < clipBox.min.z)
|| (pos.z > clipBox.max.z)));
};

fn isOutsideValueFilter(
    value:PropertyValue,
    valueType:ValueType,
    valueFilter:vec2<f32>) -> bool
{
    return valueType==VALUE_TYPE_SCALAR
        && !isInf(value.x)
        && (value.x < valueFilter.x || value.x > valueFilter.y);
};

fn clippedPosition() -> vec4<f32> {
    return vec4<f32>(0,0,2,1);
};

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

fn isOutsideRange(val:f32, range:vec2<f32>) -> bool {
    return val < range.x || val > range.y;
};

struct FragmentOutput {
    @location(0) color : vec4<f32>,
    @location(1) pick : vec4<u32> // picking
};

fn getFragmentColorFromPropertyValue(
    value:PropertyValue,
    valueType:ValueType,
    valueFilter:vec2<f32>,
    colorMap : texture_2d<f32>,
    colorMapSampler: sampler
) -> vec4<f32>
{
    var color = vec4<f32>(1,0.9,0.9,0);

    // Interpret Value based on Property Mode
    if (valueType == VALUE_TYPE_COLOR) {
        color = value;
    } else if (valueType == VALUE_TYPE_SCALAR) {
        let t = clamp((value.x-valueFilter.x)/(valueFilter.y-valueFilter.x), 0.0, 1.0);
        color = textureSample(colorMap, colorMapSampler, vec2<f32>(t,0.5));
    } else if (valueType == VALUE_TYPE_VEC3) {
        color = vec4<f32>(normalize(value.xyz), 1.0);
    }

    if (isNan(value.x)) {
        color = COLOR_NAN;
    } else if (isInf(value.x)) {
        color = COLOR_INF;
    }

    return color;
}
