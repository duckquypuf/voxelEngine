#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

#include "noise.h"

#include "voxelData.h"
#include "chunk.h"

class World
{
public:
    std::unordered_map<ChunkCoord, Chunk*> chunks;
    std::queue<ChunkCoord> chunksToGenerate;
    std::unordered_set<ChunkCoord> chunksInQueue;

    World()
    {
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFrequency(0.02f);
    }

    void updateRenderDistance(ChunkCoord centre, bool regenAll = false)
    {
        if(!regenAll)
        {
            for (int x = centre.x - renderDistance - 1; x < centre.x + renderDistance + 1; x++)
            {
                for (int z = centre.z - renderDistance - 1; z < centre.z + renderDistance + 1; z++)
                {
                    if (x < 0 || z < 0 || x >= worldWidth || z >= worldWidth)
                        continue;
                        
                    ChunkCoord coord(x, z);

                    if (chunks.find(coord) == chunks.end())
                    {
                        chunks[coord] = new Chunk(this, coord, true);
                        chunks[coord]->populateVoxelMap();
                    }
                }
            }
            
            for (int x = centre.x - renderDistance; x < centre.x + renderDistance; x++)
            {
                for (int z = centre.z - renderDistance; z < centre.z + renderDistance; z++)
                {
                    if (x < 0 || z < 0 || x >= worldWidth || z >= worldWidth)
                        continue;
                        
                    ChunkCoord coord(x, z);
                    auto it = chunks.find(coord);

                    if (it != chunks.end() && it->second != nullptr && it->second->shouldRegen && chunksInQueue.find(coord) == chunksInQueue.end())
                    {
                        chunksToGenerate.push(coord);
                        chunksInQueue.insert(coord);
                    }
                }
            }
        } else
        {
            for (int x = centre.x - renderDistance - 1; x < centre.x + renderDistance + 1; x++)
            {
                for (int z = centre.z - renderDistance - 1; z < centre.z + renderDistance + 1; z++)
                {
                    if (x < 0 || z < 0 || x >= worldWidth || z >= worldWidth)
                        continue;

                    ChunkCoord coord(x, z);

                    chunks[coord] = new Chunk(this, coord, true);
                    chunks[coord]->populateVoxelMap();
                }
            }

            for (int x = centre.x - renderDistance; x < centre.x + renderDistance; x++)
            {
                for (int z = centre.z - renderDistance; z < centre.z + renderDistance; z++)
                {
                    if (x < 0 || z < 0 || x >= worldWidth || z >= worldWidth)
                        continue;

                    ChunkCoord coord(x, z);

                    chunksToGenerate.push(coord);
                    chunksInQueue.insert(coord);
                }
            }
        }

        generateCaves(centre);
        generateLodes(centre);
        generateTrees(centre);
    }

    ChunkCoord getChunkCoordFromVec3(glm::vec3 pos)
    {
        int x = floor(pos.x);
        int y = floor(pos.y);
        int z = floor(pos.z);

        ChunkCoord coord = ChunkCoord();

        coord.x = floor(x / chunkWidth);
        coord.z = floor(z / chunkWidth);

        int localX = x % chunkWidth;
        int localZ = z % chunkWidth;

        if (coord.x < 0 || pos.x < 0 || coord.z < 0 || pos.z < 0)
            return ChunkCoord();

        return coord;
    }

    bool isVoxelSolid(ChunkCoord coord, int localX, int localY, int localZ)
    {
        int chunkX = coord.x;
        int chunkZ = coord.z;

        if(localX < 0)
        {
            chunkX--;
            localX = chunkWidth-1;
        } else if(localX > chunkWidth-1)
        {
            chunkX++;
            localX = 0;
        }

        if (localZ < 0)
        {
            chunkZ--;
            localZ = chunkWidth - 1;
        }
        else if (localZ > chunkWidth - 1)
        {
            chunkZ++;
            localZ = 0;
        }

        if(localY < 0 || localY > chunkHeight-1) return false;
        if(chunkX < 0 || chunkZ < 0 || chunkX > worldWidth-1 || chunkZ > worldWidth-1) return true;

        ChunkCoord real(chunkX, chunkZ);
        return chunks[real]->voxelMap[localX][localY][localZ].isSolid;
    }

    bool isVoxelSolid(int worldX, int worldY, int worldZ)
    {
        ChunkCoord coord = ChunkCoord();

        coord.x = floor(worldX / chunkWidth);
        coord.z = floor(worldZ / chunkWidth);

        int localX = worldX % chunkWidth;
        int localZ = worldZ % chunkWidth;

        return isVoxelSolid(coord, localX, worldY, localZ);
    }

