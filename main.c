#include "raylib.h"
#include "rlgl.h"
#include "player.h"
#include "world.h"
#include "shaders.h"
#include "raymath.h"

#define SCREEN_WIDTH 1680
#define SCREEN_HEIGHT 960
#define FPS 144

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My First Game");
    SetTargetFPS(FPS);

    DisableCursor();

    Player player;
    InitPlayer(&player);
    World world;
    InitWorld(&world);
    LightingShader shader;
    InitLightingShader(&shader);
    for (int i = 0; i < world.model.materialCount; i++) {
        world.model.materials[i].shader = shader.shader;
    }

    Mesh cube_mesh = GenMeshCube(100.0f, 2.0f, 100.0f);
    Model cube_model = LoadModelFromMesh(cube_mesh);
    Vector3 cube_position = {0.0f, -1.0f, 0.0f};
    
    cube_model.transform = MatrixTranslate(cube_position.x, cube_position.y, cube_position.z);

    // Camera3D camera = { 0 };
    //camera.position = (Vector3){ 0.0f, 2.0f, 5.0f };
    //camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    //camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    //camera.fovy = 45.0f;
    //camera.projection = CAMERA_PERSPECTIVE;


    Vector3 lightPos = (Vector3){ 10.0, 10.0, 0.0f };
    
    rlDisableBackfaceCulling();
    while (!WindowShouldClose()) {
        UpdatePlayer(&player, &cube_model);


        ApplyLightingShader(&shader, &cube_model, lightPos, player.camera.position);    
        UpdateCamera(&player.camera, CAMERA_FIRST_PERSON);
        BeginDrawing();
        ClearBackground(SKYBLUE);
        BeginMode3D(player.camera);
        DrawWorld(&world);
        DrawModel(cube_model, (Vector3){0}, 1.0f, WHITE);
        DrawGrid(10, 1.0f);
      
        EndMode3D();
        DrawFPS(10, 10);
        DrawText(TextFormat("Grounded: %d", player.isGrounded), 10, 30, 20, GRAY);
        EndDrawing();
    }

    UnloadWorld(&world);
    UnloadLightingShader(&shader);
    CloseWindow();
    return 0;
}