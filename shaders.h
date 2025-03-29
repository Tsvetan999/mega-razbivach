#ifndef SHADERS_H
#define SHADERS_H

#include "raylib.h"

// Define a structure for the lighting shader
typedef struct {
    Shader shader;
    int lightPosLoc;    // Location for light position
    int lightColorLoc;  // Location for light color
    int ambientLoc;     // Location for ambient light
    int viewPosLoc;     // Location for camera position (required by lighting.fs)
} LightingShader;

// Function prototypes
void InitLightingShader(LightingShader* shader);
void ApplyLightingShader(LightingShader* shader, Model* model, Vector3 lightPos, Vector3 cameraPos);
void UnloadLightingShader(LightingShader* shader);

#endif // SHADERS_H