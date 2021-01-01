#include "shaders.hxx"

template<typename type_t, int location>
[[using spirv: rayPayloadIn, location(location)]]
type_t shader_rayPayloadIn;

[[spirv::rmiss]]
void rmiss_shadow_shader() {
  // The rmiss shader for the glray_Trace query sent by rchit.
  shader_rayPayloadIn<bool, 1> = false;
}

// This struct hash external linkage.
shadow_shaders_t shadow_shaders {
  __spirv_data,
  __spirv_size,
  @spirv(rmiss_shadow_shader),
}; 