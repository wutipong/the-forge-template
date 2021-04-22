#include <Common_3/OS/Interfaces/IApp.h>
#include <Common_3/OS/Interfaces/IFileSystem.h>
#include <Common_3/OS/Interfaces/IInput.h>
#include <Common_3/OS/Interfaces/ILog.h>
#include <Common_3/OS/Interfaces/IProfiler.h>
#include <Common_3/OS/Interfaces/IScreenshot.h>
#include <Common_3/Renderer/IRenderer.h>
#include <Common_3/Renderer/IResourceLoader.h>
#include <Middleware_3/UI/AppUI.h>

class MainApp : public IApp {
  public:
    virtual bool Init() override;
    virtual void Exit() override;

    virtual bool Load() override;
    virtual void Unload() override;

    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    virtual const char *GetName() { return "9-Patch test"; };

  private:
    bool addSwapChain();
    bool addDepthBuffer();
};

DEFINE_APPLICATION_MAIN(MainApp)
constexpr uint32_t gImageCount = 3;

Renderer *pRenderer = nullptr;
Queue *pGraphicsQueue = nullptr;
CmdPool *pCmdPools[gImageCount] = {nullptr};
Cmd *pCmds[gImageCount] = {nullptr};

SwapChain *pSwapChain = nullptr;
RenderTarget *pDepthBuffer = nullptr;
Fence *pRenderCompleteFences[gImageCount] = {nullptr};
Semaphore *pImageAcquiredSemaphore = nullptr;
Semaphore *pRenderCompleteSemaphores[gImageCount] = {nullptr};

RootSignature *pRootSignature = NULL;

uint32_t gFrameIndex = 0;
ProfileToken gGpuProfileToken = PROFILE_INVALID_TOKEN;

/// UI
UIApp gAppUI;
GuiComponent *pGuiWindow = nullptr;
TextDrawDesc gFrameTimeDraw = TextDrawDesc(0, 0xff00ffff, 18);

bool bToggleVSync = false;

bool gTakeScreenshot = false;
void takeScreenshot() {
    if (!gTakeScreenshot)
        gTakeScreenshot = true;
}

bool MainApp::addSwapChain() {
    SwapChainDesc swapChainDesc = {};
    swapChainDesc.mWindowHandle = pWindow->handle;
    swapChainDesc.mPresentQueueCount = 1;
    swapChainDesc.ppPresentQueues = &pGraphicsQueue;
    swapChainDesc.mWidth = mSettings.mWidth;
    swapChainDesc.mHeight = mSettings.mHeight;
    swapChainDesc.mImageCount = gImageCount;
    swapChainDesc.mColorFormat = getRecommendedSwapchainFormat(true);
    swapChainDesc.mEnableVsync = mSettings.mDefaultVSyncEnabled;
    ::addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);

    return pSwapChain != NULL;
}

bool MainApp::addDepthBuffer() {
    // Add depth buffer
    RenderTargetDesc depthRT = {};
    depthRT.mArraySize = 1;
    depthRT.mClearValue.depth = 0.0f;
    depthRT.mClearValue.stencil = 0;
    depthRT.mDepth = 1;
    depthRT.mFormat = TinyImageFormat_D32_SFLOAT;
    depthRT.mStartState = RESOURCE_STATE_DEPTH_WRITE;
    depthRT.mHeight = mSettings.mHeight;
    depthRT.mSampleCount = SAMPLE_COUNT_1;
    depthRT.mSampleQuality = 0;
    depthRT.mWidth = mSettings.mWidth;
    depthRT.mFlags = TEXTURE_CREATION_FLAG_ON_TILE;
    addRenderTarget(pRenderer, &depthRT, &pDepthBuffer);

    return pDepthBuffer != NULL;
}

