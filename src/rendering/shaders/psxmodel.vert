#version 330 core

layout(location = 0) in vec3 position;

#if defined(MESH_HAS_COLORS)
layout(location = 2) in vec3 color;
#endif

#if defined(MESH_HAS_UV)
layout(location = 3) in vec2 uv;
#endif

#if defined(MESH_HAS_NORMALS)
layout(location = 4) in vec3 normal;
#endif

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

#if defined(MESH_HAS_COLORS)
out vec3 v_color;
#endif

#if defined(MESH_HAS_UV)
out vec2 v_uv;
#endif

#if defined(MESH_HAS_NORMALS)
out vec3 v_normal;
#endif

void main()
{
    vec4 model_pos = vec4(position, 1.0);
    gl_Position = projection_matrix * view_matrix * model_matrix * model_pos;

#if defined(MESH_HAS_COLORS)
    v_color = color;
#endif

#if defined(MESH_HAS_UV)
    v_uv = uv;
#endif

#if defined(MESH_HAS_NORMALS)
    // The model's normals need to be transformed, but the transform's
    // scale need to be taken into account ; hence this non-trivial
    // formula.
    // See https://learnopengl.com/Lighting/Basic-Lighting
    v_normal = mat3(transpose(inverse(model_matrix))) * normal;
#endif
}
