#pragma once

#include "blockTypes.h"
#include "lodes.h"

extern int worldWidth;
extern int chunkWidth;
extern int chunkHeight;

extern float biomeScale;
extern int terrainMinHeight;
extern int terrainHeight;

static int waterHeight = 64;
static int sandHeight = 60;

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

static float gravity = 10.0f;
static float waterGravity = 3.5f;
static float waterDrag = 0.5f;
static float waterFloat = 4.0f;
static float waterSinkSpeed = 1.0f;

extern float cubeVertices[48*6];
extern int faceChecks[6][3];
extern BlockType blockTypes[];
