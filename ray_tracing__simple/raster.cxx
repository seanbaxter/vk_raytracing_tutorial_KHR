#include "shader_common.hxx"

[[using spirv: uniform, binding(0)]]
struct {
  mat4 view;
  mat4 proj;
  mat4 viewI;
} ubo;

struct Constants {
  vec3  lightPosition;
  uint  instanceId;
  float lightIntensity;
  int   lightType;
};


[[using spirv: buffer, binding(2)]]
SceneDesc sceneDescs[];

[[spirv::vert]]
void vert_shader() {
  // Load interface variables.
  Constants constants = shader_push<Constants>;
  vec3 position       = shader_in<vec3, 0>;
  vec3 normal         = shader_in<vec3, 1>;
  vec3 color          = shader_in<vec3, 2>;
  vec2 texCoord       = shader_in<vec2, 3>;

  SceneDesc desc = sceneDescs[constants.instanceId];
  mat4 objMatrix(desc.transfo);
  mat4 objMatrixIT(desc.transfoIT);

  vec3 origin = (ubo.viewI * vec4(0, 0, 0, 1)).xyz;
  vec3 worldPos = (objMatrix * vec4(shader_in<vec3, 0>, 1)).xyz;

  // Output interface variables.
  shader_out<vec2, 1> = texCoord;
  shader_out<vec3, 2> = (objMatrixIT * vec4(normal, 0)).xyz;
  shader_out<vec3, 3> = worldPos - origin;
  shader_out<vec3, 4> = worldPos;

  glvert_Output.Position = ubo.proj * (ubo.view * vec4(worldPos, 1));
}

[[using spirv: buffer, binding(1)]]
struct [[spirv::block]] {
  WaveFrontMaterial m[];
} materials[];

[[using spirv: uniform, binding(3)]]
sampler2D textureSamplers[];

[[using spirv: buffer, binding(4)]]
struct [[spirv::block]] {
  int i[];
} matIndices[];

[[spirv::frag]]
void frag_shader() {
  // Load interface variables.
  Constants constants = shader_push<Constants>;
  vec2 texCoord       = shader_in<vec2, 1>;
  vec3 normal         = shader_in<vec3, 2>;
  vec3 viewDir        = shader_in<vec3, 3>;
  vec3 worldPos       = shader_in<vec3, 4>;

  // Object of this instance.
  SceneDesc desc = sceneDescs[constants.instanceId];
  int objId = desc.objId;

  // Material of the object.
  // Note an implicit nonuniformEXT.
  int matIndex = matIndices[objId].i[glfrag_PrimitiveID];
  WaveFrontMaterial mat = materials[objId].m[matIndex];

  normal = normalize(normal);

  // Vector toward light
  vec3 L;
  float lightIntensity = constants.lightIntensity;
  if(constants.lightType == 0) {
    // Point light.
    vec3 lDir = constants.lightPosition - worldPos;
    float d = length(lDir);
    lightIntensity = constants.lightIntensity / (d * d);
    L = normalize(lDir);

  } else {
    // Directional light.
    L = normalize(constants.lightPosition);
  }

  // Diffuse
  vec3 diffuse = computeDiffuse(mat, L, normal);
  if(mat.textureId >= 0) {
    int txtId = desc.txtOffset + mat.textureId;

    // Note the implicit nonuniformEXT.
    diffuse *= texture(textureSamplers[txtId], texCoord).xyz;
  }

  // Specular.
  vec3 specular = computeSpecular(mat, viewDir, L, normal);

  // Write output color.
  shader_out<vec4, 0> = vec4(lightIntensity * (diffuse + specular), 1);
}

raster_shaders_t raster_shaders {
  __spirv_data,
  __spirv_size,
  @spirv(vert_shader),
  @spirv(frag_shader)
};