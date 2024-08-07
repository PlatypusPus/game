#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#include "rlights.h"
#include <rlgl.h>

#define GLSL_VERSION 330

#define NUM_GRASS_BLADES 12000
#define PATCH_SIZE 28.0f
#define PI 3.14159274101257324219f
#define DEG2RAD (PI / 180.0f)

Color background_color = (Color){255, 255, 185, 100};
// Color planeColor = (Color){20, 42, 19, 255};
Color planeColor = (Color){10, 12, 19, 255};
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
        grassBlades[i].scale = GetRandomValue(10, 100) / 10.0f; // Random scale between 1.0 and 8.0
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
        DrawModelEx(model, grassBlades[i].position, (Vector3){1.0f, 0.0f, 0.0f}, (grassBlades[i].rotation * bendFactor), (Vector3){grassBlades[i].scale, grassBlades[i].scale, grassBlades[i].scale}, DARKGREEN);
    }
}

int main(void)
{
    int screenWidth = 880;
    int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    // SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "GHOST");

    Camera camera = {0};
    camera.position = (Vector3){-15.0f, 0.4f, 2.0f};
    camera.target = (Vector3){0.185f, 1.5f, -1.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.fovy = 50.0f;

    int cameraMode = CAMERA_THIRD_PERSON;

    Model model = LoadModel("assets/grass.obj");
    Mesh floor = GenMeshPlane(32.0f, 32.0f, 1, 1);
    Model ground = LoadModelFromMesh(floor);

    Texture2D grassfloor = LoadTexture("assets/grass.png");
    ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grassfloor;

    // Defining Shaders
    Shader vig_shader = LoadShader(0, "shaders/vignette.fs");
    Shader bloom = LoadShader(0,TextFormat("shaders/glsl%i/bloom.fs",GLSL_VERSION));

    int rLoc = GetShaderLocation(vig_shader, "radius");
    int blurLoc = GetShaderLocation(vig_shader, "blur");
    int colLoc = GetShaderLocation(vig_shader, "color");


    float radius = 0.125f;
    float blur = 0.48f;
    float daytime = 0.2f; // range (0, 1) but is sent to shader as a range(-1, 1) normalized upon a unit sphere
	float dayspeed = 0.015f;
    const float fboSize = 2.5f;
	bool dayrunning = true; // if day is animating

    RenderTexture2D vTexture = LoadRenderTexture(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())); // Vignette texture.
    RenderTexture2D bTexture = LoadRenderTexture(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())); // Vignette texture.
    RenderTexture2D reflectionBuffer = LoadRenderTexture(GetScreenWidth() / fboSize, GetScreenHeight() / fboSize); // FBO used for water reflection
	RenderTexture2D refractionBuffer = LoadRenderTexture(GetScreenWidth() / fboSize, GetScreenHeight() / fboSize); // FBO used for water refraction
    SetTextureFilter(reflectionBuffer.texture,TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(refractionBuffer.texture,TEXTURE_FILTER_BILINEAR);


    //Skybox
    Mesh cube = GenMeshCube(1.0f,1.0f,1.0f);
    Model skybox = LoadModelFromMesh(cube);
    skybox.materials[0].shader = LoadShader("assets/skybox.vert","assets/skybox.frag");
    int skyboxDaytimeLoc = GetShaderLocation(skybox.materials[0].shader, "daytime");
    int skyboxDayrotationLoc = GetShaderLocation(skybox.materials[0].shader, "dayrotation");
    float skyboxMoveFactor = 0.0f;
    int skyboxMoveFactorLoc = GetShaderLocation(skybox.materials[0].shader, "movefactor");
    Shader shdrCubemap = LoadShader("assets/cubemap.vert","assets/cubemap.frag");
    int param[1] = {MATERIAL_MAP_CUBEMAP};
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"environmentMapNight"),param, SHADER_UNIFORM_INT);
    int param2[1] = {MATERIAL_MAP_IRRADIANCE};
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"environmentMapDay"),param2, SHADER_UNIFORM_INT);
    int param3[1] = {0};
    SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap,"equirectangularMap"),param3, SHADER_UNIFORM_INT);
    Texture2D texHDR = LoadTexture("assets/milkyWay.hdr");
    Texture2D texHDR2 = LoadTexture("assets/daytime.hdr");
    skybox.materials[0].maps[0].texture = LoadTexture("assets/skyGradient.png");
    SetTextureFilter(skybox.materials[0].maps[0].texture,TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(skybox.materials[0].maps[0].texture,TEXTURE_WRAP_CLAMP);
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubemap,texHDR,1024,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    skybox.materials[0].maps[MATERIAL_MAP_IRRADIANCE].texture = GenTextureCubemap(shdrCubemap,texHDR2,1024,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    SetTextureFilter(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture,TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(skybox.materials[0].maps[MATERIAL_MAP_IRRADIANCE].texture,TEXTURE_FILTER_BILINEAR);
    GenTextureMipmaps(&skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
    GenTextureMipmaps(&skybox.materials[0].maps[MATERIAL_MAP_IRRADIANCE].texture);
    UnloadTexture(texHDR);      // Texture not required anymore, cubemap already generated
	UnloadTexture(texHDR2);      // Texture not required anymore, cubemap already generated 
	UnloadShader(shdrCubemap);  // Unload cubemap generation shader, not required anymore
    
    


    


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

        //animate daytime clouds
        skyboxMoveFactor += 0.0085f * GetFrameTime();
        while(skyboxMoveFactor>1.0f)
        {
            skyboxMoveFactor -=1.0f;
        }
        SetShaderValue(skybox.materials[0].shader,skyboxMoveFactorLoc,&skyboxMoveFactor, SHADER_UNIFORM_FLOAT);

        //animate daytime
        if(dayrunning)
        {
            daytime+=  dayspeed * GetFrameTime();
            while(daytime>1.0f)
            {
                daytime-=1.0;
            }
        }
        if (IsKeyDown(KEY_SPACE))
		{
			daytime += dayspeed * (5.0f - (float)dayrunning) * GetFrameTime();
			while (daytime > 1.0f)
			{
				daytime -= 1.0;
			}
		}
        float sunAngle = Lerp(-90, 270, daytime) * DEG2RAD; // -90 midnight, 90 midday
		float nDaytime = sinf(sunAngle); // normalize it to make it look like a dot product on an unit sphere (shaders expect it this way) (-1, 1)
		SetShaderValue(skybox.materials[0].shader, skyboxDaytimeLoc, &nDaytime, SHADER_UNIFORM_FLOAT);
		SetShaderValue(skybox.materials[0].shader, skyboxDayrotationLoc, &daytime, SHADER_UNIFORM_FLOAT);





        UpdateCamera(&camera, cameraMode);
        UpdateGrassPatches(camera.target, PATCH_SIZE);

        SetShaderValue(vig_shader, rLoc, &radius, SHADER_UNIFORM_FLOAT);
        SetShaderValue(vig_shader, blurLoc, &blur, SHADER_UNIFORM_FLOAT);
        SetShaderValue(vig_shader, colLoc, &vColor, SHADER_UNIFORM_VEC3);
        
        BeginDrawing();

        BeginTextureMode(vTexture);
        BeginTextureMode(reflectionBuffer);
        //  BeginTextureMode(bTexture);
        ClearBackground(BLACK);
        EndTextureMode();

        BeginTextureMode(refractionBuffer);
        ClearBackground(GREEN);
        EndTextureMode();


        BeginMode3D(camera);
        rlDisableBackfaceCulling();
        DrawModel(skybox,(Vector3){0,0,0},1.0f,WHITE);
        DrawGrassNew(model, bendFactor);

        if (cameraMode == CAMERA_THIRD_PERSON)
        {
            DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
            DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
        }

        rlEnableBackfaceCulling();
        DrawModelEx(ground, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 90.0f * DEG2RAD, (Vector3){1.0f, 1.0f, 1.0f}, planeColor);
        EndMode3D();
        EndTextureMode();

        // BeginShaderMode(bloom);
        // DrawTextureRec(bTexture.texture, (Rectangle){0, 0, bTexture.texture.width, -bTexture.texture.height}, (Vector2){0, 0}, BLANK);

        BeginShaderMode(vig_shader);
        DrawTextureRec(vTexture.texture, (Rectangle){0, 0, vTexture.texture.width, -vTexture.texture.height}, (Vector2){0, 0}, BLANK);
        EndShaderMode();


        EndDrawing();
    }

    UnloadModel(model);
    UnloadModel(ground);
    UnloadShader(vig_shader);
    UnloadShader(bloom);
    UnloadTexture(grassfloor);
    CloseWindow();

    return 0;
}

