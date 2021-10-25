#pragma once

#include <Common_3/OS/Interfaces/IApp.h>
#include <Common_3/OS/Interfaces/IFileSystem.h>
#include <Common_3/OS/Interfaces/IFont.h>
#include <Common_3/OS/Interfaces/IInput.h>
#include <Common_3/OS/Interfaces/ILog.h>
#include <Common_3/OS/Interfaces/IProfiler.h>
#include <Common_3/OS/Interfaces/IScreenshot.h>
#include <Common_3/OS/Interfaces/IUI.h>
#include <Common_3/Renderer/IRenderer.h>
#include <Common_3/Renderer/IResourceLoader.h>
#include <Common_3/ThirdParty/OpenSource/renderdoc/renderdoc_app.h>

#include <array>
#include <memory>

#include "Scene.h"

class MainApp : public IApp {
  public:
    auto Init() -> bool override;
    void Exit() override;

    auto Load() -> bool override;
    void Unload() override;

    void Update(float deltaTime) override;
    void Draw() override;

    auto GetName() -> const char * override { return "Template Application"; };

    auto CanCapture() -> bool { return rdoc_api != nullptr; };
    void Capture() { bIsCapturing = true; }

    ProfileToken gGpuProfileToken = PROFILE_INVALID_TOKEN;

    static auto Instance() -> MainApp * { return pApp; }
    static constexpr int ImageCount = 3;

  private:
    auto AddSwapChain() -> bool;
    auto AddDepthBuffer() -> bool;

    Renderer *pRenderer = nullptr;
    Queue *pGraphicsQueue = nullptr;
    std::array<CmdPool *, ImageCount> pCmdPools = {nullptr};
    std::array<Cmd *, ImageCount> pCmds = {nullptr};

    SwapChain *pSwapChain = nullptr;
    RenderTarget *pDepthBuffer = nullptr;
    Semaphore *pImageAcquiredSemaphore = nullptr;
    std::array<Fence *, ImageCount> pRenderCompleteFences = {nullptr};
    std::array<Semaphore *, ImageCount> pRenderCompleteSemaphores = {nullptr};

    uint32_t gFrameIndex = 0;

    /// UI
    UIComponent *pGuiWindow{nullptr};
    FontDrawDesc gFrameTimeDraw;

    RENDERDOC_API_1_1_2 *rdoc_api = nullptr;

    Scene currentScene;
    bool bToggleVSync = false;
    bool bIsCapturing = false;
    bool bIsTakingScreenshot = false;

    uint32_t gFontID;

    static MainApp *pApp;
};
