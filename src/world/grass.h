#ifndef GRASS_H
#define GRASS_H

#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <time.h>

#define GLSL_VERSION 100

#define NUM_GRASS_BLADES 8000
#define PATCH_SIZE 30.0f


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


Model grassBlade(Shader lightShader)
{
    Model grass = LoadModel("../assets/grass.obj");
    grass.materials[0].shader = lightShader;
    return grass;
}

void InitGrass(Vector3 playerPosition, Shader lightShader)
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
        grassBlades[i].scale = GetRandomValue(5,15); // Random scale 1.0 between  and 15.0
        grassBlades[i].rotation = GetRandomValue(10, 270);
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

// void DrawGrassNew(Model model, float bendFactor)
// {
//     for (int i = 0; i < NUM_GRASS_BLADES; i++)
//     {
//         DrawModelEx(model, grassBlades[i].position, (Vector3){1.0f, 0.0f, 0.0f}, (grassBlades[i].rotation * bendFactor), (Vector3){grassBlades[i].scale, grassBlades[i].scale, grassBlades[i].scale}, DARKGRAY);
//     }
// }

void DrawGrassNew(Model grass,float bendFactor)
{
    for (int i = 0; i < NUM_GRASS_BLADES; i++)
    {
        DrawModelEx(grass, grassBlades[i].position, (Vector3){1.0f, 0.0f, 0.0f}, (grassBlades[i].rotation * bendFactor), (Vector3){grassBlades[i].scale, grassBlades[i].scale, grassBlades[i].scale}, DARKGRAY);
    }
}


#endif