// This Shader is used for debug purposes,
// to visualize the Picking Texture.

alias PickMode = u32;
const PICK_MODE_OBJECT_ID:PickMode = 0u;
const PICK_MODE_ENTITY_TYPE:PickMode = 1u;
const PICK_MODE_ENTITY_INDEX:PickMode = 2u;
const PICK_MODE_DEPTH:PickMode = 3u;

struct Uniforms
{
    @align(16) mode: PickMode
};

@group(0) @binding(0)
var picking_texture: texture_2d<u32>;

@group(0) @binding(1)
var<uniform> ubo: Uniforms;

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

    switch ubo.mode {
        case PICK_MODE_OBJECT_ID: {
            color = hash_color(data.r);
        }
        case PICK_MODE_ENTITY_TYPE: {
            color = hash_color(data.g);
        }
        case PICK_MODE_ENTITY_INDEX: {
            color = hash_color(data.b);
        }
        case PICK_MODE_DEPTH: {
            let depth = bitcast<f32>(data.a);
            color = vec3<f32>(depth);
        }
        default: {
            color = vec3<f32>(1.0, 0.0, 1.0);
        }
    }

    return vec4<f32>(color, 1.0);
}
