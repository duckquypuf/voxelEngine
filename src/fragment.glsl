#version 410 core

in vec2 texCoord;
in vec3 normal;
flat in int texID;

out vec4 FragColor;

uniform sampler2D atlasSampler;

const int ATLAS_COLS = 16;
const int ATLAS_ROWS = 16;

void main()
{
    float tileW = 1.0 / ATLAS_COLS;
    float tileH = 1.0 / ATLAS_ROWS;

    int tileX = texID % ATLAS_COLS;
    int tileY = texID / ATLAS_COLS;

    vec2 baseUV = vec2(tileX * tileW, tileY * tileH);
    vec2 finalUV = baseUV + texCoord * vec2(tileW, tileH);

    vec4 tex = texture(atlasSampler, finalUV);

    if(tex.a == 0)
        discard;
    
    float lightLevel = 1.0;
    
    if(normal.y > 0.5)
        lightLevel = 1.0;
    
    else if(normal.y < -0.5)
        lightLevel = 0.5;
    
    else if(abs(normal.z) > 0.5)
        lightLevel = 0.8;
    
    else if(abs(normal.x) > 0.5)
        lightLevel = 0.6;

    FragColor = tex * vec4(vec3(lightLevel), 1.0);
}
