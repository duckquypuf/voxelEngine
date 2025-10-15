#pragma once
#include <glm/glm.hpp>
#include <string>
#include <math.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "FastNoiseLite.h"

constexpr unsigned int CHUNK_WIDTH = 16;
constexpr unsigned int CHUNK_HEIGHT = 64;
constexpr unsigned int TERRAIN_MIN_HEIGHT = 48;
constexpr unsigned int TERRAIN_MAX_HEIGHT = 64;
constexpr unsigned int WORLD_WIDTH = 100;
constexpr unsigned int RENDER_DISTANCE = 20;

static FastNoiseLite noise;

const glm::ivec3 faceChecks[] = {
    glm::ivec3(0, 0, -1), // Front - 0
    glm::ivec3(0, 0, 1),  // Back - 1
    glm::ivec3(-1, 0, 0), // Left - 2
    glm::ivec3(1, 0, 0),  // Right - 3
    glm::ivec3(0, -1, 0), // Bottom - 4
    glm::ivec3(0, 1, 0)   // Top - 5
};

struct FaceAxes
{
    int uAxis; // First perpendicular axis (0=x, 1=y, 2=z)
    int vAxis; // Second perpendicular axis
};

const FaceAxes faceAxes[] = {
    {0, 1}, // FRONT (faces Z) - perpendicular: X and Y
    {0, 1}, // BACK (faces Z) - perpendicular: X and Y
    {2, 1}, // LEFT (faces X) - perpendicular: Z and Y
    {2, 1}, // RIGHT (faces X) - perpendicular: Z and Y
    {0, 2}, // BOTTOM (faces Y) - perpendicular: X and Z
    {0, 2}  // TOP (faces Y) - perpendicular: X and Z
};

struct QuadVertex
{
    glm::vec3 position;
    glm::vec2 texCoord;
    float texID;
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

struct Quad
{
    std::vector<QuadVertex> vertices;
    BlockType blockType;
    FaceDirection face;
};

float getHeight(float x, float z)
{
    float n = noise.GetNoise(x, z);
    n = (n + 1.0f) * 0.5f;
    return TERRAIN_MIN_HEIGHT + n * (TERRAIN_MAX_HEIGHT-TERRAIN_MIN_HEIGHT);
}

class Chunk
{
public:
    BlockType voxelMap[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];

    unsigned int VAO = 0, VBO = 0;
    size_t vertexCount = 0;
    bool needsRebuild = true;
    bool needsRender = false;
    bool isPopulated = false;

    Chunk() {
        needsRebuild = false;
    }

    ~Chunk() {
        if (VAO)
            glDeleteVertexArrays(1, &VAO);
        if (VBO)
            glDeleteBuffers(1, &VBO);
    }

    void populateVoxelMap(int cx, int cz)
    {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                for (int z = 0; z < CHUNK_WIDTH; z++) {
                    float height = (int)getHeight(x + cx * CHUNK_WIDTH, z + cz * CHUNK_WIDTH);

                    if (y == 0) {
                        voxelMap[x][y][z] = STONE;
                    } else if (y < height-1) {
                        voxelMap[x][y][z] = DIRT;
                    } else if (y == height-1) {
                        voxelMap[x][y][z] = GRASS;
                    } else {
                        voxelMap[x][y][z] = AIR;
                    }
                }
            }
        }

        isPopulated = true;
    }

    BlockType getBlock(int x, int y, int z, int chunkX = 0, int chunkZ = 0)
    {
        if(!isPopulated)
            populateVoxelMap(chunkX, chunkZ);
        if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_WIDTH) {
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

    void generateMesh(class World &world, int cx, int cz);
    void renderMesh(Shader &shader, int cx, int cz);
    std::vector<QuadVertex> generateQuadVertices(int x, int y, int z, FaceDirection face, unsigned int width, unsigned int height, BlockType blockType);
};

class World
{
public:
    Chunk chunks[WORLD_WIDTH][WORLD_WIDTH];

    World() {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFrequency(0.005f);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(1);
        noise.SetFractalLacunarity(2.0f);
        noise.SetFractalGain(0.5f);

        noise.SetSeed(1234);
    }

    BlockType getBlock(int chunkX, int chunkZ, int localX, int localY, int localZ)
    {
        int targetChunkX = chunkX;
        int targetChunkZ = chunkZ;
        int targetLocalX = localX;
        int targetLocalZ = localZ;

        if (localX < 0) {
            targetChunkX--;
            targetLocalX += CHUNK_WIDTH;
        } else if (localX >= CHUNK_WIDTH) {
            targetChunkX++;
            targetLocalX -= CHUNK_WIDTH;
        }

        if (localZ < 0) {
            targetChunkZ--;
            targetLocalZ += CHUNK_WIDTH;
        } else if (localZ >= CHUNK_WIDTH) {
            targetChunkZ++;
            targetLocalZ -= CHUNK_WIDTH;
        }

        if (targetChunkX < 0 || targetChunkX >= WORLD_WIDTH ||
            targetChunkZ < 0 || targetChunkZ >= WORLD_WIDTH ||
            localY < 0 || localY >= CHUNK_HEIGHT)
        {
            return AIR;
        }

        return chunks[targetChunkX][targetChunkZ].getBlock(targetLocalX, localY, targetLocalZ, targetChunkX, targetChunkZ);
    }

