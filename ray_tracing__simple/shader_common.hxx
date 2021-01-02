#if __circle_build__ < 111
#error "Must compile with Circle build 111 or later"
#endif

#pragma once
#include "shaders.hxx"
#include <cmath>

template<typename type_t, int location>
[[using spirv: in, location(location)]]
type_t shader_in;

template<typename type_t, int location>
[[using spirv: out, location(location)]]
type_t shader_out;

template<typename type_t, int binding>
[[using spirv: uniform, binding(binding)]]
type_t shader_uniform;

template<typename type_t>
[[using spirv: push]]
type_t shader_push;

template<typename type_t, int location>
[[using spirv: rayPayload, location(location)]]
type_t shader_rayPayload;

template<typename type_t, int location>
[[using spirv: rayPayloadIn, location(location)]]
type_t shader_rayPayloadIn;

struct camera_t {
  mat4 view, proj, viewInv, projInv;
};

struct Vertex {
  vec3 pos;
  vec3 nrm;
  vec3 color;
  float uv[2];    // Circle's vec2 is 8-byte aligned, which differs from 
                  // nvmath's which generates this struct on the host side.
};

struct WaveFrontMaterial {
  vec3  ambient;
  vec3  diffuse;
  vec3  specular;
  vec3  transmittance;
  vec3  emission;
  float shininess;
  float ior;       // index of refraction
  float dissolve;  // 1 == opaque; 0 == fully transparent
  int   illum;     // illumination model (see http://www.fileformat.info/format/material/)
  int   textureId;
};

struct SceneDesc {
  int  objId;
  int  txtOffset;
  float transfo[16];
  float transfoIT[16];
};

inline vec3 computeDiffuse(WaveFrontMaterial mat, vec3 lightDir, vec3 normal) {
  // Lambertian
  float dotNL = max(dot(normal, lightDir), 0.f);
  vec3  c     = mat.diffuse * dotNL;
  if(mat.illum >= 1)
    c += mat.ambient;
  return c;
}

inline vec3 computeSpecular(WaveFrontMaterial mat, vec3 viewDir, vec3 lightDir, 
  vec3 normal) {

  vec3 v { };
  if(mat.illum >= 2) {
    // Compute specular only if not in shadow.
    const float kShininess = max(mat.shininess, 4.f);

    // Specular
    float kEC      = (2 + kShininess) / (2 * M_PIf32);
    vec3  V        = normalize(-viewDir);
    vec3  R        = reflect(-lightDir, normal);
    float specular = kEC * pow(max(dot(V, R), 0.f), kShininess);

    v = mat.specular * specular;
  }

  return v;
}
