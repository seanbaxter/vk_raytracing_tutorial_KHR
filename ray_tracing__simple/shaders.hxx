#pragma once

struct raytrace_shaders_t {
  const char* module_data;
  size_t module_size;

  const char* rgen;
  const char* rmiss;
  const char* rmiss_shadow;
  const char* rchit;
};

extern raytrace_shaders_t raytrace_shaders;

struct shadow_shaders_t {
  const char* module_data;
  size_t module_size;

  const char* rmiss_shadow;
};
extern shadow_shaders_t shadow_shaders;

struct raster_shaders_t {
  const char* module_data;
  size_t module_size;

  const char* vert;
  const char* frag;
  const char* frag_post;
};
