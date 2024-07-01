#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define GLSL_VERSION 330


#define NUM_GRASS_BLADES 8000
#define FIELD_SIZE 34.0f

typedef struct
{
    Vector3 position;
    float scale;
    float rotation;
} GrassBlade;

GrassBlade grassBlades[NUM_GRASS_BLADES];

float Noise(float x, float y)
{
    return sinf(x * 0.1f) * cosf(y * 0.1f) * 5.0f;
}

void InitGrass()
{
    srand(time(NULL)); // Seed for random number generation
    // Initialize grass blades with random positions and scales
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        float clusterX = GetRandomValue(-FIELD_SIZE / 2.0f, FIELD_SIZE / 2.0f);
        float clusterZ = GetRandomValue(-FIELD_SIZE / 2.0f, FIELD_SIZE / 2.0f);
        float clusterRadius = GetRandomValue(0, 5);

        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float distance = GetRandomValue(0, (int)(clusterRadius * 10)) / 10.0f;

        float x = clusterX + distance * cosf(angle);
        float z = clusterZ + distance * sinf(angle);
        float noise = Noise(x, z);

        grassBlades[i].position.x = x + noise;
        grassBlades[i].position.z = z + noise;
        grassBlades[i].position.y = 0.0f;                       // Initial height
        grassBlades[i].scale = GetRandomValue(10, 200) / 10.0f; // Random scale between 1.0 and 8.0
        grassBlades[i].rotation = GetRandomValue(0, 360);
    }
}

void DrawGrass(Model model)
{
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        DrawModelEx(model, grassBlades[i].position, (Vector3){0.0f, 1.0f, 0.0f}, grassBlades[i].rotation, (Vector3){grassBlades[i].scale, grassBlades[i].scale, grassBlades[i].scale}, DARKGREEN);
    }
}

int main(void)
{
    const int screenWidth = 880;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib");

    Camera camera = {0};
    camera.position = (Vector3){0.0f, 2.0f, -100.0f};
    camera.target = (Vector3){0.0f, 2.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.fovy = 7.0f;
    CameraYaw(&camera, -135 * DEG2RAD, true);
    CameraPitch(&camera, -45 * DEG2RAD, true, true, false);

    int cameraMode = CAMERA_THIRD_PERSON;

    // The Model
    Model model = LoadModel("./grass.obj");

    DisableCursor();
    SetTargetFPS(60);

    InitGrass();
    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, cameraMode);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){32.0f, 32.0f}, LIME);
        DrawGrass(model);
        if (cameraMode == CAMERA_THIRD_PERSON)
        {
            DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
            DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
        }

        EndMode3D();
        EndDrawing();
    }
    UnloadModel(model);
    CloseWindow();
    return 0;
}
