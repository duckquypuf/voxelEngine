#pragma once
#include <glm/glm.hpp>
#include <string>
#include <math.h>
#include <vector>
#include <iostream>

constexpr unsigned int CHUNK_WIDTH = 16;
constexpr unsigned int CHUNK_HEIGHT = 16;
constexpr unsigned int WORLD_WIDTH = 5;

const glm::ivec3 faceChecks[] = {
    glm::ivec3(0, 0, -1), // Front - 0
    glm::ivec3(0, 0, 1),  // Back - 1
    glm::ivec3(-1, 0, 0), // Left - 2
    glm::ivec3(1, 0, 0),  // Right - 3
    glm::ivec3(0, -1, 0), // Bottom - 4
    glm::ivec3(0, 1, 0)   // Top - 5
};

enum FaceDirection {
    FRONT = 0,
    BACK = 1,
    LEFT = 2,
    RIGHT = 3,
    BOTTOM = 4,
    TOP = 5
};

enum BlockType : unsigned char {
    AIR = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
};

struct BlockInfo {
    bool isSolid;
    unsigned int textures[6]; // [Front, Back, Left, Right, Bottom, Top]
    std::string name;
};

class BlockRegistry
{
public:
    static BlockInfo blockInfos[256]; // Support up to 256 block types

    static void initialize()
    {
        blockInfos[AIR] = {false, {0, 0, 0, 0, 0, 0}, "Air"};
        blockInfos[GRASS] = {true, {0, 0, 0, 0, 0, 0}, "Grass"};
        blockInfos[DIRT] = {true, {0, 0, 0, 0, 0, 0}, "Dirt"};
        blockInfos[STONE] = {true, {0, 0, 0, 0, 0, 0}, "Stone"};
    }

    // Set same texture for all faces
    static void setBlockTexture(BlockType type, unsigned int textureID)
    {
        for (int i = 0; i < 6; i++)
        {
            blockInfos[type].textures[i] = textureID;
        }
    }

    // Set different textures for each face
    static void setBlockTextures(BlockType type,
                                 unsigned int front,
                                 unsigned int back,
                                 unsigned int left,
                                 unsigned int right,
                                 unsigned int bottom,
                                 unsigned int top)
    {
        blockInfos[type].textures[FRONT] = front;
        blockInfos[type].textures[BACK] = back;
        blockInfos[type].textures[LEFT] = left;
        blockInfos[type].textures[RIGHT] = right;
        blockInfos[type].textures[BOTTOM] = bottom;
        blockInfos[type].textures[TOP] = top;
    }

    // Convenience method: set side texture for all 4 sides, different top/bottom
    static void setBlockTextures(BlockType type,
                                 unsigned int side,
                                 unsigned int top,
                                 unsigned int bottom)
    {
        blockInfos[type].textures[FRONT] = side;
        blockInfos[type].textures[BACK] = side;
        blockInfos[type].textures[LEFT] = side;
        blockInfos[type].textures[RIGHT] = side;
        blockInfos[type].textures[BOTTOM] = bottom;
        blockInfos[type].textures[TOP] = top;
    }

    static bool isSolid(BlockType type)
    {
        return blockInfos[type].isSolid;
    }

    static unsigned int getTexture(BlockType type, int face)
    {
        return blockInfos[type].textures[face];
    }
};

inline BlockInfo BlockRegistry::blockInfos[256] = {};

class Chunk
{
public:
    BlockType voxelMap[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];

    Chunk() {
        populateVoxelMap();
    }

    void populateVoxelMap()
    {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                for (int z = 0; z < CHUNK_WIDTH; z++) {
                    if (y == 0) {
                        voxelMap[x][y][z] = STONE;
                    } else if (y < CHUNK_HEIGHT-1) {
                        voxelMap[x][y][z] = DIRT;
                    } else if (y == CHUNK_HEIGHT-1) {
                        voxelMap[x][y][z] = GRASS;
                    } else {
                        voxelMap[x][y][z] = AIR;
                    }
                }
            }
        }
    }

    BlockType getBlock(int x, int y, int z)
    {
        if (x < 0 || x >= CHUNK_WIDTH ||
            y < 0 || y >= CHUNK_HEIGHT ||
            z < 0 || z >= CHUNK_WIDTH)
        {
            return AIR;
        }
        return voxelMap[x][y][z];
    }

    void setBlock(int x, int y, int z, BlockType type)
    {
        if (x >= 0 && x < CHUNK_WIDTH &&
            y >= 0 && y < CHUNK_HEIGHT &&
            z >= 0 && z < CHUNK_WIDTH)
        {
            voxelMap[x][y][z] = type;
        }
    }

    void generateMesh(class World &world, Shader &shader, unsigned int VAO, int cx, int cz);
    
};

