#include "shader_common.hxx"

[[spirv::vert]]
void vert_shader() {
  // Draw a full quad from the vertex index.
  vec2 uv((glvert_VertexIndex<< 1) & 2, glvert_VertexIndex & 2);

  // Pass the uv to location 0.
  shader_out<vec2, 0> = uv;

  // Pass the position.
  glvert_Output.Position = vec4(2 * uv - 1, 1, 1);
}

[[spirv::frag]]
void frag_shader() {
  vec2 uv = shader_in<vec2, 0>;
  float gamma = 1 / 2.2;

  vec4 fragColor = pow(texture(shader_uniform<sampler2D, 0>, uv), gamma);
  shader_out<vec4, 0> = fragColor;
}

post_shaders_t post_shaders {
  __spirv_data,
  __spirv_size,
  @spirv(vert_shader),
  @spirv(frag_shader)
};