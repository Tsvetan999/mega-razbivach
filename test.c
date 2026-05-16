#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define NUM_CUBES 1000

typedef struct Cube {
    Vector3 position;
    Color color;
    Vector3 rotation;
    Vector3 scale;  // Added for different sizes
} Cube;

int main(void) {
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Raylib Cube Instancing with Different Sizes");
    SetTargetFPS(0);

    // Initialize camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 20.0f, 10.0f, 20.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialize cubes
    Cube cubes[NUM_CUBES];
    srand(time(NULL));
    
    for (int i = 0; i < NUM_CUBES; i++) {
        cubes[i].position = (Vector3){
            (float)(rand() % 40 - 20),  // Random x (-20 to 20)
            (float)(rand() % 20),       // Random y (0 to 20)
            (float)(rand() % 40 - 20)   // Random z (-20 to 20)
        };
        cubes[i].color = (Color){
            rand() % 256,  // Random red
            rand() % 256,  // Random green
            rand() % 256,  // Random blue
            255            // Full alpha
        };
        cubes[i].rotation = (Vector3){
            (float)(rand() % 360),  // Random rotation x
            (float)(rand() % 360),  // Random rotation y
            (float)(rand() % 360)   // Random rotation z
        };
        // Random scale between 0.5 and 2.0 for each dimension
        cubes[i].scale = (Vector3){
            0.5f + (float)(rand() % 150) / 100.0f,  // Random scale x (0.5 to 2.0)
            0.5f + (float)(rand() % 150) / 100.0f,  // Random scale y (0.5 to 2.0)
            0.5f + (float)(rand() % 150) / 100.0f   // Random scale z (0.5 to 2.0)
        };
    }

    while (!WindowShouldClose()) {
        // Update
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            BeginMode3D(camera);
                // Draw all cubes with different sizes
                for (int i = 0; i < NUM_CUBES; i++) {
                    DrawCubeV(cubes[i].position, cubes[i].scale, cubes[i].color);
                    DrawCubeWiresV(cubes[i].position, cubes[i].scale, BLACK);
                }
                
                // Draw grid for reference
                DrawGrid(50, 1.0f);
            EndMode3D();

            // Draw FPS
            DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}