class World
{
public:
    Chunk chunks[WORLD_WIDTH][WORLD_WIDTH];

    World() {
    }

    BlockType getBlock(int chunkX, int chunkZ, int localX, int localY, int localZ)
    {
        int targetChunkX = chunkX;
        int targetChunkZ = chunkZ;
        int targetLocalX = localX;
        int targetLocalZ = localZ;

        if (localX < 0)
        {
            targetChunkX--;
            targetLocalX += CHUNK_WIDTH;
        }
        else if (localX >= CHUNK_WIDTH)
        {
            targetChunkX++;
            targetLocalX -= CHUNK_WIDTH;
        }

        if (localZ < 0)
        {
            targetChunkZ--;
            targetLocalZ += CHUNK_WIDTH;
        }
        else if (localZ >= CHUNK_WIDTH)
        {
            targetChunkZ++;
            targetLocalZ -= CHUNK_WIDTH;
        }

        if (targetChunkX < 0 || targetChunkX >= WORLD_WIDTH ||
            targetChunkZ < 0 || targetChunkZ >= WORLD_WIDTH ||
            localY < 0 || localY >= CHUNK_HEIGHT)
        {
            return AIR;
        }

        return chunks[targetChunkX][targetChunkZ].getBlock(targetLocalX, localY, targetLocalZ);
    }

    bool isBlockSolid(int chunkX, int chunkZ, int localX, int localY, int localZ)
    {
        BlockType block = getBlock(chunkX, chunkZ, localX, localY, localZ);
        return BlockRegistry::isSolid(block);
    }

    bool isBlockSolid(float worldX, float worldY, float worldZ) {
        int chunkX = worldX / CHUNK_WIDTH;
        int chunkZ = worldZ / CHUNK_WIDTH;
        int localX = (int)floor(worldX) % CHUNK_WIDTH;
        int localZ = (int)floor(worldZ) % CHUNK_WIDTH;

        if ((int)floor(worldX) < 0) {
            localX += CHUNK_WIDTH;
            chunkX--;
        }
        if ((int)floor(worldZ) < 0) {
            localZ += CHUNK_WIDTH;
            chunkZ--;
        }

        if (chunkX < 0 || chunkX >= WORLD_WIDTH ||
            chunkZ < 0 || chunkZ >= WORLD_WIDTH ||
            worldY < 0 || worldY >= CHUNK_HEIGHT) {
            return false;
        }

        std::cout << "Checking block at Chunk(" << chunkX << ", " << chunkZ << ") Local(" << localX << ", " << worldY << ", " << localZ << ")\n";

        return isBlockSolid(chunkX, chunkZ, localX, worldY, localZ);
    }

    bool isPlayerColliding(float playerX, float playerY, float playerZ, float playerWidth, float playerHeight) {
        float halfWidth = playerWidth / 2.0f;

        int minX = floor(playerX - halfWidth);
        int maxX = ceil(playerX + halfWidth);
        int minY = floor(playerY);
        int maxY = ceil(playerY + playerHeight);
        int minZ = floor(playerZ - halfWidth);
        int maxZ = ceil(playerZ + halfWidth);

        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                for (int z = minZ; z <= maxZ; z++) {
                    if (isBlockSolid(x, y, z))
                        return true;
                }
            }
        }

        return false;
    }
};

inline void Chunk::generateMesh(World &world, Shader &shader, unsigned int VAO, int cx, int cz)
{
    for (unsigned int x = 0; x < CHUNK_WIDTH; x++)  {
        for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
            for (unsigned int z = 0; z < CHUNK_WIDTH; z++) {
                BlockType blockType = getBlock(x, y, z);

                if (blockType == AIR)
                    continue;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(cx * CHUNK_WIDTH + x, y, cz * CHUNK_WIDTH + z));
                shader.setMat4("model", model);

                glActiveTexture(GL_TEXTURE0);
                glBindVertexArray(VAO);
                for (unsigned int p = 0; p < 6; p++) {
                    int nx = (int)x + faceChecks[p].x;
                    int ny = (int)y + faceChecks[p].y;
                    int nz = (int)z + faceChecks[p].z;

                    glBindTexture(GL_TEXTURE_2D, BlockRegistry::getTexture(blockType, p));
                    if (!world.isBlockSolid(cx, cz, nx, ny, nz))
                        glDrawArrays(GL_TRIANGLES, p * 6, 6);
                }
            }
        }
    }
}
