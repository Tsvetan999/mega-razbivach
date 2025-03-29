#include "player.h"
#include "raymath.h"

#define GRAVITY 9.8f
#define JUMP_FORCE 5.0f
#define PLAYER_HEIGHT 1.0f
#define MOVE_SPEED 5.0f
#define MOUSE_SENSITIVITY 0.1f

void InitPlayer(Player* player) {
    player->position = (Vector3){ 0.0f, 5.0f, 0.0f };
    player->camera.position = player->position;
    player->camera.target = (Vector3){ 0.0f, 5.0f, -1.0f }; // Initial look direction
    player->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    player->camera.fovy = 60.0f;
    player->camera.projection = CAMERA_PERSPECTIVE;
    player->velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
    player->speed = MOVE_SPEED;
    player->isGrounded = false;
    player->yaw = 0.0f;   // Facing forward
    player->pitch = 0.0f; // Level view
}

void UpdatePlayer(Player* player, Model* ground) {
    // Mouse input for camera rotation
    player->yaw -= GetMouseDelta().x * MOUSE_SENSITIVITY;
    player->pitch -= GetMouseDelta().y * MOUSE_SENSITIVITY;
    player->pitch = Clamp(player->pitch, -89.0f, 89.0f); // Limit vertical look

    // Calculate forward direction (ignore Y for consistent movement)
    Vector3 forward = (Vector3){
        sinf(player->yaw * DEG2RAD),
        0.0f, // Ignore pitch for movement direction
        cosf(player->yaw * DEG2RAD)
    };
    forward = Vector3Normalize(forward);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, player->camera.up));

    // Apply gravity
    player->velocity.y -= GRAVITY * GetFrameTime();

    // Horizontal movement
    Vector3 moveDir = { 0.0f, 0.0f, 0.0f };
    if (IsKeyDown(KEY_W)) moveDir = Vector3Add(moveDir, forward);
    if (IsKeyDown(KEY_S)) moveDir = Vector3Add(moveDir, Vector3Negate(forward));
    if (IsKeyDown(KEY_A)) moveDir = Vector3Add(moveDir, Vector3Negate(right));
    if (IsKeyDown(KEY_D)) moveDir = Vector3Add(moveDir, right);

    if (Vector3Length(moveDir) > 0.0f) {
        moveDir = Vector3Normalize(moveDir);
        moveDir = Vector3Scale(moveDir, player->speed * GetFrameTime());
    }

    // Apply horizontal movement
    player->position.x += moveDir.x;
    player->position.z += moveDir.z;

    // Apply vertical velocity
    Vector3 newPosition = Vector3Add(player->position, Vector3Scale(player->velocity, GetFrameTime()));

    // Ray cast downward
    Ray downRay = { 
        .position = (Vector3){ newPosition.x, newPosition.y + 0.1f, newPosition.z },
        .direction = (Vector3){ 0.0f, -1.0f, 0.0f }
    };
    RayCollision collision = GetRayCollisionMesh(downRay, ground->meshes[0], MatrixIdentity());

    if (collision.hit && player->velocity.y <= 0.0f && collision.distance <= PLAYER_HEIGHT + 0.1f ) {
        player->position.y = collision.point.y;
        player->velocity.y = 0.0f;
        player->isGrounded = true;
    } else { 
        player->position = newPosition;
        player->isGrounded = false;
    }

    // Jump
    if (player->isGrounded && IsKeyPressed(KEY_SPACE)) {
        player->velocity.y = JUMP_FORCE;
        player->isGrounded = false;
    }

    // Update camera position and target
    player->camera.position = player->position;
    Vector3 lookDir = (Vector3){
        sinf(player->yaw * DEG2RAD) * cosf(player->pitch * DEG2RAD),
        sinf(player->pitch * DEG2RAD),
        cosf(player->yaw * DEG2RAD) * cosf(player->pitch * DEG2RAD)
    };
    player->camera.target = Vector3Add(player->camera.position, Vector3Normalize(lookDir));
}