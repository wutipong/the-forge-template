#pragma once

#include <Common_3/OS/Interfaces/IApp.h>
#include <Common_3/OS/Interfaces/IFileSystem.h>
#include <Common_3/OS/Interfaces/IInput.h>
#include <Common_3/OS/Interfaces/ILog.h>
#include <Common_3/OS/Interfaces/IProfiler.h>
#include <Common_3/OS/Interfaces/IScreenshot.h>
#include <Common_3/Renderer/IRenderer.h>
#include <Common_3/Renderer/IResourceLoader.h>
#include <Common_3/ThirdParty/OpenSource/renderdoc/renderdoc_app.h>
#include <Middleware_3/UI/AppUI.h>

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
    void Capture() { isCapturing = true; }
    void TakeScreenshot() { isTakingScreenshot = true; }

    template <class SceneClass> void ChangeScene() {
        if (currentScene != nullptr) {
            currentScene->Unload();
        }

        currentScene = std::make_unique<SceneClass>();
        currentScene->Load();
    }

    UIApp appUI;
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
    GuiComponent *pGuiWindow = nullptr;
    TextDrawDesc gFrameTimeDraw = TextDrawDesc(0, 0xff00ffff, 18);

    RENDERDOC_API_1_1_2 *rdoc_api = nullptr;

    Scene currentScene;
    bool bToggleVSync = false;
    bool isTakingScreenshot = false;
    bool isCapturing = false;

    static MainApp *pApp;
};
