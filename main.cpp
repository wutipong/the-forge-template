#include "main.h"

#include <cstdlib>
#include <IGraphics.h>
#include "scene_manager.h"
#include "demo_scene.h"

extern RendererApi gSelectedRendererApi;

Renderer *pRenderer = nullptr;
SwapChain *pSwapChain = nullptr;
RenderTarget *pDepthBuffer = nullptr;

bool MainApp::Init()
{
    srand(time(NULL));

    // FILE PATHS
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SHADER_SOURCES, "Shaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SHADER_BINARIES, "CompiledShaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_GPU_CONFIG, "GPUCfg");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_TEXTURES, "Textures");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_FONTS, "Fonts");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SCREENSHOTS, "Screenshots");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SCRIPTS, "Scripts");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_OTHER_FILES, "Noesis");

    // window and renderer setup
    RendererDesc settings{};
    settings.mD3D11Supported = false;
    settings.mGLESSupported = false;
    settings.mEnableGPUBasedValidation = true;

    initRenderer(GetName(), &settings, &pRenderer);
    // check for init success
    if (!pRenderer)
        return false;

    QueueDesc queueDesc = {};
    queueDesc.mType = QUEUE_TYPE_GRAPHICS;
    queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
    addQueue(pRenderer, &queueDesc, &pGraphicsQueue);

    for (uint32_t i = 0; i < gImageCount; ++i)
    {
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

    // Initialize micro profiler and its UI.
    ProfilerDesc profiler = {};
    profiler.pRenderer = pRenderer;
    profiler.mWidthUI = mSettings.mWidth;
    profiler.mHeightUI = mSettings.mHeight;
    initProfiler(&profiler);

    // Gpu profiler can only be added after initProfile.
    gGpuProfileToken = addGpuProfiler(pRenderer, pGraphicsQueue, "Graphics");

    // Load fonts
    FontDesc font = {};
    font.pFontPath = "TitilliumText/TitilliumText-Bold.otf";
    fntDefineFonts(&font, 1, &gFontID);

    FontSystemDesc fontRenderDesc = {};
    fontRenderDesc.pRenderer = pRenderer;
    if (!initFontSystem(&fontRenderDesc))
    {
        return false; // report?
    }

    UserInterfaceDesc uiRenderDesc = {};
    uiRenderDesc.pRenderer = pRenderer;
    initUserInterface(&uiRenderDesc);

    UIComponentDesc guiDesc = {};
    guiDesc.mStartPosition = vec2(mSettings.mWidth * 0.01f, mSettings.mHeight * 0.2f);
    uiCreateComponent(GetName(), &guiDesc, &pGuiWindow);

    // Take a screenshot with a button.
    ButtonWidget screenshot;
    UIWidget *pScreenshot = uiCreateComponentWidget(pGuiWindow, "Screenshot", &screenshot, WIDGET_TYPE_BUTTON);

    waitForAllResourceLoads();

    InputSystemDesc inputDesc = {};
    inputDesc.pRenderer = pRenderer;
    inputDesc.pWindow = pWindow;
    if (!initInputSystem(&inputDesc))
        return false;

    InputActionCallback onAnyInput = [](InputActionContext *ctx)
    {
        if (ctx->mActionId > UISystemInputActions::UI_ACTION_START_ID_)
        {
            uiOnInput(ctx->mActionId, ctx->mBool, ctx->pPosition, &ctx->mFloat2);
        }
        return true;
    };
    GlobalInputActionDesc globalInputActionDesc = {GlobalInputActionDesc::ANY_BUTTON_ACTION, onAnyInput, this};
    setGlobalInputAction(&globalInputActionDesc);

    SetStartScene<DemoScene>();
    InitCurrentScene(gImageCount);

    return true;
}

void MainApp::Exit()
{
    ExitCurrentScene();
    exitInputSystem();
    exitUserInterface();
    exitFontSystem();

    exitProfiler();

    for (uint32_t i = 0; i < gImageCount; ++i)
    {
        removeFence(pRenderer, pRenderCompleteFences[i]);
        removeSemaphore(pRenderer, pRenderCompleteSemaphores[i]);

        removeCmd(pRenderer, pCmds[i]);
        removeCmdPool(pRenderer, pCmdPools[i]);
    }

    removeSemaphore(pRenderer, pImageAcquiredSemaphore);

    exitResourceLoaderInterface(pRenderer);
    removeQueue(pRenderer, pGraphicsQueue);
    exitRenderer(pRenderer);
    pRenderer = nullptr;
}

