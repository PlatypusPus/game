#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#define RLIGHTS_IMPLEMENTATION
#include "include/rlights.h"
#define RLGL_IMPLEMENTATION
#include <rlgl.h>

#define GLSL_VERSION 100

#define NUM_GRASS_BLADES 9000
#define PATCH_SIZE 28.0f


Color background_color = (Color){255, 255, 185, 100};
Color planeColor = (Color){20, 42, 19, 255};
Color vColor = (Color){14, 13, 14, 1};

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

void ToggleFullScreen(int screenWidth, int screenHeight)
{
    if (IsWindowFullscreen())
    {
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    }
    else
    {
        ToggleFullscreen();
        SetWindowSize(screenWidth, screenHeight);
    }
}

void InitGrass(Vector3 playerPosition)
{
    srand(time(NULL)); // Seed for random number generation
    // Initialize grass blades with random positions and scales
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float distance = GetRandomValue(0, (int)(PATCH_SIZE * 10)) / 10.0f;

        float x = playerPosition.x + distance * cosf(angle);
        float z = playerPosition.z + distance * sinf(angle);
        float noise = Noise(x, z);

        grassBlades[i].position.x = x + noise;
        grassBlades[i].position.z = z + noise;
        grassBlades[i].position.y = 0.0f;                       // Initial height
        grassBlades[i].scale = GetRandomValue(1,15); // Random scale 1.0 between  and 15.0
        grassBlades[i].rotation = GetRandomValue(0, 360);
    }
}

void UpdateGrassPatches(Vector3 playerPosition, float patchSize)
{
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        if (grassBlades[i].position.x < playerPosition.x - patchSize / 2)
        {
            grassBlades[i].position.x += patchSize;
        }
        else if (grassBlades[i].position.x > playerPosition.x + patchSize / 2)
        {
            grassBlades[i].position.x -= patchSize;
        }

        if (grassBlades[i].position.z < playerPosition.z - patchSize / 2)
        {
            grassBlades[i].position.z += patchSize;
        }
        else if (grassBlades[i].position.z > playerPosition.z + patchSize / 2)
        {
            grassBlades[i].position.z -= patchSize;
        }
    }
}

void DrawGrassNew(Model model, float bendFactor)
{
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        DrawModelEx(model, grassBlades[i].position, (Vector3){1.0f, 0.0f, 0.0f}, (grassBlades[i].rotation * bendFactor), (Vector3){grassBlades[i].scale, grassBlades[i].scale, grassBlades[i].scale}, DARKGRAY);
    }
}

int main(void)
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

//Lights implementation
    Shader lightShader = LoadShader(TextFormat("shaders/glsl%i/lighting.vs",GLSL_VERSION),TextFormat("shaders/glsl%i/lighting.fs",GLSL_VERSION));
    lightShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(lightShader, "viewPos");
    // lightShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(lightShader, "matModel");
    int ambientLoc = GetShaderLocation(lightShader,"ambient");
    SetShaderValue(lightShader, ambientLoc, (float[4]){ 0.05f, 0.075f, 0.07f, 1.0f }, SHADER_UNIFORM_VEC4);
    float attenuation = 0.05f;
    int attenuationLoc = GetShaderLocation(lightShader, "attenuationLoc");
    SetShaderValue(lightShader, attenuationLoc, &attenuation, SHADER_UNIFORM_FLOAT);
    

//Creating Lights
    Light lights[MAX_LIGHTS] = {0};
    lights[0] = CreateLight(LIGHT_POINT,(Vector3){0.0f,1.5f,0.0f},Vector3Zero(),WHITE,attenuation,lightShader);

//Grass blade
    Model model = LoadModel("assets/grass.obj");
    model.materials[0].shader = lightShader;


    Mesh floor = GenMeshPlane(32.0f, 32.0f, 1, 1);
    Model ground = LoadModelFromMesh(floor);

    Texture2D grassfloor = LoadTexture("assets/grass.png");
    ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grassfloor;
    Shader vig_shader = LoadShader(0, "shaders/vignette.fs");


    int rLoc = GetShaderLocation(vig_shader, "radius");
    int blurLoc = GetShaderLocation(vig_shader, "blur");
    int colLoc = GetShaderLocation(vig_shader, "color");

