#include "voxelData.h"

int worldWidth = 100;
int chunkWidth = 16;
int chunkHeight = 128;

float biomeScale = 0.02f;
int terrainMinHeight = 32;
int terrainHeight = 52;

float caveGenLargeScale = 0.02f;
float caveGenMediumScale = 0.05f;
float caveGenSmallScale = 0.1f;
float caveGenThreshold = 0.6f;

Lode lodes[] = 
{
    Lode("Dirt", 2, 0.1f, 0.7f, 1000.0f, 20, 50),
};
int lodeCount = 1;

float treeZoneScale = 0.05f;
float treeZoneThreshold = 0.55f;
int treeZoneOffset = 3000;

float treePlacementScale = 0.7f;
float treePlacementThreshold = 0.85f;
int treePlacementOffset = 1000;
int treeMinHeight = 4;
int treeMaxHeight = 7;

bool useRD = true;
int renderDistance = 5;

float gravity = 10.0f;

float cubeVertices[] = {
    // Position (3D)       | Normal (3D)         | TexCoords (2D)
    // ---- Front Face ----
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom-left
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // Bottom-right
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,   // Top-right
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,   // Top-right (Repeated)
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // Top-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom-left (Repeated)

    // ---- Back Face ----
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // Bottom-left
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // Top-left
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,   // Top-right
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,   // Top-right (Repeated)
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // Bottom-right
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // Bottom-left (Repeated)

    // ---- Right Face ----
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom-left
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // Bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Top-right
    0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Top-right (Repeated)
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Top-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom-left (Repeated)

    // ---- Left Face ----
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // Bottom-right
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // Top-right
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Top-left
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Top-left (Repeated)
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom-left
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // Bottom-right (Repeated)

    // ---- Top Face ----
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // Top-left
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // Top-right
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // Top-right (Repeated)
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // Bottom-right
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left (Repeated)

    // ---- Bottom Face ----
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Top-left
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // Top-right
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,   // Bottom-right
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,   // Bottom-right (Repeated)
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,  // Bottom-left
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f  // Top-left (Repeated)
};

int faceChecks[6][3] = {
    // Front Back Right Left Top Bottom
    {0, 0, 1},
    {0, 0, -1},
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0}
};

BlockType blockTypes[] =
{
    BlockType({0, 0, 0, 0, 0, 0}, false, "Air", true, true),
    BlockType({1, 1, 1, 1, 0, 2}, true, "Grass Block"),
    BlockType({2, 2, 2, 2, 2, 2}, true, "Dirt Block"),
    BlockType({3, 3, 3, 3, 3, 3}, true, "Stone Block"),
    BlockType({4, 4, 4, 4, 5, 5}, true, "Oak Log"),
    BlockType({6, 6, 6, 6, 6, 6}, true, "Oak Leaves", true),
};
