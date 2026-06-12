#include "raylib.h"
#include "raymath.h"

#define GRAVITY         32.0f
#define MAX_SPEED       20.0f
#define CROUCH_SPEED     5.0f
#define JUMP_FORCE      12.0f
#define MAX_ACCEL      150.0f

#define FRICTION         0.86f
#define AIR_DRAG         0.98f

#define CONTROL         15.0f
#define CROUCH_HEIGHT    0.0f
#define STAND_HEIGHT     1.0f
#define BOTTOM_HEIGHT    0.5f

#define MAX_FIREBALLS 1000
#define NORMALIZE_INPUT  0

typedef struct {
    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    bool isGrounded;
} Body;

typedef struct {
    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    float lifetime;
    bool isDead;
} Fireball;

static Vector2 sensitivity = { 0.001f, 0.001f };
static Body player = { 0 };
static Vector2 lookRotation = { 0 };
static float headTimer = 0.0f;
static float walkLerp = 0.0f;
static float headLerp = STAND_HEIGHT;
static Vector2 lean = { 0 };

// Нови променливи за звук и състояние на играта
static Sound soundStep;
static Sound soundJump;
static Sound soundFireball;
static float stepSoundTimer = 0.0f;
static float masterVolume = 0.5f; // По подразбиране на 50%

static bool isPaused = false;
static bool showHelp = false;

void ShootFireball(Camera* camera, Vector3 playerPosition, Fireball* fireballs, int* fireballcount);
static void DrawLevel(void);
static void UpdateCameraFPS(Camera* camera);
static void UpdateBody(Body* body, float rot, char side, char forward, bool jumpPressed, bool crouchHold);

