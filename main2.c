#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#define NUM_GRASS_BLADES 400
#define FIELD_SIZE 32.0f
#define MAX_SWAY_AMPLITUDE 0.2f
#define SWAY_SPEED 5.0f

typedef struct
{
    Vector3 position;
} GrassBlade;

GrassBlade grassBlades[NUM_GRASS_BLADES];

void InitGrass()
{
    // Initialize grass blades with random positions
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        grassBlades[i].position.x = GetRandomValue(-FIELD_SIZE / 2.0f, FIELD_SIZE / 2.0f);
        grassBlades[i].position.z = GetRandomValue(-FIELD_SIZE / 2.0f, FIELD_SIZE / 2.0f);
        grassBlades[i].position.y = 0.0f; // Initial height
    }
}
void DrawGrass()
{
    // Draw each grass blade as a triangle
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        // Define vertices of the triangle
        Vector3 v1 = {grassBlades[i].position.x - 0.1f, grassBlades[i].position.y, grassBlades[i].position.z};
        Vector3 v2 = {grassBlades[i].position.x + 0.1f, grassBlades[i].position.y, grassBlades[i].position.z};
        Vector3 v3 = {grassBlades[i].position.x, grassBlades[i].position.y + 2.0f, grassBlades[i].position.z};

        // Draw the triangle
        DrawTriangle3D(v1, v2, v3, DARKGREEN);
        DrawTriangle3D(v2, v1, v3, DARKGREEN);
    }
}

int main(void)
{
    const int screenWidth = 880;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "got it");

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
    Model model = LoadModel("./blade.obj");
    Texture2D texture = LoadTexture("./blade.png");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    Vector3 modelPosition = {0.0f, 0.0f, 0.0f};
    BoundingBox bounds = GetMeshBoundingBox(model.meshes[0]); // Setting mesh bounds

    DisableCursor();
    SetTargetFPS(60);

    InitGrass();

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, cameraMode);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){32.0f, 32.0f}, LIGHTGRAY);
        DrawGrass();
        if (cameraMode == CAMERA_THIRD_PERSON)
        {
            DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
            DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
        }
        DrawModel(model, modelPosition, 10.0f, GREEN);
        EndMode3D();
        EndDrawing();
    }
    UnloadModel(model);
    UnloadTexture(texture);
    CloseWindow();
    return 0;
}