    BlockType getBlock(float worldX, float worldY, float worldZ) {
        int chunkX = worldX / CHUNK_WIDTH;
        int chunkZ = worldZ / CHUNK_WIDTH;
        int localX = (int)floor(worldX) % CHUNK_WIDTH;
        int localZ = (int)floor(worldZ) % CHUNK_WIDTH;

        if ((int)floor(worldX) < 0)
        {
            localX += CHUNK_WIDTH;
            chunkX--;
        }
        if ((int)floor(worldZ) < 0)
        {
            localZ += CHUNK_WIDTH;
            chunkZ--;
        }

        if (chunkX < 0 || chunkX >= WORLD_WIDTH ||
            chunkZ < 0 || chunkZ >= WORLD_WIDTH ||
            worldY < 0 || worldY >= CHUNK_HEIGHT)
        {
            return AIR;
        }

        return chunks[chunkX][chunkZ].getBlock(localX, worldY, localZ);
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

        return isBlockSolid(chunkX, chunkZ, localX, worldY, localZ);
    }

    bool isPlayerColliding(float playerX, float playerY, float playerZ, float playerWidth, float playerHeight) {
        float halfWidth = playerWidth / 2.0f;

        int minX = floor(playerX - halfWidth);
        int maxX = floor(playerX + halfWidth);
        int minY = floor(playerY);
        int maxY = floor(playerY + playerHeight);
        int minZ = floor(playerZ - halfWidth);
        int maxZ = floor(playerZ + halfWidth);

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

inline void Chunk::generateMesh(World &world, int cx, int cz)
{
    bool meshed[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH][6] = {false};
    std::vector<QuadVertex> allVertices;
    std::vector<Quad> quads;

    for (unsigned int x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
            for (unsigned int z = 0; z < CHUNK_WIDTH; z++) {
                BlockType blockType = getBlock(x, y, z);

                if (blockType == AIR) {
                    continue;
                }

                for(unsigned int p = 0; p < 6; p++) {
                    int nx = (int)x + faceChecks[p].x;
                    int ny = (int)y + faceChecks[p].y;
                    int nz = (int)z + faceChecks[p].z;

                    if(world.isBlockSolid(cx, cz, nx, ny, nz))
                        continue;

                    if(meshed[x][y][z][p])
                        continue;

                    int uAxis = faceAxes[p].uAxis;
                    int vAxis = faceAxes[p].vAxis;

                    glm::ivec3 pos = glm::ivec3(x, y, z);
                    unsigned int width = 1;

                    while (true) {
                        glm::ivec3 nextPos = pos;
                        nextPos[uAxis] += width;

                        if (nextPos.x < 0 || nextPos.x >= CHUNK_WIDTH ||
                            nextPos.y < 0 || nextPos.y >= CHUNK_HEIGHT ||
                            nextPos.z < 0 || nextPos.z >= CHUNK_WIDTH)
                            break;

                        if (meshed[nextPos.x][nextPos.y][nextPos.z][p])
                            break;

                        if (world.getBlock(cx, cz, nextPos.x, nextPos.y, nextPos.z) != blockType)
                            break;

                        int nextNX = nextPos.x + faceChecks[p].x;
                        int nextNY = nextPos.y + faceChecks[p].y;
                        int nextNZ = nextPos.z + faceChecks[p].z;
                        if (world.isBlockSolid(cx, cz, nextNX, nextNY, nextNZ))
                            break;

                        width++;
                    }

                    unsigned int height = 1;
                    bool canExtend = true;

                    while (canExtend) {
                        for (unsigned int u = 0; u < width; u++) {
                            glm::ivec3 checkPos = pos;
                            checkPos[uAxis] += u;
                            checkPos[vAxis] += height;

                            // Bounds check
                            if (checkPos.x < 0 || checkPos.x >= CHUNK_WIDTH ||
                                checkPos.y < 0 || checkPos.y >= CHUNK_HEIGHT ||
                                checkPos.z < 0 || checkPos.z >= CHUNK_WIDTH)
                            {
                                canExtend = false;
                                break;
                            }

                            // Already meshed check
                            if (meshed[checkPos.x][checkPos.y][checkPos.z][p])
                            {
                                canExtend = false;
                                break;
                            }

                            // Block type check
                            if (world.getBlock(cx, cz, checkPos.x, checkPos.y, checkPos.z) != blockType)
                            {
                                canExtend = false;
                                break;
                            }

                            // Neighbor solid check (is face exposed?)
                            int checkNX = checkPos.x + faceChecks[p].x;
                            int checkNY = checkPos.y + faceChecks[p].y;
                            int checkNZ = checkPos.z + faceChecks[p].z;
                            if (world.isBlockSolid(cx, cz, checkNX, checkNY, checkNZ))
                            {
                                canExtend = false;
                                break;
                            }
                        }

                        if (canExtend)
                            height++;
                    }
                    
                    for (unsigned int h = 0; h < height; h++) {
                        for (unsigned int w = 0; w < width; w++) {
                            glm::ivec3 meshPos = pos;
                            meshPos[uAxis] += w;
                            meshPos[vAxis] += h;
                            meshed[meshPos.x][meshPos.y][meshPos.z][p] = true;
                        }
                    }

                    std::vector<QuadVertex> quadVerts = generateQuadVertices((int)x, (int)y, (int)z, (FaceDirection)p, width, height, blockType);

                    allVertices.insert(allVertices.end(), quadVerts.begin(), quadVerts.end());
                }   
            }
        }
    }

    if (this->VAO)
        glDeleteVertexArrays(1, &this->VAO);
    if (this->VBO)
        glDeleteBuffers(1, &this->VBO);

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(QuadVertex), allVertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)0);
    glEnableVertexAttribArray(0);

    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, texCoord));
    glEnableVertexAttribArray(1);

    // TexID attribute
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, texID));
    glEnableVertexAttribArray(2);

    vertexCount = allVertices.size();
    needsRebuild = false;
}