int main(void)
{
    const int screenWidth = 1600;
    const int screenHeight = 960;

    InitWindow(screenWidth, screenHeight, "3d Game");
    SetExitKey(KEY_NULL);

    // Инициализиране на аудио системата
    InitAudioDevice();
    soundStep = LoadSound("step.wav");
    soundJump = LoadSound("jump.wav");
    soundFireball = LoadSound("fireball.wav");
    SetMasterVolume(masterVolume);

    Camera camera = { 0 };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = (Vector3){
        player.position.x,
        player.position.y + (BOTTOM_HEIGHT + headLerp),
        player.position.z,
    };

    Fireball fireballs[MAX_FIREBALLS] = { 0 };
    int fireballCount = 0;

    UpdateCameraFPS(&camera);
    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Превключване на Помощния екран
        if (IsKeyPressed(KEY_H)) showHelp = !showHelp;

        // Превключване на Пауза чрез ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            isPaused = !isPaused;
            if (isPaused) EnableCursor(); // Показваме мишката в менюто
            else DisableCursor();        // Скриваме я обратно в играта
        }

        if (!isPaused) {
            Vector2 mouseDelta = GetMouseDelta();
            lookRotation.x -= mouseDelta.x * sensitivity.x;
            lookRotation.y += mouseDelta.y * sensitivity.y;

            char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
            char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
            bool crouching = IsKeyDown(KEY_LEFT_SHIFT);

            bool jumpPressed = IsKeyPressed(KEY_SPACE);
            // Възпроизвеждане на звук при скок
            if (jumpPressed && player.isGrounded) PlaySound(soundJump);

            UpdateBody(&player, lookRotation.x, sideway, forward, jumpPressed, crouching);

            float delta = GetFrameTime();
            headLerp = Lerp(headLerp, (crouching ? CROUCH_HEIGHT : STAND_HEIGHT), 20.0f * delta);
            camera.position = (Vector3){
                player.position.x,
                player.position.y + (BOTTOM_HEIGHT + headLerp),
                player.position.z,
            };

            if (player.isGrounded && ((forward != 0) || (sideway != 0)))
            {
                headTimer += delta * 3.0f;
                walkLerp = Lerp(walkLerp, 1.0f, 10.0f * delta);
                camera.fovy = Lerp(camera.fovy, 55.0f, 5.0f * delta);

                // Звук от тичане (само ако не сме клекнали)
                if (!crouching) {
                    stepSoundTimer += delta;
                    if (stepSoundTimer >= 0.4f) { // На всеки 0.4 секунди стъпка
                        PlaySound(soundStep);
                        stepSoundTimer = 0.0f;
                    }
                }
            }
            else
            {
                walkLerp = Lerp(walkLerp, 0.0f, 10.0f * delta);
                camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f * delta);
            }

            lean.x = Lerp(lean.x, sideway * 0.02f, 10.0f * delta);
            lean.y = Lerp(lean.y, forward * 0.015f, 10.0f * delta);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                ShootFireball(&camera, player.position, fireballs, &fireballCount);
            }

            for (int i = 0; i < fireballCount; i++) {
                Fireball* fb = &fireballs[i];
                if (fb->isDead) continue;
                fb->position = Vector3Add(fb->position, Vector3Scale(fb->velocity, delta));
                fb->lifetime -= delta;
                if (fb->lifetime <= 0.0f) fb->isDead = true;
            }

            UpdateCameraFPS(&camera);
        }
        else {
            // Логика за кликане в менюто на пауза
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePos = GetMousePosition();

                // Бутон за Fullscreen / Windowed
                if (CheckCollisionPointRec(mousePos, (Rectangle) { screenWidth / 2 - 100, screenHeight / 2 - 60, 200, 40 })) {
                    ToggleFullscreen();
                }
                // Бутон за увеличаване на звука (+)
                if (CheckCollisionPointRec(mousePos, (Rectangle) { screenWidth / 2 - 60, screenHeight / 2, 40, 40 })) {
                    masterVolume = Clamp(masterVolume + 0.1f, 0.0f, 1.0f);
                    SetMasterVolume(masterVolume);
                }
                // Бутон за намаляване на звука (-)
                if (CheckCollisionPointRec(mousePos, (Rectangle) { screenWidth / 2 + 20, screenHeight / 2, 40, 40 })) {
                    masterVolume = Clamp(masterVolume - 0.1f, 0.0f, 1.0f);
                    SetMasterVolume(masterVolume);
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        for (int i = 0; i < fireballCount; i++) {
            if (!fireballs[i].isDead) {
                DrawSphere(fireballs[i].position, 1.0f, RED);
            }
        }
        DrawLevel();
        EndMode3D();

        // Основен интерфейс (Инфо панел)
        DrawRectangle(5, 5, 200, 60, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(5, 5, 200, 60, BLUE);
        DrawText(TextFormat("- Fireballs: (%d)", fireballCount), 15, 10, 10, BLACK);
        DrawText(TextFormat("- Speed: (%05.2f)", Vector2Length((Vector2) { player.velocity.x, player.velocity.z })), 15, 25, 10, BLACK);
        DrawText("Press H for Controls Help", 15, 40, 10, DARKGRAY);

        // Рисуване на Помощния екран (Покриващ панел)
        if (showHelp) {
            DrawRectangle(screenWidth / 2 - 200, screenHeight / 2 - 150, 400, 220, Fade(BLACK, 0.8f));
            DrawRectangleLines(screenWidth / 2 - 200, screenHeight / 2 - 150, 400, 220, WHITE);
            DrawText("GAME CONTROLS", screenWidth / 2 - 60, screenHeight / 2 - 130, 16, ORANGE);
            DrawText("- WASD: Move Player", screenWidth / 2 - 160, screenHeight / 2 - 90, 14, WHITE);
            DrawText("- Left Click: Shoot Fireball", screenWidth / 2 - 160, screenHeight / 2 - 60, 14, WHITE);
            DrawText("- Space: Jump", screenWidth / 2 - 160, screenHeight / 2 - 30, 14, WHITE);
            DrawText("- Left Shift: Crouch", screenWidth / 2 - 160, screenHeight / 2, 14, WHITE);
            DrawText("- ESC: Pause / Settings Menu", screenWidth / 2 - 160, screenHeight / 2 + 30, 14, WHITE);
        }

        // Рисуване на Менюто при Пауза
        if (isPaused) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));
            DrawRectangle(screenWidth / 2 - 150, screenHeight / 2 - 120, 300, 240, RAYWHITE);
            DrawRectangleLines(screenWidth / 2 - 150, screenHeight / 2 - 120, 300, 240, BLUE);

            DrawText("GAME PAUSED", screenWidth / 2 - 60, screenHeight / 2 - 100, 16, BLACK);

            // Рендериране на Бутоните
            DrawRectangle(screenWidth / 2 - 100, screenHeight / 2 - 60, 200, 40, LIGHTGRAY);
            DrawText("Toggle Fullscreen", screenWidth / 2 - 75, screenHeight / 2 - 50, 14, BLACK);

            DrawRectangle(screenWidth / 2 - 60, screenHeight / 2, 40, 40, LIGHTGRAY);
            DrawText("+", screenWidth / 2 - 45, screenHeight / 2 + 10, 20, BLACK);

            DrawRectangle(screenWidth / 2 + 20, screenHeight / 2, 40, 40, LIGHTGRAY);
            DrawText("-", screenWidth / 2 + 35, screenHeight / 2 + 10, 20, BLACK);

            DrawText(TextFormat("Volume: %d%%", (int)(masterVolume * 100)), screenWidth / 2 - 50, screenHeight / 2 + 60, 14, BLACK);
        }

        EndDrawing();
    }

    // Изчистване на паметта
    UnloadSound(soundStep);
    UnloadSound(soundJump);
    UnloadSound(soundFireball);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

