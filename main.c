#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "stdlib.h"

#define SCREEN_WIDTH 1680
#define SCREEN_HEIGHT 960
#define FPS 144

struct cube_model {
    Mesh mesh;
    Model model;
    Vector3 position;
};

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My First Game");
    SetTargetFPS(FPS);

    DisableCursor();

    int n = 5; 

    struct cube_model *cube_array;
    cube_array = (struct cube_model*) malloc(n * sizeof(struct cube_model));

    cube_array[0].mesh = GenMeshCube(50.0f, 2.0f, 50.0f);
    cube_array[0].model = LoadModelFromMesh(cube_array[0].mesh);
    cube_array[0].position.x = 0.0f;
    cube_array[0].position.y = -1.0f;
    cube_array[0].position.z = 0.0f;
    cube_array[0].model.transform = MatrixTranslate(cube_array[0].position.x, cube_array[0].position.y, cube_array[0].position.z);

    for (size_t i = 1; i < n; i++) {
        cube_array[i].mesh = GenMeshCube(3.0f, 10.0f, 5.0f);
        cube_array[i].model = LoadModelFromMesh(cube_array[i].mesh);
        cube_array[i].position = (Vector3){4.0f * i, 0.0f, 2.0f * i}; 
        cube_array[i].model.transform = MatrixTranslate(cube_array[i].position.x, cube_array[i].position.y, cube_array[i].position.z);
    }
    
    
    WHITE;
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 5.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    rlDisableBackfaceCulling();
    
    while (!WindowShouldClose()) {


        //ApplyLightingShader(&shader, &cube_model1, &cube_model2, lightPos, player.camera.position);    
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        BeginDrawing();
        ClearBackground(SKYBLUE);
        BeginMode3D(camera);
        for (size_t i = 0; i < n; i++)
        {
            DrawModel(cube_array[i].model, (Vector3){0}, 1.0f, (Color){13 * i, 13 * i, 13 * i, 255});
        }
        
        DrawGrid(10, 1.0f);
      
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}