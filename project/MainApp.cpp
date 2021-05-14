#include "MainApp.h"

#include <array>

#include "TestScene.h"

DEFINE_APPLICATION_MAIN(MainApp)

MainApp *MainApp::pApp;

auto MainApp::AddSwapChain() -> bool {
    SwapChainDesc swapChainDesc = {};
    swapChainDesc.mWindowHandle = pWindow->handle;
    swapChainDesc.mPresentQueueCount = 1;
    swapChainDesc.ppPresentQueues = &pGraphicsQueue;
    swapChainDesc.mWidth = mSettings.mWidth;
    swapChainDesc.mHeight = mSettings.mHeight;
    swapChainDesc.mImageCount = ImageCount;
    swapChainDesc.mColorFormat = getRecommendedSwapchainFormat(true);
    swapChainDesc.mEnableVsync = mSettings.mDefaultVSyncEnabled;
    ::addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);

    return pSwapChain != nullptr;
}

auto MainApp::AddDepthBuffer() -> bool {
    // Add depth buffer
    RenderTargetDesc depthRT = {};
    depthRT.mArraySize = 1;
    depthRT.mClearValue.depth = 0.0F;
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

    return pDepthBuffer != nullptr;
}

auto MainApp::Init() -> bool {
    pApp = this;
    currentScene = std::make_unique<TestScene>();

    // FILE PATHS
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SHADER_SOURCES, "Shaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SHADER_BINARIES, "CompiledShaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_GPU_CONFIG, "GPUCfg");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_TEXTURES, "Textures");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_MESHES, "Meshes");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_FONTS, "Fonts");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SCREENSHOTS, "Screenshots");

    if (!initInputSystem(pWindow)) {
        return false;
    }

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
                      bool capture = MainApp::Instance()->appUI.OnButton(ctx->mBinding, ctx->mBool, ctx->pPosition);
                      setEnableCaptureInput(capture && INPUT_ACTION_PHASE_CANCELED != ctx->mPhase);
                      return true;
                  },
                  this};
    addInputAction(&actionDesc);

    if (auto mod = GetModuleHandleA("renderdoc.dll")) {
        auto RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
    }

    return true;
}
void MainApp::Exit() { exitInputSystem(); }
auto MainApp::Load() -> bool {
    if (mSettings.mResetGraphics || (pRenderer == nullptr)) {
        // window and renderer setup
        RendererDesc settings = {nullptr};
        initRenderer(GetName(), &settings, &pRenderer);
        // check for init success
        if (pRenderer == nullptr) {
            return false;
        }

        QueueDesc queueDesc = {};
        queueDesc.mType = QUEUE_TYPE_GRAPHICS;
        queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
        addQueue(pRenderer, &queueDesc, &pGraphicsQueue);

        for (uint32_t i = 0; i < ImageCount; ++i) {
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

        if (!appUI.Init(pRenderer)) {
            return false;
        }

        appUI.LoadFont("TitilliumText/TitilliumText-Bold.otf");

        // Initialize micro profiler and it's UI.
        initProfiler();
        initProfilerUI(&appUI, mSettings.mWidth, mSettings.mHeight);

        // Gpu profiler can only be added after initProfile.
        gGpuProfileToken = addGpuProfiler(pRenderer, pGraphicsQueue, "Graphics");

        /************************************************************************/
        // GUI
        /************************************************************************/
        GuiDesc guiDesc = {};
        guiDesc.mStartPosition = vec2(mSettings.mWidth * 0.01F, mSettings.mHeight * 0.2F);
        pGuiWindow = appUI.AddGuiComponent(GetName(), &guiDesc);
#if !defined(TARGET_IOS)
        pGuiWindow->AddWidget(CheckboxWidget("Toggle VSync\t\t\t\t\t", &bToggleVSync));
#endif

        // Take a screenshot with a button.
        ButtonWidget screenshot("Screenshot");
        screenshot.pOnEdited = [] { MainApp::Instance()->TakeScreenshot(); };
        pGuiWindow->AddWidget(screenshot);

        if (rdoc_api != nullptr) {
            ButtonWidget captureBtn("Capture Frame");
            captureBtn.pOnEdited = [] { MainApp::Instance()->Capture(); };
            pGuiWindow->AddWidget(captureBtn);
        }

        waitForAllResourceLoads();
    }

    if (!AddSwapChain()) {
        return false;
    }

    if (!AddDepthBuffer()) {
        return false;
    }

    if (!appUI.Load(pSwapChain->ppRenderTargets, 1)) {
        return false;
    }

    waitForAllResourceLoads();

    currentScene->Load(pRenderer, pSwapChain);

    return true;
}
void MainApp::Unload() {

    waitQueueIdle(pGraphicsQueue);

    currentScene->Unload(pRenderer);

    appUI.Unload();

#if defined(USE_BREADCRUMB)
    removePipeline(pRenderer, pCrashPipeline);
#endif
    removeSwapChain(pRenderer, pSwapChain);
    removeRenderTarget(pRenderer, pDepthBuffer);

    if (mSettings.mResetGraphics || mSettings.mQuit) {
        exitProfilerUI();

        appUI.Exit();

        // Exit profile
        exitProfiler();

        for (uint32_t i = 0; i < ImageCount; ++i) {
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
    if (pSwapChain->mEnableVsync != static_cast<int>(bToggleVSync)) {
        waitQueueIdle(pGraphicsQueue);
        gFrameIndex = 0;
        ::toggleVSync(pRenderer, &pSwapChain);
    }
#endif

    updateInputSystem(mSettings.mWidth, mSettings.mHeight);

    /************************************************************************/
    // Scene Update
    /************************************************************************/
    static float currentTime = 0.0F;
    currentTime += deltaTime * 1000.0F;

    /************************************************************************/
    // Update GUI
    /************************************************************************/
    appUI.Update(deltaTime);

    currentScene->Update(deltaTime);
}

void MainApp::Draw() {
    if (isCapturing) {
        rdoc_api->StartFrameCapture(nullptr, nullptr);
    }
    uint32_t swapchainImageIndex;
    acquireNextImage(pRenderer, pSwapChain, pImageAcquiredSemaphore, nullptr, &swapchainImageIndex);

    RenderTarget *pRenderTarget = pSwapChain->ppRenderTargets[swapchainImageIndex];
    Semaphore *pRenderCompleteSemaphore = pRenderCompleteSemaphores[gFrameIndex];
    Fence *pRenderCompleteFence = pRenderCompleteFences[gFrameIndex];

    // Stall if CPU is running "Swap Chain Buffer Count" frames ahead of GPU
    FenceStatus fenceStatus;
    getFenceStatus(pRenderer, pRenderCompleteFence, &fenceStatus);
    if (fenceStatus == FENCE_STATUS_INCOMPLETE) {
        waitForFences(pRenderer, 1, &pRenderCompleteFence);
    }

    // Reset cmd pool for this frame
    resetCmdPool(pRenderer, pCmdPools[gFrameIndex]);

    Cmd *cmd = pCmds[gFrameIndex];
    beginCmd(cmd);

    cmdBeginGpuFrameProfile(cmd, gGpuProfileToken);

    RenderTargetBarrier barriers[] = {
        {pRenderTarget, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET},
    };
    cmdResourceBarrier(cmd, 0, nullptr, 0, nullptr, 1, barriers);

    // simply record the screen cleaning command
    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
    loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
    loadActions.mClearDepth.depth = 0.0F;
    loadActions.mClearDepth.stencil = 0;
    cmdBindRenderTargets(cmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(cmd, 0.0F, 0.0F, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0F, 1.0F);
    cmdSetScissor(cmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdSetViewport(cmd, 0.0F, 0.0F, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0F, 1.0F);

    loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    cmdBindRenderTargets(cmd, 1, &pRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);
    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw UI");

    const float txtIndent = 8.F;
    float2 txtSizePx = cmdDrawCpuProfile(cmd, float2(txtIndent, 15.F), &gFrameTimeDraw);
    cmdDrawGpuProfile(cmd, float2(txtIndent, txtSizePx.y + 30.F), gGpuProfileToken, &gFrameTimeDraw);

    cmdDrawProfilerUI();

    appUI.Gui(pGuiWindow);
    appUI.Draw(cmd);
    cmdBindRenderTargets(cmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw Scene");
    currentScene->Draw(cmd, gFrameIndex);
    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    barriers[0] = {pRenderTarget, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT};
    cmdResourceBarrier(cmd, 0, nullptr, 0, nullptr, 1, barriers);

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
    if (isTakingScreenshot) {
        // Metal platforms need one renderpass to prepare the swapchain textures for copy.
        if (prepareScreenshot(pSwapChain)) {
            captureScreenshot(pSwapChain, swapchainImageIndex, RESOURCE_STATE_PRESENT, "Screenshot.png");
            isTakingScreenshot = false;
        }
    }

    PresentStatus presentStatus = queuePresent(pGraphicsQueue, &presentDesc);
    flipProfiler();

    if (presentStatus == PRESENT_STATUS_DEVICE_RESET) {
        Thread::Sleep(5000); // Wait for a few seconds to allow the driver to come back online before doing a reset.
        mSettings.mResetGraphics = true;
    }

    gFrameIndex = (gFrameIndex + 1) % ImageCount;
    if (isCapturing) {
        rdoc_api->EndFrameCapture(nullptr, nullptr);
        isCapturing = false;
    }
}
