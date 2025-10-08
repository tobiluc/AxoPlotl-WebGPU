struct Uniforms {
    mvp : mat4x4<f32>, // model view projecion matrix
};

@group(0) @binding(0)
var<uniform> uniforms : Uniforms;

struct VertexInput {
    @location(0) position : vec3f,
    @location(1) color : vec3f,
};

struct VertexOutput {
    @builtin(position) position : vec4f,
    @location(0) color : vec3f, // pass to fragment shader
};

@vertex
fn vs_main(input : VertexInput) -> VertexOutput {
    var output : VertexOutput;
    output.position = uniforms.mvp * vec4f(input.position, 1.0);
    output.color = input.color; // forward to fragment stage
    return output;
}

@fragment
fn fs_main(@location(0) color : vec3f) -> @location(0) vec4f {
    return vec4f(color, 1.0);
}