    bool isVoxelTransparent(ChunkCoord coord, int localX, int localY, int localZ)
    {
        int chunkX = coord.x;
        int chunkZ = coord.z;

        if (localX < 0)
        {
            chunkX--;
            localX = chunkWidth - 1;
        }
        else if (localX > chunkWidth - 1)
        {
            chunkX++;
            localX = 0;
        }

        if (localZ < 0)
        {
            chunkZ--;
            localZ = chunkWidth - 1;
        }
        else if (localZ > chunkWidth - 1)
        {
            chunkZ++;
            localZ = 0;
        }

        if (localY < 0 || localY > chunkHeight - 1)
            return false;
        if (chunkX < 0 || chunkZ < 0 || chunkX > worldWidth - 1 || chunkZ > worldWidth - 1)
            return true;

        ChunkCoord real(chunkX, chunkZ);
        return chunks[real]->voxelMap[localX][localY][localZ].isTransparent;
    }

    bool checkForVoxel(float worldX, float worldY, float worldZ)
    {
        int x = floor(worldX);
        int y = floor(worldY);
        int z = floor(worldZ);

        ChunkCoord coord = ChunkCoord();

        coord.x = floor(x / chunkWidth);
        coord.z = floor(z / chunkWidth);

        int localX = x % chunkWidth;
        int localZ = z % chunkWidth;

        if(coord.x < 0 || worldX < 0 || coord.z < 0 || worldZ < 0 || worldY < 0 || worldY > chunkHeight-1) return false;

        return !chunks[coord]->voxelMap[localX][y][localZ].isAir;
    }

    BlockType getVoxel(int worldX, int worldY, int worldZ)
    {
        int x = floor(worldX);
        int y = floor(worldY);
        int z = floor(worldZ);

        ChunkCoord coord = ChunkCoord();

        coord.x = floor(x / chunkWidth);
        coord.z = floor(z / chunkWidth);

        int localX = x % chunkWidth;
        int localZ = z % chunkWidth;

        if (coord.x < 0 || worldX < 0 || coord.z < 0 || worldZ < 0 || worldY < 0 || worldY > chunkHeight - 1)
            return blockTypes[0]; // Air

        return chunks[coord]->voxelMap[localX][y][localZ];
    }

    void generateTrees(ChunkCoord centre)
    {
        bool needsTreeGen = false;
        for (int cx = centre.x - renderDistance; cx < centre.x + renderDistance; cx++)
        {
            for (int cz = centre.z - renderDistance; cz < centre.z + renderDistance; cz++)
            {
                if (cx < 0 || cz < 0 || cx >= worldWidth || cz >= worldWidth)
                    continue;

                ChunkCoord coord(cx, cz);
                auto it = chunks.find(coord);
                if (it != chunks.end() && it->second != nullptr && !it->second->treesGenerated)
                {
                    needsTreeGen = true;
                    break;
                }
            }
            if (needsTreeGen)
                break;
        }

        if (!needsTreeGen)
            return;

        for (int cx = centre.x - renderDistance; cx < centre.x + renderDistance; cx++)
        {
            for (int cz = centre.z - renderDistance; cz < centre.z + renderDistance; cz++)
            {
                if (cx < 0 || cz < 0 || cx >= worldWidth || cz >= worldWidth)
                    continue;

                ChunkCoord coord(cx, cz);
                auto it = chunks.find(coord);
                if (it == chunks.end() || it->second == nullptr || it->second->treesGenerated)
                    continue;

                for (int x = 0; x < chunkWidth; x++)
                {
                    for (int z = 0; z < chunkWidth; z++)
                    {
                        float treeZone01 = getPerlinNoise(coord.x * chunkWidth + x + treeZoneOffset, coord.z * chunkWidth + z + treeZoneOffset, treeZoneScale);

                        if (treeZone01 > treeZoneThreshold)
                        {
                            float treePlacement01 = getPerlinNoise(coord.x * chunkWidth + x + treePlacementOffset, coord.z * chunkWidth + z + treePlacementOffset, treePlacementScale);

                            if (treePlacement01 > treePlacementThreshold)
                            {
                                // Find the ground height
                                float heightValue01 = getPerlinNoise(coord.x * chunkWidth + x, coord.z * chunkWidth + z, biomeScale);
                                int heightValue = heightValue01 * terrainHeight + terrainMinHeight;

                                if(getVoxel(coord.x*chunkWidth + x, heightValue, coord.z*chunkWidth + z) != blockTypes[1]) // Grass
                                {
                                    continue;
                                }

                                int treeY = heightValue + 1;
                                int treeHeight = treeMinHeight + (rand() % (treeMaxHeight - treeMinHeight));

                                if (treeY + treeHeight < chunkHeight)
                                {
                                    int worldX = coord.x * chunkWidth + x;
                                    int worldZ = coord.z * chunkWidth + z;

                                    // Place trunk
                                    for (int i = 0; i < treeHeight; i++)
                                        placeTreeVoxel(worldX, treeY + i, worldZ, 4); // Oak Log

                                    // Place leaves
                                    for (int lx = -2; lx <= 2; lx++)
                                    {
                                        for (int lz = -2; lz <= 2; lz++)
                                        {
                                            for (int ly = treeHeight - 3; ly < treeHeight-1; ly++)
                                            {
                                                if(lx != 0 || lz != 0)
                                                    placeTreeVoxel(worldX + lx, treeY + ly, worldZ + lz, 5); // Oak Leaves
                                            }
                                        }
                                    }

                                    for (int lx = -1; lx <= 1; lx++)
                                    {
                                        for (int lz = -1; lz <= 1; lz++)
                                        {
                                            if (lx != 0 || lz != 0)
                                                placeTreeVoxel(worldX + lx, treeY + treeHeight-1, worldZ + lz, 5); // Oak Leaves
                                        }
                                    }

                                    placeTreeVoxel(worldX + 1, treeY + treeHeight, worldZ, 5); // Oak Leaves
                                    placeTreeVoxel(worldX, treeY + treeHeight, worldZ + 1, 5); // Oak Leaves
                                    placeTreeVoxel(worldX - 1, treeY + treeHeight, worldZ, 5); // Oak Leaves
                                    placeTreeVoxel(worldX, treeY + treeHeight, worldZ - 1, 5); // Oak Leaves
                                    placeTreeVoxel(worldX, treeY + treeHeight, worldZ, 5); // Oak Leaves

                                    it->second->shouldRegen = true;
                                }
                            }
                        }
                    }
                }

                it->second->treesGenerated = true;
            }
        }
    }

