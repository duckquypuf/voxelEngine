#version 410 core
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec3 worldPos;
} gs_in[];

uniform mat4 view;
uniform mat4 projection;
uniform float lineWidth; // in pixels

void main()
{
    vec4 p0 = projection * view * vec4(gs_in[0].worldPos, 1.0);
    vec4 p1 = projection * view * vec4(gs_in[1].worldPos, 1.0);
    
    // Perspective divide for NDC
    vec2 ndc0 = p0.xy / p0.w;
    vec2 ndc1 = p1.xy / p1.w;
    
    // Direction vector
    vec2 dir = normalize(ndc1 - ndc0);
    vec2 perp = vec2(-dir.y, dir.x);
    
    // Convert line width from pixels to NDC
    vec2 offset = perp * lineWidth / vec2(1440.0, 900.0);
    
    // Emit quad (two triangles)
    gl_Position = vec4((ndc0 - offset) * p0.w, p0.z, p0.w);
    EmitVertex();
    
    gl_Position = vec4((ndc0 + offset) * p0.w, p0.z, p0.w);
    EmitVertex();
    
    gl_Position = vec4((ndc1 - offset) * p1.w, p1.z, p1.w);
    EmitVertex();
    
    gl_Position = vec4((ndc1 + offset) * p1.w, p1.z, p1.w);
    EmitVertex();
    
    EndPrimitive();
}