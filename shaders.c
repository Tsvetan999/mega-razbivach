#include "shaders.h"

// Initialize the lighting shader with Raylib's example shaders
void InitLightingShader(LightingShader* shader) {
    // Load the GLSL 330 lighting shaders from Raylib's examples
    shader->shader = LoadShader("resources/shaders/glsl330/lighting.vs", 
                               "resources/shaders/glsl330/lighting.fs");

    // Get uniform locations
    shader->lightPosLoc = GetShaderLocation(shader->shader, "lights[0].position"); // First light's position
    shader->lightColorLoc = GetShaderLocation(shader->shader, "lights[0].color");  // First light's color
    shader->ambientLoc = GetShaderLocation(shader->shader, "ambient");            // Ambient light
    shader->viewPosLoc = GetShaderLocation(shader->shader, "viewPos");            // Camera position

    // Set shader location for view position (required by the shader)
    shader->shader.locs[SHADER_LOC_VECTOR_VIEW] = shader->viewPosLoc;
}

// Apply the lighting shader to a model and set uniforms
void ApplyLightingShader(LightingShader* shader, Model* model, Vector3 lightPos, Vector3 cameraPos) {
    // Assign the shader to the first material (extend to all materials if needed)
    model->materials[0].shader = shader->shader;

    // Light properties
    Vector3 lightColor = (Vector3){ 1.0f, 1.0f, 1.0f }; // White light
    float lightColorNorm[4] = { lightColor.x, lightColor.y, lightColor.z, 1.0f }; // Normalized for shader
    float ambient[4] = { 0.3f, 0.3f, 0.3f, 1.0f }; // Ambient light (RGBA)

    // Set shader values
    SetShaderValue(shader->shader, shader->lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader->shader, shader->lightColorLoc, lightColorNorm, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader->shader, shader->ambientLoc, ambient, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader->shader, shader->viewPosLoc, &cameraPos, SHADER_UNIFORM_VEC3);

    // Enable the first light (required by the shader)
    int lightEnabled = 1;
    int lightType = 1; // Point light (0 = directional, 1 = point)
    int enabledLoc = GetShaderLocation(shader->shader, "lights[0].enabled");
    int typeLoc = GetShaderLocation(shader->shader, "lights[0].type");
    SetShaderValue(shader->shader, enabledLoc, &lightEnabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader->shader, typeLoc, &lightType, SHADER_UNIFORM_INT);
}

// Unload the lighting shader
void UnloadLightingShader(LightingShader* shader) {
    UnloadShader(shader->shader);
}