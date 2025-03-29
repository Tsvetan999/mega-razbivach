#include "world.h"
#include <stdio.h>

void InitWorld(World* world) {
    world->model = LoadModel("assets/models/uploads_files_3935360_Demo.glb");
    //Texture2D texture = LoadTexture("assets/textures/grass_texture.png");
    //world->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    world->position = (Vector3){ 0.0f, 0.0f, 0.0f };
}

void DrawWorld(World* world) {
    DrawModel(world->model, world->position, 1.0f, WHITE);
}

void UnloadWorld(World* world) {
    UnloadModel(world->model); // Also unloads textures
}