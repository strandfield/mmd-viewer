#version 330 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 color;

out vec4 v_color;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

uniform float opacity;

void main()
{
    gl_Position = projection_matrix * model_view_matrix * vec4(position, 1.0);
    v_color = vec4(color.rgb / 255.0f, opacity);
}
