#pragma once

#include "blockTypes.h"
#include "lodes.h"

extern int worldWidth;
extern int chunkWidth;
extern int chunkHeight;

extern float biomeScale;
extern int terrainMinHeight;
extern int terrainHeight;

extern float caveGenLargeScale;
extern float caveGenMediumScale;
extern float caveGenSmallScale;
extern float caveGenThreshold;

extern Lode lodes[];
extern int lodeCount;

extern float treeZoneScale;
extern float treeZoneThreshold;
extern int treeZoneOffset;

extern float treePlacementScale;
extern float treePlacementThreshold;
extern int treePlacementOffset;
extern int treeMinHeight;
extern int treeMaxHeight;

extern bool useRD;
extern int renderDistance;

extern float gravity;

extern float cubeVertices[48*6];
extern int faceChecks[6][3];
extern BlockType blockTypes[];
