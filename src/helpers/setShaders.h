#ifndef SHADERS_H
#define SHADERS_H
#define RLIGHTS_IMPLEMENTATION
#define RLGL_IMPLEMENTATION

#include <raylib.h>
#include <raymath.h>
#include "../include/rlights.h"
#define GLSL_VERSION 100


Color background_color = (Color){255, 255, 185, 100};
Color planeColor = (Color){20, 42, 19, 255};
Color vColor = (Color){14, 13, 14, 1};

Light lights[MAX_LIGHTS] = {0};
Shader SetLights()
{
    //Light Shader
    Shader lightShader = LoadShader(TextFormat("../shaders/glsl%i/lighting.vs",GLSL_VERSION),TextFormat("../shaders/glsl%i/lighting.fs",GLSL_VERSION));
    lightShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(lightShader, "viewPos");
    int ambientLoc = GetShaderLocation(lightShader,"ambient");
    float attenuation = 0.05f;
    int attenuationLoc = GetShaderLocation(lightShader, "attenuationLoc");
    SetShaderValue(lightShader, ambientLoc, (float[4]){ 0.05f, 0.075f, 0.07f, 1.0f }, SHADER_UNIFORM_VEC4);
    SetShaderValue(lightShader, attenuationLoc, &attenuation, SHADER_UNIFORM_FLOAT);
    

    lights[0] = CreateLight(LIGHT_POINT,(Vector3){0.0f,1.5f,0.0f},Vector3Zero(),WHITE,attenuation,lightShader);
    return lightShader;
}

void lightShaderUpdate(Camera camera,Shader lightShader)
{
// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(lightShader, lightShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        Vector3 light_head = (Vector3){0+camera.target.x,5+camera.target.y,8+camera.target.z};
        lights[0].position = light_head;
// Update light values (actually, only enable/disable them)
        UpdateLightValues(lightShader,lights[0]);
}

Shader SetVignette()
{
    //Vignetter effect Shader
    Shader vig_shader = LoadShader(0, "../shaders/vignette.fs");
    int rLoc = GetShaderLocation(vig_shader, "radius");
    int blurLoc = GetShaderLocation(vig_shader, "blur");
    int colLoc = GetShaderLocation(vig_shader, "color");
    float radius = 0.125f;
    float blur = 0.65f;
    SetShaderValue(vig_shader, rLoc, &radius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(vig_shader, blurLoc, &blur, SHADER_UNIFORM_FLOAT);
    SetShaderValue(vig_shader, colLoc, &vColor, SHADER_UNIFORM_VEC3);
    return vig_shader;
}


#endif