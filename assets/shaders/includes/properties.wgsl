alias PropValue = f32;

alias PropValueMode = u32;
const PROP_VALUE_MODE_NOPROP:PropValueMode = 0u;
const PROP_VALUE_MODE_SCALAR:PropValueMode = 1u;
const PROP_VALUE_MODE_VEC3:PropValueMode = 2u;
const PROP_VALUE_MODE_MAT3x3:PropValueMode = 3u;

alias PropFilterMode = u32;
const PROP_FILTER_MODE_NONE:PropFilterMode = 0u;
const PROP_FILTER_MODE_INSIDE_RANGE:PropFilterMode = 1u;
const PROP_FILTER_MODE_NAN = 2u;
const PROP_FILTER_MODE_INF = 3u;

struct PropFilterOptions
{
    @align(16) valueMode: PropValueMode,
    @align(16) filterMode: PropFilterMode,
    @align(16) invert: u32,
    @align(16) scalarRange: vec2<f32>,
    @align(16) scalarIncl: vec2<u32>
};

fn propertyValueMatchesFilter(_val:PropValue, _filter:PropFilterOptions) -> bool
{
    if (_filter.filterMode == PROP_FILTER_MODE_NONE ||
        _filter.valueMode == PROP_VALUE_MODE_NOPROP) {
        return true;
    }

    if (_filter.valueMode != PROP_VALUE_MODE_SCALAR) {
        return false;
    }

    let lowerOK = select(
        _val > _filter.scalarRange.x,
        _val >= _filter.scalarRange.x,
        _filter.scalarIncl.x != 0u);

    let upperOK = select(
        _val < _filter.scalarRange.y,
        _val <= _filter.scalarRange.y,
        _filter.scalarIncl.y != 0u);

    let inRange = lowerOK && upperOK;

    var result = false;
    if (_filter.filterMode == PROP_FILTER_MODE_INSIDE_RANGE) {
        result |= inRange;
    } else if (_filter.filterMode == PROP_FILTER_MODE_NAN) {
        result |= isNan(_val);
    } else if (_filter.filterMode == PROP_FILTER_MODE_INF) {
        result |= isInf(_val);
    }

    if (_filter.invert != 0u) {
        result = !result;
    }

    return result;
}

//fn isOutsideValueFilter(
//    value:PropValue,
//    valueType:ValueMode,
//    valueFilter:vec2<f32>) -> bool
//{
//    return valueType==VALUE_MODE_SCALAR
//        && !isInf(value.x)
//        && (value.x < valueFilter.x || value.x > valueFilter.y);
//};

fn getFragmentColorFromPropertyValue(
    _val:PropValue,
    _filter:PropFilterOptions,
    _colorMap : texture_2d<f32>,
    _colorMapSampler: sampler
) -> vec4<f32>
{
    var color = vec4<f32>(1,0.9,0.9,0);

    // Interpret Value based on Property Mode
    if (_filter.valueMode == PROP_VALUE_MODE_NOPROP)
    {
        color = vec4<f32>(1.0);
    }
    else if (_filter.valueMode == PROP_VALUE_MODE_SCALAR)
    {
        let rangeSize = _filter.scalarRange.y - _filter.scalarRange.x;
        var t = 0.5;
        if (rangeSize > 0.0) {
            t = clamp(
                (_val - _filter.scalarRange.x) / rangeSize,
                0.0,
                1.0
            );
        }
        color = textureSample(_colorMap, _colorMapSampler, vec2<f32>(t,0.5));
    }
    else if (_filter.valueMode == PROP_VALUE_MODE_VEC3)
    {
        color = vec4<f32>(_val, _val, _val, 1.0);
    }

    if (isNan(_val)) {
        color = COLOR_NAN;
    } else if (isInf(_val)) {
        color = COLOR_INF;
    }

    return color;
}
