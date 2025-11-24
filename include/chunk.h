#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "voxelData.h"
#include "shader.h"

struct ChunkCoord
{
    int x, z;

    ChunkCoord(int x=0, int z=0)
    {
        this->x = x;
        this->z = z;
    }

    bool operator==(const ChunkCoord &other) const
    {
        return x == other.x && z == other.z;
    }
};

namespace std
{
    template <>
    struct hash<ChunkCoord>
    {
        size_t operator()(const ChunkCoord &c) const
        {
            // simple hash combo
            return ((size_t)c.x << 32) ^ (size_t)c.z;
        }
    };
}

class World;

class Chunk
{
public:
    ChunkCoord coord;
    std::vector<std::vector<std::vector<BlockType>>> voxelMap;
    bool shouldRegen;
    bool treesGenerated;
    bool cavesGenerated;
    bool lodesGenerated;

    std::vector<float> vertices;
    int vertexCount = 0;

    Chunk() : world(nullptr), coord(ChunkCoord(0, 0)) {}

    Chunk(World* world, ChunkCoord coord, bool gen = false)
    {
        this->world = world;
        this->coord = coord;
        VAO = 0;
        VBO = 0;
        shouldRegen = gen;
        treesGenerated = false;
        cavesGenerated = false;
        lodesGenerated = false;
    }

    void populateVoxelMap();

    void generateMesh();

    void renderChunk(Shader *shader, const glm::mat4 &view, const glm::mat4 &projection)
    {
        if (vertexCount == 0) return;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(coord.x * chunkWidth, 0.0f, coord.z * chunkWidth));

        shader->use();
        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

    void setVoxel(int localX, int localY, int localZ, unsigned int block);

private:
    World* world;
    unsigned int VAO, VBO;
};