// Функциите UpdateBody, UpdateCameraFPS и DrawLevel остават същите...
void UpdateBody(Body* body, float rot, char side, char forward, bool jumpPressed, bool crouchHold) {
    Vector2 input = (Vector2){ (float)side, (float)-forward };
    float delta = GetFrameTime();
    if (!body->isGrounded) body->velocity.y -= GRAVITY * delta;
    if (body->isGrounded && jumpPressed) {
        body->velocity.y = JUMP_FORCE;
        body->isGrounded = false;
    }
    Vector3 front = (Vector3){ sinf(rot), 0.f, cosf(rot) };
    Vector3 right = (Vector3){ cosf(-rot), 0.f, sinf(-rot) };
    Vector3 desiredDir = (Vector3){ input.x * right.x + input.y * front.x, 0.0f, input.x * right.z + input.y * front.z, };
    body->dir = Vector3Lerp(body->dir, desiredDir, CONTROL * delta);
    float decel = (body->isGrounded ? FRICTION : AIR_DRAG);
    Vector3 hvel = (Vector3){ body->velocity.x * decel, 0.0f, body->velocity.z * decel };
    float hvelLength = Vector3Length(hvel);
    if (hvelLength < (MAX_SPEED * 0.01f)) hvel = (Vector3){ 0 };
    float speed = Vector3DotProduct(hvel, body->dir);
    float maxSpeed = (crouchHold ? CROUCH_SPEED : MAX_SPEED);
    float accel = Clamp(maxSpeed - speed, 0.f, MAX_ACCEL * delta);
    hvel.x += body->dir.x * accel; hvel.z += body->dir.z * accel;
    body->velocity.x = hvel.x; body->velocity.z = hvel.z;
    body->position.x += body->velocity.x * delta; body->position.y += body->velocity.y * delta; body->position.z += body->velocity.z * delta;
    if (body->position.y <= 0.0f) { body->position.y = 0.0f; body->velocity.y = 0.0f; body->isGrounded = true; }
}

static void UpdateCameraFPS(Camera* camera) {
    const Vector3 up = (Vector3){ 0.0f, 1.0f, 0.0f };
    const Vector3 targetOffset = (Vector3){ 0.0f, 0.0f, -1.0f };
    Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, lookRotation.x);
    float maxAngleUp = Vector3Angle(up, yaw) - 0.001f;
    if (-(lookRotation.y) > maxAngleUp) { lookRotation.y = -maxAngleUp; }
    float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw) * -1.0f + 0.001f;
    if (-(lookRotation.y) < maxAngleDown) { lookRotation.y = -maxAngleDown; }
    Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));
    float pitchAngle = -lookRotation.y - lean.y;
    pitchAngle = Clamp(pitchAngle, -PI / 2 + 0.0001f, PI / 2 - 0.0001f);
    Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitchAngle);
    float headSin = sinf(headTimer * PI); float headCos = cosf(headTimer * PI);
    camera->up = Vector3RotateByAxisAngle(up, pitch, headSin * 0.01f + lean.x);
    Vector3 bobbing = Vector3Scale(right, headSin * 0.1f); bobbing.y = fabsf(headCos * 0.15f);
    camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, walkLerp));
    camera->target = Vector3Add(camera->position, pitch);
}

static void DrawLevel(void) {
    const int floorExtent = 25; const float tileSize = 5.0f; const Color tileColor1 = (Color){ 150, 200, 200, 255 };
    for (int y = -floorExtent; y < floorExtent; y++) {
        for (int x = -floorExtent; x < floorExtent; x++) {
            if ((y & 1) && (x & 1)) DrawPlane((Vector3) { x* tileSize, 0.0f, y* tileSize }, (Vector2) { tileSize, tileSize }, tileColor1);
            else if (!(y & 1) && !(x & 1)) DrawPlane((Vector3) { x* tileSize, 0.0f, y* tileSize }, (Vector2) { tileSize, tileSize }, LIGHTGRAY);
        }
    }
    const Vector3 towerSize = (Vector3){ 16.0f, 32.0f, 16.0f }; const Color towerColor = (Color){ 150, 200, 200, 255 };
    Vector3 towerPos = (Vector3){ 16.0f, 16.0f, 16.0f };
    DrawCubeV(towerPos, towerSize, towerColor); DrawCubeWiresV(towerPos, towerSize, DARKBLUE);
    towerPos.x *= -1; DrawCubeV(towerPos, towerSize, towerColor); DrawCubeWiresV(towerPos, towerSize, DARKBLUE);
    towerPos.z *= -1; DrawCubeV(towerPos, towerSize, towerColor); DrawCubeWiresV(towerPos, towerSize, DARKBLUE);
    towerPos.x *= -1; DrawCubeV(towerPos, towerSize, towerColor); DrawCubeWiresV(towerPos, towerSize, DARKBLUE);
    DrawSphere((Vector3) { 300.0f, 300.0f, 0.0f }, 100.0f, (Color) { 255, 0, 0, 255 });
}

// Функцията за стреляне е поставена най-отдолу, за да я следите лесно
void ShootFireball(Camera* camera, Vector3 playerPosition, Fireball* fireballs, int* fireballcount) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
    Fireball fb = { 0 };
    fb.position = Vector3Add(playerPosition, Vector3Scale(forward, 1.5f));
    fb.velocity = Vector3Scale(forward, 25.0f); // Малко по-бърза за по-добро усещане
    fb.lifetime = 3.0f;
    fb.isDead = false;

    if (*fireballcount < MAX_FIREBALLS) {
        fireballs[*fireballcount] = fb;
        (*fireballcount)++;
        PlaySound(soundFireball); // Пуска звук при успешно изстрелване
    }
}