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

class MainApp : public IApp {
  public:
    virtual bool Init() override;
    virtual void Exit() override;

    virtual bool Load() override;
    virtual void Unload() override;

    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    virtual const char *GetName() { return "Template Application"; };

    bool CanCapture() { return rdoc_api != nullptr; };
    void Capture() { isCapturing = true; }
    void TakeScreenshot() { isTakingScreenshot = true; }

    UIApp appUI;
    ProfileToken gGpuProfileToken = PROFILE_INVALID_TOKEN;

    static MainApp *Instance() { return pApp; }
    static constexpr int ImageCount = 3;

  private:
    bool AddSwapChain();
    bool AddDepthBuffer();

    Renderer *pRenderer = nullptr;
    Queue *pGraphicsQueue = nullptr;
    std::array<CmdPool *, ImageCount> pCmdPools = {nullptr};
    std::array<Cmd *, ImageCount>pCmds = {nullptr};

    SwapChain *pSwapChain = nullptr;
    RenderTarget *pDepthBuffer = nullptr;
    Semaphore *pImageAcquiredSemaphore = nullptr;
    std::array<Fence *, ImageCount>pRenderCompleteFences = {nullptr};
    std::array<Semaphore *, ImageCount>pRenderCompleteSemaphores = {nullptr};

    uint32_t gFrameIndex = 0;

    /// UI
    GuiComponent *pGuiWindow = nullptr;
    TextDrawDesc gFrameTimeDraw = TextDrawDesc(0, 0xff00ffff, 18);

    RENDERDOC_API_1_1_2 *rdoc_api = NULL;

    bool bToggleVSync = false;
    bool isTakingScreenshot = false;
    bool isCapturing = false;

    static MainApp *pApp;
};
