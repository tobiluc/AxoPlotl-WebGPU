
fn getDepthCorrection(
    _posW : f32, // homogenous coordinate
    _screenDepthCorrection : f32,
    _proj : mat4x4<f32>,
    _viewPortHeight: f32
) -> f32
{
    // Perspective-correct screen-space sphere depth
    let focalLength = _proj[1][1];
    let k = _screenDepthCorrection / (_viewPortHeight * focalLength);
    let B = _proj[3][2];
    let depthCorrection =
        B * _posW *
        (1.0 / (1.0 - k) - 1.0);
    return depthCorrection;
}
