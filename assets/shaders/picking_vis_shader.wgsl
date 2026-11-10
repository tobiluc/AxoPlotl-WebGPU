struct Uniforms
{
    @align(16) mode: u32
};

@group(0) @binding(0)
var picking_texture: texture_2d<u32>;

@group(0) @binding(1)
var<uniform> uniforms: Uniforms;

struct VertexOutput
{
    @builtin(position) position: vec4<f32>,
};

@vertex
fn vs_main(
    @builtin(vertex_index) index: u32
) -> VertexOutput
{
    var positions = array<vec2<f32>, 3>(
        vec2<f32>(-1.0, -1.0),
        vec2<f32>( 3.0, -1.0),
        vec2<f32>(-1.0,  3.0)
    );

    var output: VertexOutput;
    output.position = vec4<f32>(
        positions[index], 0.0, 1.0
    );

    return output;
}

fn hash_color(id: u32) -> vec3<f32>
{
    let r = f32((id * 97u) % 255u) / 255.0;
    let g = f32((id * 57u) % 255u) / 255.0;
    let b = f32((id * 23u) % 255u) / 255.0;

    return vec3<f32>(r, g, b);
}

@fragment
fn fs_main(
    @builtin(position) position: vec4<f32>
) -> @location(0) vec4<f32>
{
    let pixel = vec2<i32>(position.xy);

    let data = textureLoad(
        picking_texture,
        pixel,
        0
    );

    var color = vec3<f32>(0.0);

    switch uniforms.mode
    {
        case 0u: {
            // Object Id
            color = hash_color(data.r);
        }

        case 1u: {
            // Entity Type
            color = hash_color(data.g);
        }

        case 2u: {
            // Entity Index
            color = hash_color(data.b);
        }

        case 3u: {
            // Depth
            color = vec3<f32>(bitcast<f32>(data.a));
        }

        default: {
            color = vec3<f32>(1.0, 0.0, 1.0);
        }
    }

    // Zero represents the background.
    if (data.r == 0u && uniforms.mode == 0u)
    {
        color = vec3<f32>(0.0);
    }

    return vec4<f32>(color, 1.0);
}
