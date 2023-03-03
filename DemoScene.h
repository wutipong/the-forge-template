#ifndef DEMO_SCENE_H
#define DEMO_SCENE_H

#include "IScene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IOperatingSystem.h>
#include <Math/MathTypes.h>

class DemoScene : public IScene
{
public:
    void Init(uint32_t imageCount) override;
    void Exit() override;
    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer,
              uint32_t imageCount) override;
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer) override;
    void Update(float deltaTime, uint32_t width, uint32_t height) override;
    void PreDraw(uint32_t frameIndex) override;
    void Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex) override;

private:
    int vertexCount{};
    static constexpr size_t MAX_STARS = 768;

    vec3 position[MAX_STARS] = {};
    vec4 color[MAX_STARS] = {};
    vec3 lightPosition{1.0f, 0, 0};
    vec3 lightColor{0.9f, 0.9f, 0.7f};

    Shader *pShader;
    RootSignature *pRootSignature;
    DescriptorSet *pDescriptorSetUniforms;
    Buffer **pProjViewUniformBuffer = nullptr;
    Buffer *pSphereVertexBuffer = nullptr;
    Pipeline *pSpherePipeline;

    ICameraController *pCameraController;

    struct UniformBlock
    {
        CameraMatrix mProjectView;
        mat4 mToWorldMat[MAX_STARS];
        vec4 mColor[MAX_STARS];

        vec3 mLightPosition;
        vec3 mLightColor;
    } uniform = {};
};


#endif // DEMO_SCENE_H
