#include <raylib.h>
#define RLGL_IMPLEMENTATION
#include "include/rlights.h"
#include <rlgl.h>

//header imports
#include "helpers/screen.h"
#include "helpers/setShaders.h"
#include "world/grass.h"
#include "world/skybox.h"
#include "world/ground.h"


#define GLSL_VERSION 100

int main()
{
    int screenWidth = 880;
    int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "GHOST");

    Camera camera = {0};
    camera.position = (Vector3){-15.0f, 2.0f, 2.0f};
    camera.target = (Vector3){0.185f, 1.5f, -1.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.fovy = 60.0f;

    int cameraMode = CAMERA_THIRD_PERSON;
    float time = 0.0f;

    //Lights
    Shader lightShader = SetLights();

    //Skybox
    Model skybox = skyBox();
    //Floor
    Model ground = Ground();
    //Initial Grass
    Model grass = grassBlade(lightShader);
    InitGrass(camera.position, lightShader);
    //Vignette
    Shader vig_shader = SetVignette();
    RenderTexture2D vTexture = LoadRenderTexture(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())); // Vignette texture.

    DisableCursor();
    SetTargetFPS(60);
    

    while(!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F))
            ToggleFullScreen(screenWidth, screenHeight);
        time += GetFrameTime();
        float bendFactor = sinf(time * 2.0f) * 3.0f * DEG2RAD;

        UpdateCamera(&camera, cameraMode);
        //Updating Grass
        UpdateGrassPatches(camera.target, PATCH_SIZE);
        //Update the shader with the camera view vector
        lightShaderUpdate(camera,lightShader);


        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        BeginShaderMode(lightShader);
        rlDisableBackfaceCulling();

        //Draw Skybox
        rlDisableDepthMask();
            DrawModel(skybox, (Vector3){0,0,0},30.0f,BLACK);
        rlEnableDepthMask();

        //Draw grass blades
        DrawGrassNew(grass,bendFactor);

        if (cameraMode == CAMERA_THIRD_PERSON)
        {
            DrawCube(camera.target, 0.5f, 1.5f, 0.5f, RAYWHITE);
            DrawCubeWires(camera.target, 0.5f, 2.5f, 0.5f, WHITE);
        }
        rlEnableBackfaceCulling();

        //Draw Ground
        DrawModelEx(ground, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 90.0f * DEG2RAD, (Vector3){1.0f, 1.0f, 1.0f}, planeColor);

        EndMode3D();

        //Draw Vignette
        BeginShaderMode(vig_shader);
        DrawTextureRec(vTexture.texture, (Rectangle){0, 0, vTexture.texture.width, -vTexture.texture.height}, (Vector2){0, 0}, BLANK);
        EndShaderMode();
        
        float frameTime = GetFrameTime();  // Time taken for one frame
        DrawFPS(10,30);
        DrawText(TextFormat("Frame Time: %02.02f ms", frameTime * 1000), 10, 10, 20, RED);
        EndDrawing();
    }


    UnloadModel(ground);
    UnloadModel(skybox);
    UnloadShader(vig_shader);
    UnloadShader(skybox.materials[0].shader);
    UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);

    //Unload Models
    return 0;
}