//Skybox
    Mesh cube = GenMeshCube(1.0f,1.0f,1.0f);
    Model skybox = LoadModelFromMesh(cube);
    skybox.materials[0].shader = LoadShader(TextFormat("shaders/glsl%i/skybox.vs",GLSL_VERSION),TextFormat("shaders/glsl%i/skybox.fs",GLSL_VERSION));
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"doGamma"), (int[1]){0}, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"vflipped"), (int[1]){0}, SHADER_UNIFORM_INT);
    Shader shdrCubemap = LoadShader(TextFormat("shaders/glsl%i/cubemap.vs",100),TextFormat("shaders/glsl%i/cubemap.fs",100));
    SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), (int[1]){ 0 }, SHADER_UNIFORM_INT);

    Image img = LoadImage("assets/skyBoxx.png");
    TextureCubemap cubeMapTexture = LoadTextureCubemap(img,CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(img);
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = cubeMapTexture;




    float radius = 0.125f;
    float blur = 0.48f;

    RenderTexture2D vTexture = LoadRenderTexture(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())); // Vignette texture.

    float time = 0.0f;

    DisableCursor();
    SetTargetFPS(60);
    InitGrass(camera.position);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F))
            ToggleFullScreen(screenWidth, screenHeight);

        time += GetFrameTime();
        float bendFactor = sinf(time * 2.0f) * 3.0f * DEG2RAD;

        UpdateCamera(&camera, cameraMode);
        UpdateGrassPatches(camera.target, PATCH_SIZE);

        SetShaderValue(vig_shader, rLoc, &radius, SHADER_UNIFORM_FLOAT);
        SetShaderValue(vig_shader, blurLoc, &blur, SHADER_UNIFORM_FLOAT);
        SetShaderValue(vig_shader, colLoc, &vColor, SHADER_UNIFORM_VEC3);

        
// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(lightShader, lightShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        Vector3 light_head = (Vector3){0+camera.target.x,5+camera.target.y,8+camera.target.z};
        lights[0].position = light_head;
// Update light values (actually, only enable/disable them)
        UpdateLightValues(lightShader,lights[0]);


        //Draw
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        BeginShaderMode(lightShader);
        rlDisableBackfaceCulling();
        rlDisableDepthMask();
            DrawModel(skybox, (Vector3){0,0,0},30.0f,BLACK);
        rlEnableDepthMask();
        DrawGrassNew(model, bendFactor);

        if (cameraMode == CAMERA_THIRD_PERSON)
        {
            DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
            DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
        }

        rlEnableBackfaceCulling();
        DrawModelEx(ground, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 90.0f * DEG2RAD, (Vector3){1.0f, 1.0f, 1.0f}, planeColor);
        // DrawPlane(Vector3Zero(),(Vector2){30,30},planeColor);
        // if (lights[0].enabled) 
        //     DrawSphereEx(lights[0].position, 0.2f, 8, 8, lights[0].color);
        // else 
        //     DrawSphereWires(lights[0].position, 0.2f, 8, 8, ColorAlpha(lights[0].color, 0.3f));

        EndMode3D();
        BeginShaderMode(vig_shader);
        DrawTextureRec(vTexture.texture, (Rectangle){0, 0, vTexture.texture.width, -vTexture.texture.height}, (Vector2){0, 0}, BLANK);
        EndShaderMode();

        EndDrawing();
    }

    UnloadModel(model);
    UnloadModel(ground);
    UnloadModel(skybox);
    UnloadShader(vig_shader);
    UnloadShader(skybox.materials[0].shader);
    UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);

    UnloadTexture(grassfloor);
    CloseWindow();

    return 0;
}