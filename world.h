#ifndef WORLD_H
#define WORLD_H

#include "raylib.h"

typedef struct {
    Model model;
    Vector3 position;
} World;

void InitWorld(World* world);
void DrawWorld(World* world);
void UnloadWorld(World* world);

#endif