#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct 
{
    Camera3D camera;
    Vector3 position;
    Vector3 velocity;
    float speed;
    bool isGrounded;
    float yaw;
    float pitch;
} Player;

void InitPlayer(Player* player);
void UpdatePlayer(Player* player, Model* ground);

#endif
