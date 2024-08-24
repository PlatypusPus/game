#ifndef SKYBOX_H
#define SKYBOX_H
#define GLSL_VERSION 100
#include <raylib.h>
#include <raymath.h>

Model skyBox(){
    Mesh cube = GenMeshCube(1.0f,1.0f,1.0f);
    Model skybox = LoadModelFromMesh(cube);
    skybox.materials[0].shader = LoadShader(TextFormat("../shaders/glsl%i/skybox.vs",GLSL_VERSION),TextFormat("../shaders/glsl%i/skybox.fs",GLSL_VERSION));
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"doGamma"), (int[1]){0}, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader,"vflipped"), (int[1]){0}, SHADER_UNIFORM_INT);
    Shader shdrCubemap = LoadShader(TextFormat("../shaders/glsl%i/cubemap.vs",100),TextFormat("../shaders/glsl%i/cubemap.fs",100));
    SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), (int[1]){ 0 }, SHADER_UNIFORM_INT);

    Image img = LoadImage("../assets/skyBox.png");
    TextureCubemap cubeMapTexture = LoadTextureCubemap(img,CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(img);
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = cubeMapTexture;
    
    return skybox;
}
#endif