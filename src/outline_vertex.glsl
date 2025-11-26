#version 410 core
layout(location = 0) in vec3 aPos;

out VS_OUT {
    vec3 worldPos;
} vs_out;

uniform mat4 model;

void main()
{
    vs_out.worldPos = (model * vec4(aPos, 1.0)).xyz;
}