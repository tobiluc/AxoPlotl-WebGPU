
struct LineQuadCorner
{
    // In Normalized Device Coordinates
    position: vec4<f32>,

    // Relative to Edge Origin
    // in [-0.5*L, +0.5*L] x [-0.5*W, +0.5*W]
    quadCorner: vec2<f32>,

    // x: Edge Length (L), y: Line Width (W)
    quadSize: vec2<f32>,

    // Interpolation t along edge in [0,1]
    t:f32
};

fn getLineQuadCorner(
    _clip0:vec4<f32>,
    _clip1:vec4<f32>,
    _corner_i:u32,
    _viewportSize:vec2<f32>,
    _lineWidth:f32) -> LineQuadCorner
{
    //-------------------------------
    // An Edge instance is rendered
    // as a thin quad with width
    // given in screen space
    //-------------------------------

    var res : LineQuadCorner;

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

    let config = configs[_corner_i];
    res.t = config.x;
    let side = config.y;

    // Convert NDC to pixel coordinates.
    // Y is inverted because screen coordinates usually grow downward.
    let ndc0 = _clip0.xy / _clip0.w;
    let ndc1 = _clip1.xy / _clip1.w;

    // Convert to pixel coordinates
    let screen0 = ndc0 * ubo.viewportSize * 0.5;
    let screen1 = ndc1 * ubo.viewportSize * 0.5;

    let edgeVector = screen1 - screen0;
    let edgeLength = length(edgeVector);

    let edgeDir = edgeVector / max(edgeLength, 0.0001);
    let perpDir = vec2<f32>(-edgeDir.y, edgeDir.x);

    // Half-width in pixels
    let offsetPixels = perpDir * (_lineWidth * 0.5 * side);

    // Convert back to NDC
    let offsetNdc = offsetPixels * 2.0 / _viewportSize;

    var outPos = mix(_clip0, _clip1, res.t);

    // Apply offset in clip space.
    outPos.x += offsetNdc.x * outPos.w;
    outPos.y += offsetNdc.y * outPos.w;

    res.position = outPos;
    res.quadSize = vec2<f32>(edgeLength, ubo.lineWidth);
    res.quadCorner = vec2<f32>(
        (config.x - 0.5) * edgeLength,
        0.5 * config.y * ubo.lineWidth
        );

    return res;
}

fn getLineQuadShading(
    _quadCorner: vec2<f32>,
    _quadSize: vec2<f32>
) -> f32
{
    // Fake 3D
    let y = _quadCorner.y / (_quadSize.y * 0.5);
    let shading = 0.78 + 0.22 * sqrt(max(0.0, 1.0 - y * y));
    let highlight = 0.10 * exp(-pow((y + 0.45) * 12.0, 2.0));
    let shadow = 0.12 * smoothstep(0.1, 1.0, y);
    let finalShading = shading + highlight - shadow;
    return finalShading;
}
