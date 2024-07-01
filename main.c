#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define GLSL_VERSION 330

#define NUM_GRASS_BLADES 12000
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
        grassBlades[i].scale = GetRandomValue(10, 100) / 10.0f; // Random scale between 1.0 and 8.0
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

    SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib");

    Camera camera = {0};
    camera.position = (Vector3){0.0f, 4.0f, -100.0f};
    camera.target = (Vector3){0.0f, 3.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.fovy = 5.0f;
    // CameraYaw(&camera, -135 * DEG2RAD, true);
    // CameraPitch(&camera, -45 * DEG2RAD, true, true, false);

    int cameraMode = CAMERA_THIRD_PERSON;

    // The Model
    Model model = LoadModel("./grass.obj");

    // Loading the shader and setting up uniforms
    Shader shader = LoadShader(TextFormat("shaders/glsl%i/lighting.vs", GLSL_VERSION), TextFormat("shaders/glsl%i/fog.fs", GLSL_VERSION));
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    // Ambient light level
    int ambienLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambienLoc, (float[4]){0.3f, 0.3f, 0.3f, 1.0f}, SHADER_UNIFORM_VEC4);

    float fogDensity = 0.01f;
    int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
    SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);

    Vector3 fogColor = {1.0f, 1.0f, 1.0f}; // Light gray-blue fog color
    int fogColorLoc = GetShaderLocation(shader, "fogColor");
    SetShaderValue(shader, fogColorLoc, &fogColor, SHADER_UNIFORM_VEC3);
    model.materials[0].shader = shader;

    // Creating light
    CreateLight(LIGHT_POINT, (Vector3){0, 5, 6}, Vector3Zero(), WHITE, shader);

    DisableCursor();
    SetTargetFPS(60);

    InitGrass();
    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, cameraMode);

        SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);

        // Updating the ligth shader with the current camera position
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position.x, SHADER_UNIFORM_VEC3);

        BeginDrawing();
        ClearBackground(GRAY);

        BeginMode3D(camera);
        DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){32.0f, 32.0f}, GRAY);
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
    UnloadShader(shader);
    CloseWindow();
    return 0;
}