inline void Chunk::renderMesh(Shader& shader, int cx, int cz) {
    if (vertexCount == 0)
        return;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(cx * CHUNK_WIDTH, 0, cz * CHUNK_WIDTH));
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

inline std::vector<QuadVertex> Chunk::generateQuadVertices(int x, int y, int z, FaceDirection face, unsigned int width, unsigned int height, BlockType blockType) {
    std::vector<QuadVertex> vertices;
    glm::vec3 basePos(x, y, z);
    glm::vec3 corners[4];

    switch (face)
    {
    case FRONT: // -Z face
        corners[0] = basePos + glm::vec3(0, 0, 0);
        corners[1] = basePos + glm::vec3(width, 0, 0);
        corners[2] = basePos + glm::vec3(width, height, 0);
        corners[3] = basePos + glm::vec3(0, height, 0);
        break;

    case BACK: // +Z face
        corners[0] = basePos + glm::vec3(width, 0, 1);
        corners[1] = basePos + glm::vec3(0, 0, 1);
        corners[2] = basePos + glm::vec3(0, height, 1);
        corners[3] = basePos + glm::vec3(width, height, 1);
        break;

    case LEFT: // -X face
        corners[0] = basePos + glm::vec3(0, 0, width);
        corners[1] = basePos + glm::vec3(0, 0, 0);
        corners[2] = basePos + glm::vec3(0, height, 0);
        corners[3] = basePos + glm::vec3(0, height, width);
        break;

    case RIGHT: // +X face
        corners[0] = basePos + glm::vec3(1, 0, 0);
        corners[1] = basePos + glm::vec3(1, 0, width);
        corners[2] = basePos + glm::vec3(1, height, width);
        corners[3] = basePos + glm::vec3(1, height, 0);
        break;

    case BOTTOM: // -Y face
        corners[0] = basePos + glm::vec3(0, 0, 0);
        corners[1] = basePos + glm::vec3(0, 0, height);
        corners[2] = basePos + glm::vec3(width, 0, height);
        corners[3] = basePos + glm::vec3(width, 0, 0);
        break;

    case TOP: // +Y face
        corners[0] = basePos + glm::vec3(0, 1, 0);
        corners[1] = basePos + glm::vec3(width, 1, 0);
        corners[2] = basePos + glm::vec3(width, 1, height);
        corners[3] = basePos + glm::vec3(0, 1, height);
        break;
    }

    glm::vec2 texCoords[4];
    if (face == BOTTOM)
    {
        texCoords[0] = glm::vec2(0, 0);
        texCoords[1] = glm::vec2(0, height);
        texCoords[2] = glm::vec2(width, height);
        texCoords[3] = glm::vec2(width, 0);
    }
    else
    {
        texCoords[0] = glm::vec2(0, 0);
        texCoords[1] = glm::vec2(width, 0);
        texCoords[2] = glm::vec2(width, height);
        texCoords[3] = glm::vec2(0, height);
    }

    unsigned int texID = BlockRegistry::getTexture(blockType, face);

    vertices.push_back({corners[0], texCoords[0], (float)texID});
    vertices.push_back({corners[1], texCoords[1], (float)texID});
    vertices.push_back({corners[2], texCoords[2], (float)texID});
    vertices.push_back({corners[2], texCoords[2], (float)texID});
    vertices.push_back({corners[3], texCoords[3], (float)texID});
    vertices.push_back({corners[0], texCoords[0], (float)texID});

    return vertices;
}
