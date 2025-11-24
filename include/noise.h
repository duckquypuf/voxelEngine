#pragma once

#include "FastNoiseLite.h"
#include "voxelData.h"

inline FastNoiseLite noise;

inline float getPerlinNoise(float x, float z, float scale)
{
    noise.SetFrequency(scale);
    float n = noise.GetNoise(x, z);
    n = (n + 1.0f) * 0.5f;
    return n;
}

inline float getPerlinNoise3D(float x, float y, float z, float scale)
{
    noise.SetFrequency(scale);
    float n = noise.GetNoise(x, y, z);
    n = (n + 1.0f) * 0.5f;
    return n;
}

inline float getCaveNoise(int worldX, int worldY, int worldZ)
{
    float noise1 = getPerlinNoise3D(worldX, worldY, worldZ, caveGenLargeScale);

    float noise2 = getPerlinNoise3D(worldX, worldY, worldZ, caveGenMediumScale);

    float noise3 = getPerlinNoise3D(worldX, worldY, worldZ, caveGenSmallScale);

    return noise1 * 0.5f + noise2 * 0.3f + noise3 * 0.2f;
}
