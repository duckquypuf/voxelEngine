#include "voxelData.h"
#include "chunk.h"
#include "world.h"

void Chunk::populateVoxelMap()
{
    voxelMap.resize(chunkWidth);
    for (int x = 0; x < chunkWidth; x++)
    {
        voxelMap[x].resize(chunkHeight);
        for (int y = 0; y < chunkHeight; y++)
        {
            voxelMap[x][y].resize(chunkWidth);
        }
    }

    for (int x = 0; x < chunkWidth; x++)
    {
        for (int y = 0; y < chunkHeight; y++)
        {
            for (int z = 0; z < chunkWidth; z++)
            {
                voxelMap[x][y][z] = blockTypes[world->genVoxel(coord, x, y, z)];
            }
        }
    }
}

void Chunk::generateMesh()
{
    vertices.clear();
    vertices.reserve(chunkWidth * chunkHeight * chunkWidth * 6 * 48); // Estimate max size
    for (int y = 0; y < chunkHeight; y++)
    {
        bool layerHasBlocks = false;
        for (int x = 0; x < chunkWidth && !layerHasBlocks; x++)
            for (int z = 0; z < chunkWidth && !layerHasBlocks; z++)
                if (!voxelMap[x][y][z].isAir) layerHasBlocks = true;

        if (!layerHasBlocks) continue;

        for (int x = 0; x < chunkWidth; x++)
        {
            for (int z = 0; z < chunkWidth; z++)
            {
                BlockType &block = voxelMap[x][y][z];
                if (block.isAir)
                    continue;

                for (int p = 0; p < 6; p++)
                {
                    int nx = x + faceChecks[p][0];
                    int ny = y + faceChecks[p][1];
                    int nz = z + faceChecks[p][2];

                    if (!world->isVoxelSolid(coord, nx, ny, nz) || world->isVoxelTransparent(coord, nx, ny, nz))
                    {
                        // Draw face
                        int offset = p * 48; // 6 verts * 8 floats
                        const int stride = 8;
                        for (int i = 0; i < 48; i += stride)
                        {
                            // position 3 floats add the block local position
                            vertices.push_back(cubeVertices[offset + i + 0] + (float)x);
                            vertices.push_back(cubeVertices[offset + i + 1] + (float)y);
                            vertices.push_back(cubeVertices[offset + i + 2] + (float)z);

                            // normal 3 floats unchanged
                            vertices.push_back(cubeVertices[offset + i + 3]);
                            vertices.push_back(cubeVertices[offset + i + 4]);
                            vertices.push_back(cubeVertices[offset + i + 5]);

                            // texcoords 2 floats unchanged
                            vertices.push_back(cubeVertices[offset + i + 6]);
                            vertices.push_back(cubeVertices[offset + i + 7]);

                            int tid = block.textures[p];
                            vertices.push_back(*(float *)&tid);
                        }
                    }
                    else
                    {
                        // Skip face
                        continue;
                    }
                }
            }
        }
    }

    vertexCount = vertices.size() / 9;

    if (VAO == 0)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(3, 1, GL_INT, sizeof(float) * 9, (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    shouldRegen = false;
}

void Chunk::setVoxel(int localX, int localY, int localZ, unsigned int block)
{
    voxelMap[localX][localY][localZ] = blockTypes[block];
    world->regenerateChunks(coord, localX, localZ);
}
