#include "MainApp.h"

#include "TestScene.h"
#include "Scene.h"
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

namespace {
IApp *pAppInstance;

ProfileToken gGpuProfileToken = PROFILE_INVALID_TOKEN;

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

UIComponent *pGuiWindow{nullptr};
FontDrawDesc gFrameTimeDraw;
uint32_t gFontID;

RENDERDOC_API_1_1_2 *rdoc_api = nullptr;

bool bToggleVSync = false;
bool bIsCapturing = false;
bool bIsTakingScreenshot = false;

Scene currentScene;

} // namespace

auto AppInstance() -> IApp * { return pAppInstance; }

static auto AddSwapChain() -> bool {
    auto &&mSettings = AppInstance()->mSettings;
    auto &&pWindow = AppInstance()->pWindow;

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

static auto AddDepthBuffer() -> bool {
    auto &&mSettings = AppInstance()->mSettings;
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

auto Init(IApp *app) -> bool {
    ::pAppInstance = app;
    auto &&mSettings = AppInstance()->mSettings;
    auto &&pWindow = AppInstance()->pWindow;

    // currentScene = NopScene::Create();
    currentScene = TestScene::Create();

    // FILE PATHS
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SHADER_SOURCES, "Shaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SHADER_BINARIES, "CompiledShaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_GPU_CONFIG, "GPUCfg");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_TEXTURES, "Textures");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_MESHES, "Meshes");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_FONTS, "Fonts");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SCREENSHOTS, "Screenshots");

    // window and renderer setup
    RendererDesc settings = {};
    initRenderer(AppInstance()->GetName(), &settings, &pRenderer);
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

    // Load fonts
    FontDesc font{};
    font.pFontPath = "TitilliumText/TitilliumText-Bold.otf";
    fntDefineFonts(&font, 1, &gFontID);

    FontSystemDesc fontRenderDesc{};
    fontRenderDesc.pRenderer = pRenderer;
    if (!initFontSystem(&fontRenderDesc))
        return false; // report?

    // Initialize Forge User Interface Rendering
    UserInterfaceDesc uiRenderDesc{};
    uiRenderDesc.pRenderer = pRenderer;
    initUserInterface(&uiRenderDesc);

    // Initialize micro profiler and its UI.
    ProfilerDesc profiler{};
    profiler.pRenderer = pRenderer;
    profiler.mWidthUI = mSettings.mWidth;
    profiler.mHeightUI = mSettings.mHeight;
    initProfiler(&profiler);

    // Gpu profiler can only be added after initProfile.
    gGpuProfileToken = addGpuProfiler(pRenderer, pGraphicsQueue, "Graphics");

    /************************************************************************/
    // GUI
    /************************************************************************/
    UIComponentDesc guiDesc{};
    guiDesc.mStartPosition = vec2(mSettings.mWidth * 0.01f, mSettings.mHeight * 0.2f);
    uiCreateComponent(AppInstance()->GetName(), &guiDesc, &pGuiWindow);

#if !defined(TARGET_IOS)
    CheckboxWidget cbVsync;
    cbVsync.pData = &bToggleVSync;
    uiCreateComponentWidget(pGuiWindow, "Toggle VSync", &cbVsync, WIDGET_TYPE_CHECKBOX);
#endif

    if (rdoc_api != nullptr) {

        // Take a screenshot with a button.
        ButtonWidget button;
        UIWidget *pScreenshot = uiCreateComponentWidget(pGuiWindow, "Capture Frame", &button, WIDGET_TYPE_BUTTON);
        uiSetWidgetOnEditedCallback(pScreenshot, [] { bIsCapturing = true; });
    }

    // Take a screenshot with a button.
    ButtonWidget screenshot;
    UIWidget *pScreenshot = uiCreateComponentWidget(pGuiWindow, "Screenshot", &screenshot, WIDGET_TYPE_BUTTON);
    uiSetWidgetOnEditedCallback(pScreenshot, [] { bIsTakingScreenshot = true; });

    InputSystemDesc inputDesc{};
    inputDesc.pRenderer = pRenderer;
    inputDesc.pWindow = pWindow;
    if (!initInputSystem(&inputDesc)) {
        return false;
    }

    {
        InputActionDesc actionDesc{InputBindings::BUTTON_DUMP, [](InputActionContext *ctx) {
                                       dumpProfileData(pRenderer->pName);
                                       return true;
                                   }};
        addInputAction(&actionDesc);
    }
    {

        InputActionDesc actionDesc{InputBindings::BUTTON_FULLSCREEN, [](InputActionContext *ctx) {
                                       toggleFullscreen(AppInstance()->pWindow);
                                       return true;
                                   }};
        addInputAction(&actionDesc);
    }
    {

        InputActionDesc actionDesc{InputBindings::BUTTON_EXIT, [](InputActionContext *ctx) {
                                       requestShutdown();
                                       return true;
                                   }};
        addInputAction(&actionDesc);
    }
    {
        InputActionDesc actionDesc{InputBindings::BUTTON_ANY, [](InputActionContext *ctx) {
                                       bool capture = uiOnButton(ctx->mBinding, ctx->mBool, ctx->pPosition);
                                       setEnableCaptureInput(capture && INPUT_ACTION_PHASE_CANCELED != ctx->mPhase);
                                       return true;
                                   }};
        addInputAction(&actionDesc);
    }

    if (auto mod = GetModuleHandleA("renderdoc.dll")) {
        auto RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
    }

    currentScene.Init(pRenderer);

    return true;
}

