#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#include "rlights.h"

#define GLSL_VERSION 330

#define NUM_GRASS_BLADES 15000
#define FIELD_SIZE 34.0f
#define PI 3.14159274101257324219f
#define DEG2RAD (PI / 180.0f)

Color background_color = (Color){255,255,185,100};
Color planeColor = (Color){20,42,19,255};

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

void ToggleFullScreen(int screenWidth, int screenHeight){
            if(IsWindowFullscreen()){
            int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            ToggleFullscreen();
        }
        else{
            ToggleFullscreen();
            SetWindowSize(screenWidth, screenHeight);
        }
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

void DrawGrassNew(Model model, float bendfactor )
{
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        DrawModelEx(model, grassBlades[i].position, (Vector3){1.0f, 1.0f, 0.0f}, (grassBlades[i].rotation*bendfactor), (Vector3){grassBlades[i].scale, grassBlades[i].scale, grassBlades[i].scale}, DARKGREEN);
    }
}

void DrawGrass(Model model, Material material, Matrix transform)
{
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        DrawMesh(model.meshes[0],material,transform);
    }
}


int main(void)
{
    int screenWidth = 880;
    int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "mmmm");

    Camera camera = {0};
    camera.position = (Vector3){-15.0f, 0.4f, 2.0f};
    camera.target = (Vector3){0.185f, 1.5f, -1.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.fovy = 50.0f;


    int cameraMode = CAMERA_THIRD_PERSON;

    Material material = LoadMaterialDefault();
    // material.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    // The Model
    Model model = LoadModel("./grass.obj");

        float time = 0.0f;
        float direction = 1;
    

    DisableCursor();
    SetTargetFPS(60);

    InitGrass();
    while (!WindowShouldClose())
    {   

        if (IsKeyPressed(KEY_C))
            ToggleFullScreen(screenWidth, screenHeight);

        time += GetFrameTime();
        float bendFactor = sinf(time * 2.0f) * 3.0f*DEG2RAD;
        // float minAngle=-45.0f*DEG2RAD;
        // float maxAngle=45.0f*DEG2RAD;
        // if (bendFactor>maxAngle){
        //     direction*=-1;
        // }
        // else if(bendFactor<minAngle){
        //     direction*=-1;
        // }
        // printf("%f\n",bendFactor);
        // bendFactor = fmaxf(minAngle, fminf(maxAngle, bendFactor));

        UpdateCamera(&camera, cameraMode);
        // Matrix transforms = MatrixMultiply(MatrixTranslate( 1.5f, 0.0f, 0.0f), MatrixRotateX(bendFactor-0.5f));


        BeginDrawing();
        ClearBackground(background_color);

        BeginMode3D(camera);
        DrawPlane((Vector3){0.0f, 0.0f, 0.0f},(Vector2){32.0f, 32.0f}, planeColor);
        DrawGrassNew(model, bendFactor);
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
