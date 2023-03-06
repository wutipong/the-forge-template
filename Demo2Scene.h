//
// Created by mr_ta on 3/4/2023.
//

#ifndef DEMO2SCENE_H
#define DEMO2SCENE_H

#include "IScene.h"

class Demo2Scene : public IScene
{
public:
    virtual void Init(uint32_t imageCount);
    virtual void Exit();
    virtual void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
                      RenderTarget *pDepthBuffer, uint32_t imageCount);
    virtual void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    virtual void Update(float deltaTime, uint32_t width, uint32_t height);
    virtual void PreDraw(uint32_t frameIndex);
    virtual void Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex);

private:
    static constexpr size_t CUBE_COUNT = 2;

    int cubeVertexCount;
    Buffer *pCubeVertexBuffer;

    Buffer **pCubeUniformBuffers;
    Buffer **pSceneUniformBuffer;

    ICameraController *pCameraController;

    Shader *pShader;
    RootSignature *pRootSignature;
    DescriptorSet *pDescriptorSetScene;
    DescriptorSet *pDescriptorSetCube;

    Pipeline *pPipeline;

    struct CubeUniformBlock
    {
        mat4 Transform;
        vec4 Color;
    } cubes[CUBE_COUNT];

    static constexpr size_t POINT_LIGHT_COUNT = 2;
    struct SceneUniformBlock
    {
        //Camera
        CameraMatrix mProjectView;
        // Ambient Light
        vec4 AmbientLight;
        // Point Light
        vec4 PointLightPosition[POINT_LIGHT_COUNT];
        vec4 PointLightColor[POINT_LIGHT_COUNT];
    } scene;
};

#endif // DEMO2SCENE_H