void Exit() {
    exitInputSystem();
    exitUserInterface();
    exitFontSystem();
    exitProfiler();

    currentScene.Exit(pRenderer);

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

    exitRenderer(pRenderer);
    pRenderer = nullptr;
}

auto Load() -> bool {

    if (!AddSwapChain()) {
        return false;
    }

    if (!AddDepthBuffer()) {
        return false;
    }

    RenderTarget *ppPipelineRenderTargets[]{pSwapChain->ppRenderTargets[0], pDepthBuffer};
    if (!addFontSystemPipelines(ppPipelineRenderTargets, 2, nullptr))
        return false;

    if (!addUserInterfacePipelines(ppPipelineRenderTargets[0])) {
        return false;
    }

    waitForAllResourceLoads();

    currentScene.Load(pRenderer, pSwapChain);

    waitForAllResourceLoads();

    return true;
}
void Unload() {

    waitQueueIdle(pGraphicsQueue);

    currentScene.Unload(pRenderer);

    removeUserInterfacePipelines();
    removeFontSystemPipelines();

    removeSwapChain(pRenderer, pSwapChain);
    removeRenderTarget(pRenderer, pDepthBuffer);
}

void Update(float deltaTime) {
    auto &&mSettings = AppInstance()->mSettings;

#if !defined(TARGET_IOS)
    if (pSwapChain->mEnableVsync != static_cast<int>(bToggleVSync)) {
        waitQueueIdle(pGraphicsQueue);
        gFrameIndex = 0;
        ::toggleVSync(pRenderer, &pSwapChain);
    }
#endif

    updateInputSystem(mSettings.mWidth, mSettings.mHeight);

    currentScene.Update(deltaTime);
}

void Draw() {
    if (bIsCapturing) {
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

    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw Scene");
    { currentScene.Draw(cmd, gFrameIndex); }
    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;

    cmdBindRenderTargets(cmd, 1, &pRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);
    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw UI");
    {
        gFrameTimeDraw.mFontColor = 0xff00ffff;
        gFrameTimeDraw.mFontSize = 18.0f;
        gFrameTimeDraw.mFontID = gFontID;

        const float txtIndent = 8.F;
        float2 txtSizePx = cmdDrawCpuProfile(cmd, float2(txtIndent, 15.F), &gFrameTimeDraw);
        cmdDrawGpuProfile(cmd, float2(txtIndent, txtSizePx.y + 30.F), gGpuProfileToken, &gFrameTimeDraw);

        cmdDrawGpuProfile(cmd, float2(8.f, txtSizePx.y + 30.f), gGpuProfileToken, &gFrameTimeDraw);

        cmdDrawUserInterface(cmd);

        cmdBindRenderTargets(cmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
    }
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
    if (bIsTakingScreenshot) {
        // Metal platforms need one renderpass to prepare the swapchain textures for copy.
        if (prepareScreenshot(pSwapChain)) {
            captureScreenshot(pSwapChain, swapchainImageIndex, RESOURCE_STATE_PRESENT, "Screenshot.png");
            bIsTakingScreenshot = false;
        }
    }

    queuePresent(pGraphicsQueue, &presentDesc);
    flipProfiler();

    gFrameIndex = (gFrameIndex + 1) % ImageCount;
    if (bIsCapturing) {
        rdoc_api->EndFrameCapture(nullptr, nullptr);
        bIsCapturing = false;
    }
}