    void placeTreeVoxel(int worldX, int worldY, int worldZ, int blockType)
    {
        ChunkCoord coord;
        coord.x = floor(worldX / chunkWidth);
        coord.z = floor(worldZ / chunkWidth);

        int localX = worldX % chunkWidth;
        int localZ = worldZ % chunkWidth;

        if (coord.x < 0 || coord.z < 0 || coord.x >= worldWidth || coord.z >= worldWidth)
            return;
        if (worldY < 0 || worldY >= chunkHeight)
            return;

        auto it = chunks.find(coord);
        if (it != chunks.end() && it->second != nullptr)
        {
            if (it->second->voxelMap[localX][worldY][localZ].isAir)
            {
                it->second->voxelMap[localX][worldY][localZ] = blockTypes[blockType];
            }
        }
    }

    void generateCaves(ChunkCoord centre)
    {
        bool needsCaveGen = false;
        for (int cx = centre.x - renderDistance; cx < centre.x + renderDistance; cx++)
        {
            for (int cz = centre.z - renderDistance; cz < centre.z + renderDistance; cz++)
            {
                if (cx < 0 || cz < 0 || cx >= worldWidth || cz >= worldWidth)
                    continue;

                ChunkCoord coord(cx, cz);
                auto it = chunks.find(coord);
                if (it != chunks.end() && it->second != nullptr && !it->second->cavesGenerated)
                {
                    needsCaveGen = true;
                    break;
                }
            }
            if (needsCaveGen)
                break;
        }

        if (!needsCaveGen)
            return;

        for (int cx = centre.x - renderDistance; cx < centre.x + renderDistance; cx++)
        {
            for (int cz = centre.z - renderDistance; cz < centre.z + renderDistance; cz++)
            {
                if (cx < 0 || cz < 0 || cx >= worldWidth || cz >= worldWidth)
                    continue;

                ChunkCoord coord(cx, cz);
                auto it = chunks.find(coord);
                if (it == chunks.end() || it->second == nullptr || it->second->cavesGenerated)
                    continue;

                bool chunkModified = false;

                for (int x = 0; x < chunkWidth; x++)
                {
                    for (int y = 1; y < chunkHeight - 1; y++)
                    {
                        for (int z = 0; z < chunkWidth; z++)
                        {
                            int worldX = coord.x * chunkWidth + x;
                            int worldZ = coord.z * chunkWidth + z;

                            float caveNoise = getCaveNoise(worldX, y, worldZ);

                            if (caveNoise > caveGenThreshold && !it->second->voxelMap[x][y][z].isAir)
                            {
                                it->second->voxelMap[x][y][z] = blockTypes[0];
                                chunkModified = true;
                            }
                        }
                    }
                }

                if (chunkModified)
                {
                    it->second->shouldRegen = true;
                }

                it->second->cavesGenerated = true;
            }
        }
    }

