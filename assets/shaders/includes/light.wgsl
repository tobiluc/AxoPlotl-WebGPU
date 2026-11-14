
struct DirectionalLight {
    @align(16) color: vec3<f32>,
    @align(16) dir: vec3<f32>,
    @align(16) strength: f32
};

fn getLightContribution(
    l:DirectionalLight,
    n:vec3<f32>) -> vec3<f32>
{
    // Directional diffuse lighting
    let N = normalize(n);
    let L = normalize(-l.dir);
    let diffuse = max(dot(N, L), 0.0);
    let lighting = l.color
                   * l.strength * diffuse;
    return lighting;
};
