//
// Created by mr_ta on 3/4/2023.
//

#ifndef DEMO2SCENE_H
#define DEMO2SCENE_H

#include "IInput.h"
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

    bool OnInputAction(InputActionContext *ctx);

private:
    static constexpr size_t OBJECT_COUNT = 3;

    int cubeVertexCount;
    Buffer *pCubeVertexBuffer;

    int sphereVertexCount;
    Buffer *pSphereVertexBuffer;

    Buffer **pObjectUniformBuffers;
    Buffer **pSceneUniformBuffer;

    ICameraController *pCameraController;

    Shader *pShader;
    RootSignature *pRootSignature;
    DescriptorSet *pDescriptorSetSceneUniform;
    DescriptorSet *pDescriptorSetObjectUniform;

    Pipeline *pPipeline;

    struct ObjectUniformBlock
    {
        mat4 Transform;
        vec4 Color;
    } objects[OBJECT_COUNT];

    enum class ObjectType
    {
        Cube,
        Sphere
    } objectTypes[OBJECT_COUNT];

    static constexpr size_t DIRECTIONAL_LIGHT_COUNT = 2;
    static constexpr size_t POINT_LIGHT_COUNT = 2;
    struct SceneUniformBlock
    {
        // Camera
        CameraMatrix mProjectView;
        // Ambient Light
        vec4 AmbientLight;
        // Directional Light;
        vec4 DirectionalLightDirection[DIRECTIONAL_LIGHT_COUNT];
        vec4 DirectionalLightColor[DIRECTIONAL_LIGHT_COUNT];
        // Point Light
        vec4 PointLightPosition[POINT_LIGHT_COUNT];
        vec4 PointLightColor[POINT_LIGHT_COUNT];
    } scene;
};

#endif // DEMO2SCENE_H