    void generateLodes(ChunkCoord centre)
    {
        bool needsLodeGen = false;
        for (int cx = centre.x - renderDistance; cx < centre.x + renderDistance; cx++)
        {
            for (int cz = centre.z - renderDistance; cz < centre.z + renderDistance; cz++)
            {
                if (cx < 0 || cz < 0 || cx >= worldWidth || cz >= worldWidth)
                    continue;

                ChunkCoord coord(cx, cz);
                auto it = chunks.find(coord);
                if (it != chunks.end() && it->second != nullptr && !it->second->lodesGenerated)
                {
                    needsLodeGen = true;
                    break;
                }
            }
            if (needsLodeGen)
                break;
        }

        if (!needsLodeGen)
            return;

        for (int cx = centre.x - renderDistance; cx < centre.x + renderDistance; cx++)
        {
            for (int cz = centre.z - renderDistance; cz < centre.z + renderDistance; cz++)
            {
                if (cx < 0 || cz < 0 || cx >= worldWidth || cz >= worldWidth)
                    continue;

                ChunkCoord coord(cx, cz);
                auto it = chunks.find(coord);
                if (it == chunks.end() || it->second == nullptr || it->second->lodesGenerated)
                    continue;

                bool chunkModified = false;

                for (int x = 0; x < chunkWidth; x++)
                {
                    for (int y = 1; y < chunkHeight - 1; y++)
                    {
                        for (int z = 0; z < chunkWidth; z++)
                        {
                            int worldX = coord.x * chunkWidth + x;
                            int worldZ = coord.z * chunkWidth + z;

                            for(int l = 0; l < lodeCount; l++)
                            {
                                const Lode& lode = lodes[l];

                                if(y < lode.minHeight || y > lode.maxHeight) continue;

                                float lodeNoise = getPerlinNoise3D(worldX + lode.offset, y, worldZ + lode.offset, lode.scale);

                                if (lodeNoise > lode.threshold && it->second->voxelMap[x][y][z] == blockTypes[3])
                                {
                                    it->second->voxelMap[x][y][z] = blockTypes[lode.blockID];
                                    chunkModified = true;
                                }
                            }
                        }
                    }
                }

                if (chunkModified)
                {
                    it->second->shouldRegen = true;
                }

                it->second->lodesGenerated = true;
            }
        }
    }

    int genVoxel(ChunkCoord coord, int x, int y, int z)
    {
        /* TERRAIN PASS */
        float heightValue01 = getPerlinNoise(coord.x * chunkWidth + x, coord.z * chunkWidth + z, biomeScale);
        int heightValue = heightValue01 * terrainHeight + terrainMinHeight;

        int voxel = 0;

        if (y > heightValue)
        {
            voxel = 0; // Air
        }
        else if (y == heightValue)
        {
            return 1; // Grass
        }
        else if (y >= heightValue - 4 && y < heightValue)
        {
            return 2; // Dirt
        }
        else if (y < heightValue - 4)
        {
            return 3; // Stone
        }

        return voxel;
    }

    void setVoxel(float worldX, float worldY, float worldZ, int block)
    {
        int x = floor(worldX);
        int y = floor(worldY);
        int z = floor(worldZ);

        ChunkCoord coord = ChunkCoord();

        coord.x = floor(x / chunkWidth);
        coord.z = floor(z / chunkWidth);

        int localX = x % chunkWidth;
        int localZ = z % chunkWidth;

        if (coord.x < 0 || worldX < 0 || coord.z < 0 || worldZ < 0 || worldY < 0 || worldY > chunkHeight - 1)
            return;

        chunks[coord]->setVoxel(localX, y, localZ, block);
    }
    
    void regenerateChunks(ChunkCoord origin, int localX, int localZ)
    {
        chunks[origin]->generateMesh();

        if(localX == 0 && origin.x > 0)
        {
            chunks[ChunkCoord(origin.x-1, origin.z)]->generateMesh();
        }

        if(localZ == 0 && origin.z > 0)
        {
            chunks[ChunkCoord(origin.x, origin.z-1)]->generateMesh();
        }

        if(localX == chunkWidth-1 && origin.x < worldWidth-1)
        {
            chunks[ChunkCoord(origin.x+1, origin.z)]->generateMesh();
        }

        if(localZ == chunkWidth - 1 && origin.z < worldWidth - 1)
        {
            chunks[ChunkCoord(origin.x, origin.z+1)]->generateMesh();
        }
    }
};