bool MainApp::Init() {
    // FILE PATHS
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SHADER_SOURCES, "Shaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SHADER_BINARIES, "CompiledShaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_GPU_CONFIG, "GPUCfg");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_TEXTURES, "Textures");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_FONTS, "Fonts");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SCREENSHOTS, "Screenshots");

    if (!initInputSystem(pWindow))
        return false;

    // App Actions
    InputActionDesc actionDesc = {InputBindings::BUTTON_DUMP,
                                  [](InputActionContext *ctx) {
                                      dumpProfileData(((Renderer *)ctx->pUserData),
                                                      ((Renderer *)ctx->pUserData)->pName);
                                      return true;
                                  },
                                  pRenderer};
    addInputAction(&actionDesc);
    actionDesc = {InputBindings::BUTTON_FULLSCREEN,
                  [](InputActionContext *ctx) {
                      toggleFullscreen(((IApp *)ctx->pUserData)->pWindow);
                      return true;
                  },
                  this};
    addInputAction(&actionDesc);
    actionDesc = {InputBindings::BUTTON_EXIT, [](InputActionContext *ctx) {
                      requestShutdown();
                      return true;
                  }};
    addInputAction(&actionDesc);
    actionDesc = {InputBindings::BUTTON_ANY,
                  [](InputActionContext *ctx) {
                      bool capture = gAppUI.OnButton(ctx->mBinding, ctx->mBool, ctx->pPosition);
                      setEnableCaptureInput(capture && INPUT_ACTION_PHASE_CANCELED != ctx->mPhase);
                      return true;
                  },
                  this};
    addInputAction(&actionDesc);

    return true;
}
void MainApp::Exit() { exitInputSystem(); }
bool MainApp::Load() {
    if (mSettings.mResetGraphics || !pRenderer) {
        // window and renderer setup
        RendererDesc settings = {0};
        initRenderer(GetName(), &settings, &pRenderer);
        // check for init success
        if (!pRenderer)
            return false;

        QueueDesc queueDesc = {};
        queueDesc.mType = QUEUE_TYPE_GRAPHICS;
        queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
        addQueue(pRenderer, &queueDesc, &pGraphicsQueue);

        for (uint32_t i = 0; i < gImageCount; ++i) {
            CmdPoolDesc cmdPoolDesc = {};
            cmdPoolDesc.pQueue = pGraphicsQueue;
            addCmdPool(pRenderer, &cmdPoolDesc, &pCmdPools[i]);
            CmdDesc cmdDesc = {};
            cmdDesc.pPool = pCmdPools[i];
            addCmd(pRenderer, &cmdDesc, &pCmds[i]);

            addFence(pRenderer, &pRenderCompleteFences[i]);
            addSemaphore(pRenderer, &pRenderCompleteSemaphores[i]);
        }
        addSemaphore(pRenderer, &pImageAcquiredSemaphore);

        initResourceLoaderInterface(pRenderer);

        initScreenshotInterface(pRenderer, pGraphicsQueue);

#if defined(USE_BREADCRUMB)
        addShader(pRenderer, &crashShader, &pCrashShader);
#endif

        if (!gAppUI.Init(pRenderer))
            return false;

        gAppUI.LoadFont("TitilliumText/TitilliumText-Bold.otf");

        // Initialize micro profiler and it's UI.
        initProfiler();
        initProfilerUI(&gAppUI, mSettings.mWidth, mSettings.mHeight);

        // Gpu profiler can only be added after initProfile.
        gGpuProfileToken = addGpuProfiler(pRenderer, pGraphicsQueue, "Graphics");

        /************************************************************************/
        // GUI
        /************************************************************************/
        GuiDesc guiDesc = {};
        guiDesc.mStartPosition = vec2(mSettings.mWidth * 0.01f, mSettings.mHeight * 0.2f);
        pGuiWindow = gAppUI.AddGuiComponent(GetName(), &guiDesc);
#if !defined(TARGET_IOS)
        pGuiWindow->AddWidget(CheckboxWidget("Toggle VSync\t\t\t\t\t", &bToggleVSync));
#endif

        // Take a screenshot with a button.
        ButtonWidget screenshot("Screenshot");
        screenshot.pOnEdited = takeScreenshot;
        pGuiWindow->AddWidget(screenshot);

#if defined(USE_BREADCRUMB)
        ButtonWidget crashButton("Simulate crash");
        crashButton.pOnEdited = []() { bSimulateCrash = true; };
        pGuiWindow->AddWidget(crashButton);
#endif
        waitForAllResourceLoads();
    }

    if (!addSwapChain())
        return false;

    if (!addDepthBuffer())
        return false;

    if (!gAppUI.Load(pSwapChain->ppRenderTargets, 1))
        return false;

    return true;
}
void MainApp::Unload() {
    waitQueueIdle(pGraphicsQueue);

    gAppUI.Unload();

#if defined(USE_BREADCRUMB)
    removePipeline(pRenderer, pCrashPipeline);
#endif
    removeSwapChain(pRenderer, pSwapChain);
    removeRenderTarget(pRenderer, pDepthBuffer);

    if (mSettings.mResetGraphics || mSettings.mQuit) {
        exitProfilerUI();

        gAppUI.Exit();

        // Exit profile
        exitProfiler();

#if defined(USE_BREADCRUMB)
        removeShader(pRenderer, pCrashShader);
#endif

        for (uint32_t i = 0; i < gImageCount; ++i) {
            removeFence(pRenderer, pRenderCompleteFences[i]);
            removeSemaphore(pRenderer, pRenderCompleteSemaphores[i]);

            removeCmd(pRenderer, pCmds[i]);
            removeCmdPool(pRenderer, pCmdPools[i]);
        }
        removeSemaphore(pRenderer, pImageAcquiredSemaphore);

        exitResourceLoaderInterface(pRenderer);
        exitScreenshotInterface();
        removeQueue(pRenderer, pGraphicsQueue);
        removeRenderer(pRenderer);
    }
}

