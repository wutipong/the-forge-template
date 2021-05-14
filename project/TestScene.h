#pragma once

#include "Scene.h"

#include "MainApp.h"

#include "Common_3/Renderer/IResourceLoader.h"
#include "Common_3/OS/Interfaces/ICameraController.h"

class TestScene : public Scene {
  public:
    void Update(float deltaTime) override;
    void Draw(Cmd *cmd, int imageIndex) override;

    auto Load(Renderer *pRenderer, SwapChain *pSwapChain) -> bool override;

    void Unload(Renderer *pRenderer) override;

    void DoUI() override;

  private:
    Geometry *pGeometry = nullptr;
    Texture *pTexture = nullptr;
    Shader *pShader = nullptr;
    Sampler *pSampler = nullptr;
    RootSignature *pRootSignature = nullptr;
    Pipeline *pPipeline = nullptr;

    std::array<Buffer *, MainApp::ImageCount> pUniformBuffers = {nullptr};
    DescriptorSet *pDescriptorSetTexture = {nullptr};
    DescriptorSet *pDescriptorSetUniforms = {nullptr};

    ICameraController *pCameraController = nullptr;
};
