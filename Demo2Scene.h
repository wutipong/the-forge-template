//
// Created by mr_ta on 3/4/2023.
//

#ifndef DEMO2SCENE_H
#define DEMO2SCENE_H

#include "IInput.h"
#include "IScene.h"
#include "IUI.h"
#include "ShapeDrawer.h"

class Demo2Scene : public IScene
{
public:
    void Init(uint32_t imageCount) override;
    void Exit() override;
    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer,
              uint32_t imageCount) override;
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer) override;
    void Update(float deltaTime, uint32_t width, uint32_t height) override;
    void PreDraw(uint32_t frameIndex) override;
    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer,
              uint32_t frameIndex) override;

    bool OnInputAction(InputActionContext *ctx);

private:
    static constexpr size_t OBJECT_COUNT = 3;

    ShapeDrawer shapeDrawer;

    Buffer **pUbObjects{};
    Buffer **pUbScene{};
    Buffer **pUbLightSources{};

    ICameraController *pCameraController{};

    Shader *pShObjects{};
    Shader *pShShadow{};
    Shader *pShLightSources{};

    RootSignature *pRootSignature{};

    DescriptorSet *pDsSceneUniform{};
    DescriptorSet *pDsObjectUniform{};
    DescriptorSet *pDsLightSourcesUniform{};
    DescriptorSet *pDsTexture{};

    Pipeline *pPlObjects{};
    Pipeline *pPlShadow{};
    Pipeline *pPlLightSources{};

    struct ObjectUniformBlock
    {
        mat4 Transform;
        float4 Color;
    };

    ObjectUniformBlock objects[OBJECT_COUNT]{};

    ShapeDrawer::Shape objectTypes[OBJECT_COUNT]{};

    static constexpr size_t DIRECTIONAL_LIGHT_COUNT = 2;
    struct SceneUniformBlock
    {
        // Camera
        vec4 CameraPosition;
        CameraMatrix ProjectView;

        // Directional Light;
        float4 LightDirection[DIRECTIONAL_LIGHT_COUNT];
        float4 LightColor[DIRECTIONAL_LIGHT_COUNT];
        float4 LightAmbient[DIRECTIONAL_LIGHT_COUNT];
        float4 LightIntensity[DIRECTIONAL_LIGHT_COUNT];

        // Shadow
        mat4 ShadowTransform;
    } scene{};

    ObjectUniformBlock lightSources[DIRECTIONAL_LIGHT_COUNT] = {};

    UIComponent *pObjectWindow{};

    static constexpr float SHADOW_MAP_DIMENSION = 1024;
    RenderTarget *pRtShadow;

    void ResetLightSettings();
    void DrawShadowRT(Cmd *&pCmd, uint32_t frameIndex);
};

#endif // DEMO2SCENE_H