void MainApp::Update(float deltaTime) {
#if !defined(TARGET_IOS)
    if (pSwapChain->mEnableVsync != bToggleVSync) {
        waitQueueIdle(pGraphicsQueue);
        gFrameIndex = 0;
        ::toggleVSync(pRenderer, &pSwapChain);
    }
#endif

    updateInputSystem(mSettings.mWidth, mSettings.mHeight);

    /************************************************************************/
    // Scene Update
    /************************************************************************/
    static float currentTime = 0.0f;
    currentTime += deltaTime * 1000.0f;

    /************************************************************************/
    // Update GUI
    /************************************************************************/
    gAppUI.Update(deltaTime);
}
void MainApp::Draw() {
    uint32_t swapchainImageIndex;
    acquireNextImage(pRenderer, pSwapChain, pImageAcquiredSemaphore, NULL, &swapchainImageIndex);

    RenderTarget *pRenderTarget = pSwapChain->ppRenderTargets[swapchainImageIndex];
    Semaphore *pRenderCompleteSemaphore = pRenderCompleteSemaphores[gFrameIndex];
    Fence *pRenderCompleteFence = pRenderCompleteFences[gFrameIndex];

    // Stall if CPU is running "Swap Chain Buffer Count" frames ahead of GPU
    FenceStatus fenceStatus;
    getFenceStatus(pRenderer, pRenderCompleteFence, &fenceStatus);
    if (fenceStatus == FENCE_STATUS_INCOMPLETE)
        waitForFences(pRenderer, 1, &pRenderCompleteFence);

    // Reset cmd pool for this frame
    resetCmdPool(pRenderer, pCmdPools[gFrameIndex]);

    Cmd *cmd = pCmds[gFrameIndex];
    beginCmd(cmd);

    cmdBeginGpuFrameProfile(cmd, gGpuProfileToken);

    RenderTargetBarrier barriers[] = {
        {pRenderTarget, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET},
    };
    cmdResourceBarrier(cmd, 0, NULL, 0, NULL, 1, barriers);

    // simply record the screen cleaning command
    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
    loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
    loadActions.mClearDepth.depth = 0.0f;
    loadActions.mClearDepth.stencil = 0;
    cmdBindRenderTargets(cmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, NULL, NULL, -1, -1);
    cmdSetViewport(cmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(cmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    const uint32_t sphereVbStride = sizeof(float) * 6;
    const uint32_t skyboxVbStride = sizeof(float) * 4;

    // Using the malfunctioned pipeline
#if defined(USE_BREADCRUMB)
    if (bSimulateCrash) {
        gCrashedFrame = gFrameIndex;
        bSimulateCrash = false;
        bHasCrashed = true;
        pipeline = pCrashPipeline;
        LOGF(LogLevel::eERROR, "[Breadcrumb] Simulating a GPU crash situation...");
    }
#endif

#if defined(USE_BREADCRUMB)
    // Marker on top of the pip, won't wait for the following draw commands.
    cmdWriteMarker(cmd, MARKER_TYPE_IN, gValidMarkerValue, pMarkerBuffer[gFrameIndex], 0);
#endif

#if defined(USE_BREADCRUMB)
    // Marker on bottom of the pip, will wait for draw command to be executed.
    cmdWriteMarker(cmd, MARKER_TYPE_OUT, gValidMarkerValue, pMarkerBuffer[gFrameIndex], 1);

    // Handle readback warning
    if (pMarkerBuffer[gFrameIndex]->pCpuMappedAddress) {
        unmapBuffer(pRenderer, pMarkerBuffer[gFrameIndex]);
    }
#endif

    loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    cmdBindRenderTargets(cmd, 1, &pRenderTarget, NULL, &loadActions, NULL, NULL, -1, -1);
    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw UI");

    const float txtIndent = 8.f;
    float2 txtSizePx = cmdDrawCpuProfile(cmd, float2(txtIndent, 15.f), &gFrameTimeDraw);
    cmdDrawGpuProfile(cmd, float2(txtIndent, txtSizePx.y + 30.f), gGpuProfileToken, &gFrameTimeDraw);

    cmdDrawProfilerUI();

    gAppUI.Gui(pGuiWindow);
    gAppUI.Draw(cmd);
    cmdBindRenderTargets(cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    barriers[0] = {pRenderTarget, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT};
    cmdResourceBarrier(cmd, 0, NULL, 0, NULL, 1, barriers);

    cmdEndGpuFrameProfile(cmd, gGpuProfileToken);
    endCmd(cmd);

    QueueSubmitDesc submitDesc = {};
    submitDesc.mCmdCount = 1;
    submitDesc.mSignalSemaphoreCount = 1;
    submitDesc.mWaitSemaphoreCount = 1;
    submitDesc.ppCmds = &cmd;
    submitDesc.ppSignalSemaphores = &pRenderCompleteSemaphore;
    submitDesc.ppWaitSemaphores = &pImageAcquiredSemaphore;
    submitDesc.pSignalFence = pRenderCompleteFence;
    queueSubmit(pGraphicsQueue, &submitDesc);
    QueuePresentDesc presentDesc = {};
    presentDesc.mIndex = swapchainImageIndex;
    presentDesc.mWaitSemaphoreCount = 1;
    presentDesc.pSwapChain = pSwapChain;
    presentDesc.ppWaitSemaphores = &pRenderCompleteSemaphore;
    presentDesc.mSubmitDone = true;

    // captureScreenshot() must be used before presentation.
    if (gTakeScreenshot) {
        // Metal platforms need one renderpass to prepare the swapchain textures for copy.
        if (prepareScreenshot(pSwapChain)) {
            captureScreenshot(pSwapChain, swapchainImageIndex, RESOURCE_STATE_PRESENT,
                              "01_Transformations_Screenshot.png");
            gTakeScreenshot = false;
        }
    }

    PresentStatus presentStatus = queuePresent(pGraphicsQueue, &presentDesc);
    flipProfiler();

    if (presentStatus == PRESENT_STATUS_DEVICE_RESET) {
        Thread::Sleep(5000); // Wait for a few seconds to allow the driver to come back online before doing a reset.
        mSettings.mResetGraphics = true;
    }

    gFrameIndex = (gFrameIndex + 1) % gImageCount;
}