bool MainApp::Load(ReloadDesc *pReloadDesc)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        SwapChainDesc swapChainDesc = {};
        swapChainDesc.mWindowHandle = pWindow->handle;
        swapChainDesc.mPresentQueueCount = 1;
        swapChainDesc.ppPresentQueues = &pGraphicsQueue;
        swapChainDesc.mWidth = mSettings.mWidth;
        swapChainDesc.mHeight = mSettings.mHeight;
        swapChainDesc.mImageCount = gImageCount;
        swapChainDesc.mColorFormat = getRecommendedSwapchainFormat(true, true);
        swapChainDesc.mEnableVsync = mSettings.mVSyncEnabled;
        swapChainDesc.mFlags = SWAP_CHAIN_CREATION_FLAG_ENABLE_FOVEATED_RENDERING_VR;
        addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);

        if (pSwapChain == nullptr)
            return false;

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
        depthRT.mFlags = TEXTURE_CREATION_FLAG_ON_TILE | TEXTURE_CREATION_FLAG_VR_MULTIVIEW;
        addRenderTarget(pRenderer, &depthRT, &pDepthBuffer);
        if (pDepthBuffer == nullptr)
            return false;
    }

    UserInterfaceLoadDesc uiLoad = {};
    uiLoad.mColorFormat = pSwapChain->ppRenderTargets[0]->mFormat;
    uiLoad.mHeight = mSettings.mHeight;
    uiLoad.mWidth = mSettings.mWidth;
    uiLoad.mLoadType = pReloadDesc->mType;
    loadUserInterface(&uiLoad);

    FontSystemLoadDesc fontLoad = {};
    fontLoad.mColorFormat = pSwapChain->ppRenderTargets[0]->mFormat;
    fontLoad.mHeight = mSettings.mHeight;
    fontLoad.mWidth = mSettings.mWidth;
    fontLoad.mLoadType = pReloadDesc->mType;
    loadFontSystem(&fontLoad);

    LoadCurrentScene(pReloadDesc, pRenderer, pSwapChain->ppRenderTargets[0], pDepthBuffer, gImageCount);
    return true;
}

void MainApp::Unload(ReloadDesc *pReloadDesc)
{
    waitQueueIdle(pGraphicsQueue);

    UnloadCurrentScene(pReloadDesc, pRenderer);
    unloadFontSystem(pReloadDesc->mType);
    unloadUserInterface(pReloadDesc->mType);
    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        removeSwapChain(pRenderer, pSwapChain);
        removeRenderTarget(pRenderer, pDepthBuffer);
    }
}

void MainApp::Update(float deltaTime)
{
    updateInputSystem(deltaTime, mSettings.mWidth, mSettings.mHeight);

    UpdateCurrentScene(deltaTime, mSettings.mWidth, mSettings.mHeight);
}

void MainApp::Draw()
{
    uint32_t swapchainImageIndex;
    acquireNextImage(pRenderer, pSwapChain, pImageAcquiredSemaphore, nullptr, &swapchainImageIndex);

    RenderTarget *pRenderTarget = pSwapChain->ppRenderTargets[swapchainImageIndex];
    Semaphore *pRenderCompleteSemaphore = pRenderCompleteSemaphores[gFrameIndex];
    Fence *pRenderCompleteFence = pRenderCompleteFences[gFrameIndex];

    // Stall if CPU is running "Swap Chain Buffer Count" frames ahead of GPU
    FenceStatus fenceStatus;
    getFenceStatus(pRenderer, pRenderCompleteFence, &fenceStatus);
    if (fenceStatus == FENCE_STATUS_INCOMPLETE)
        waitForFences(pRenderer, 1, &pRenderCompleteFence);

    PreDrawCurrentScene(gFrameIndex);

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
    loadActions.mClearDepth.depth = 0.0f;
    cmdBindRenderTargets(cmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(cmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(cmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw Scene");
    DrawCurrentScene(cmd, pRenderTarget, pDepthBuffer, gFrameIndex);
    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    cmdBindRenderTargets(cmd, 1, &pRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);

    loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    cmdBindRenderTargets(cmd, 1, &pRenderTarget, nullptr, &loadActions, NULL, NULL, -1, -1);
    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw UI");

    gFrameTimeDraw.mFontColor = 0xff00ffff;
    gFrameTimeDraw.mFontSize = 18.0f;
    gFrameTimeDraw.mFontID = gFontID;
    float2 txtSizePx = cmdDrawCpuProfile(cmd, float2(8.f, 15.f), &gFrameTimeDraw);
    cmdDrawGpuProfile(cmd, float2(8.f, txtSizePx.y + 75.f), gGpuProfileToken, &gFrameTimeDraw);

    cmdDrawUserInterface(cmd);

    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    cmdBindRenderTargets(cmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
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

    queuePresent(pGraphicsQueue, &presentDesc);
    flipProfiler();

    gFrameIndex = (gFrameIndex + 1) % gImageCount;
}

const char *MainApp::GetName() { return "The Forge Template"; }

DEFINE_APPLICATION_MAIN(MainApp);