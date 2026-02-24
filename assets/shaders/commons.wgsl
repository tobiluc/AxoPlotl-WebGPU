struct Property {
    value : vec4<f32>
};

alias Mode = u32;
const MODE_COLOR:Mode = 0;
const MODE_SCALAR:Mode = 1;
const MODE_VEC3:Mode = 2;

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

fn clippedPosition() -> vec4<f32> {
    return vec4<f32>(0,0,2,1);
};

fn isOutsideRange(val:f32, range:vec2<f32>) -> bool {
    return val < range.x || val > range.y;
};

fn getFragmentColorFromPropertyValue(
    value:vec4<f32>,
    mode:Mode,
    valueFilter:vec2<f32>,
    colorMap : texture_2d<f32>,
    colorMapSampler: sampler
) -> vec4<f32> {
    // Interpret Value based on Property Mode
    if (mode == MODE_COLOR) {
        return value;
    } else if (mode == MODE_SCALAR) {
        let t = clamp((value.x-valueFilter.x)/(valueFilter.y-valueFilter.x), 0.0, 1.0);
        return textureSample(colorMap, colorMapSampler, vec2<f32>(t,0.5));
    } else if (mode == MODE_VEC3) {
        return vec4<f32>(normalize(value.xyz), 1.0);
    }
    return vec4<f32>(1,0.9,0.9,0);
}
