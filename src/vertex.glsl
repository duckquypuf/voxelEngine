#version 410

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in int aTexID;

out vec3 normal;
out vec2 texCoord;
flat out int texID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    normal = mat3(transpose(inverse(model))) * aNormal;
    texCoord = aTexCoord;
    texID = int(aTexID);
}
