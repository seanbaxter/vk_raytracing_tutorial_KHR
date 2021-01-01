#include "shader_common.hxx"

// Buffers.
[[using spirv: uniform, binding(0), set(1)]]
camera_t cam;

[[using spirv: buffer, binding(1), set(1)]]
struct [[spirv::block]] {
  WaveFrontMaterial m[];
} materials[];

[[using spirv: buffer, binding(2), set(1)]]
SceneDesc sceneDescs[];

[[using spirv: uniform, binding(3), set(1)]]
sampler2D textureSamplers[];

[[using spirv: buffer, binding(4), set(1)]]
struct [[spirv::block]] {
  int i[];
} matIndices[];

[[using spirv: buffer, binding(5), set(1)]]
struct [[spirv::block]] {
  Vertex v[];
} vertices[];

[[using spirv: buffer, binding(6), set(1)]]
struct [[spirv::block]] {
  int i[];
} indices[];

// Ray tracing interface varibales.
[[using spirv: uniform, binding(0)]]
accelerationStructure topLevelAS;

[[using spirv: uniform, binding(1), format(rgba32f)]]
image2D image;

[[using spirv: hitAttribute]]
vec2 hit_attribs;

[[spirv::rgen]]
void rgen_shader() {
  vec2 pixelCenter = vec2(glray_LaunchID.xy) + vec2(0.5);
  vec2 inUV        = pixelCenter / vec2(glray_LaunchSize.xy);
  vec2 d           = 2 * inUV - 1;

  vec4 origin    = cam.viewInv * vec4(0, 0, 0, 1);
  vec4 target    = cam.projInv * vec4(d.x, d.y, 1, 1);
  vec4 direction = cam.viewInv * vec4(normalize(target.xyz), 0);

  uint  rayFlags = gl_RayFlagsOpaque;
  float tMin     = 0.001;
  float tMax     = 10000.0;

  glray_Trace(topLevelAS,     // acceleration structure
              rayFlags,       // rayFlags
              0xFF,           // cullMask
              0,              // sbtRecordOffset
              0,              // sbtRecordStride
              0,              // missIndex
              origin.xyz,     // ray origin
              tMin,           // ray min range
              direction.xyz,  // ray direction
              tMax,           // ray max range
              0               // payload (location = 0)
  );

  imageStore(image, ivec2(glray_LaunchID.xy), 
    vec4(shader_rayPayload<vec3, 0>, 1.0));
}

////////////////////////////////////////////////////////////////////////////////

[[spirv::rmiss]]
void rmiss_shader() {
  // Modulate the clear color.
  vec4 clearColor = shader_push<vec4>;
  shader_rayPayloadIn<vec3, 0> = clearColor.xyz * 0.8f;
}

[[spirv::rmiss]]
void rmiss_shadow_shader() {
  // The rmiss shader for the glray_Trace query sent by rchit.
  shader_rayPayloadIn<bool, 1> = false;
}

////////////////////////////////////////////////////////////////////////////////

struct Constants {
  float clearColor[4];
  vec3  lightPosition;
  float lightIntensity;
  int   lightType;
};

[[spirv::rchit]]
void rchit_shader() {
  // Object of this instance.
  SceneDesc desc = sceneDescs[glray_InstanceCustomIndex];
  int objId = desc.objId;

  mat4 transfo(desc.transfo);
  mat4 transfoIT(desc.transfoIT);

  // Get the push constants.
  Constants constants = shader_push<Constants>;

  // Note that nonuniformEXT is implicitly added by Circle when using a 
  // dynamic non-uniform index into a resource array.
  int indx = indices[objId].i[3 * glray_PrimitiveID + 0];
  int indy = indices[objId].i[3 * glray_PrimitiveID + 1];
  int indz = indices[objId].i[3 * glray_PrimitiveID + 2];

  Vertex v0 = vertices[objId].v[indx];
  Vertex v1 = vertices[objId].v[indy];
  Vertex v2 = vertices[objId].v[indz]; 

  vec3 bary(1 - hit_attribs.x - hit_attribs.y, hit_attribs.x, hit_attribs.y);

  // Interpolate vertex normals.
  vec3 normal = mat3(v0.nrm, v1.nrm, v2.nrm) * bary;
  normal = normalize((transfoIT * vec4(normal, 0)).xyz);

  // Interpolate vertex positions.
  vec3 worldPos = mat3(v0.pos, v1.pos, v2.pos) * bary;
  worldPos = (transfo * vec4(worldPos, 1)).xyz;

  vec3 L = normalize(constants.lightPosition - worldPos);
  float lightIntensity = constants.lightIntensity;
  float d = 100000.0f;

  if(constants.lightType == 0) {
    // Point light.
    vec3 lDir = constants.lightPosition - worldPos;
    d = length(lDir);
    lightIntensity = constants.lightIntensity / (d * d);
    L = normalize(lDir);

  } else {
    // Directional light.
    L = normalize(constants.lightPosition);
  }
  // Non-uniform accesses.
  int matIdx = matIndices[objId].i[glray_PrimitiveID];
  WaveFrontMaterial mat = materials[objId].m[matIdx];

  // Diffuse.
  vec3 diffuse = computeDiffuse(mat, L, normal);
  if(mat.textureId >= 0) {
    // Interpolate vertex uv coordinates.
    vec2 uv = mat3x2(vec2(v0.uv), vec2(v1.uv), vec2(v2.uv)) * bary;

    // Nonuniform access to textureSamplers resource array.
    int txtId = mat.textureId + desc.txtOffset;
    diffuse *= textureLod(textureSamplers[txtId], uv, 0).xyz;
  }

  vec3 specular = 0;
  float attenuation = 1;

  if(dot(normal, L) > 0) {
    float tMin = .001f;
    float tMax = d;
    vec3 origin = glray_WorldRayOrigin + glray_WorldRayDirection * glray_HitT;

    uint flags = gl_RayFlagsTerminateOnFirstHit | gl_RayFlagsOpaque | 
      gl_RayFlagsSkipClosestHitShader;

    // Mark the fragment as shadowed. rmiss_shadow_shadow will reset this to
    // false.
    shader_rayPayload<bool, 1> = true;

    glray_Trace(
      topLevelAS,  // acceleration structure
      flags,       // rayFlags
      0xFF,        // cullMask
      0,           // sbtRecordOffset
      0,           // sbtRecordStride
      1,           // missIndex
      origin,      // ray origin
      tMin,        // ray min range
      L,           // ray direction
      tMax,        // ray max range
      1            // payload (location = 1)
    );

    bool b = shader_rayPayload<bool, 1>;
    if(b) {
      attenuation = .3;
    } else {
      specular = computeSpecular(mat, glray_WorldRayDirection, L, normal);
    }
  }

  shader_rayPayloadIn<vec3, 0> = 
    lightIntensity * attenuation * (diffuse + specular);
}

// This struct hash external linkage.
raytrace_shaders_t raytrace_shaders {
  __spirv_data,
  __spirv_size,
  @spirv(rgen_shader),
  @spirv(rmiss_shader),
  0, // @spirv(rmiss_shadow_shader),
  @spirv(rchit_shader)
}; 