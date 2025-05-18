#version 330 core

#if defined(MESH_HAS_COLORS)
in vec3 v_color;
#endif

#if defined(MESH_HAS_UV)
in vec2 v_uv;
#endif

#if defined(MESH_HAS_NORMALS)
in vec3 v_normal;
#endif

uniform vec4 material_color;

#if defined(MATERIAL_TEXTURE)
uniform sampler2D texture_diffuse;
#endif

#if defined(LIGHTING_ON)
struct Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
};
uniform Light light;
#endif

out vec4 FragColor;

void main()
{
    float opacity = 1;
#if defined(MATERIAL_TEXTURE)
    vec4 texColor = texture(texture_diffuse, v_uv.xy);
    vec3 result_color = texColor.rgb;
    opacity = texColor.a;
#elif defined(MESH_HAS_COLORS)
    vec3 result_color = v_color / 255;
#else
    vec3 result_color = material_color.rgb;
#endif

#if defined(LIGHTING_ON) && defined(MESH_HAS_NORMALS)
    vec3 ambient = light.ambient * result_color;

    // diffuse
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * result_color;

    result_color = ambient + diffuse;
#endif

    FragColor = vec4(result_color, opacity);
}
