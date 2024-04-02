#include "MainApp.h"

#include <IFileSystem.h>
#include <IFont.h>
#include <IGraphics.h>
#include <IInput.h>
#include <IProfiler.h>
#include <IResourceLoader.h>
#include <IScreenshot.h>
#include <IUI.h>
#include <RingBuffer.h>
#include <cstdlib>
#include <string>
#include "DemoScene.h"
#include "Settings.h"

namespace Scene = DemoScene;

namespace
{
    Renderer *pRenderer = nullptr;
    SwapChain *pSwapChain = nullptr;

    UIComponent *pGuiWindow = nullptr;
    Queue *pGraphicsQueue = nullptr;
    uint32_t gFontID = 0;
    GpuCmdRing gGraphicsCmdRing = {};

    Semaphore *pImageAcquiredSemaphore = nullptr;

    ProfileToken gGpuProfileToken = PROFILE_INVALID_TOKEN;
} // namespace

const char *MainApp::GetName() { return "The Forge Template"; }

bool MainApp::Init()
{
    srand(time(NULL));
    extern PlatformParameters gPlatformParameters;

    for (int i = 0; i < IApp::argc; i++)
    {
        std::string arg(IApp::argv[i]);
        if (arg == "--vulkan")
        {
            gPlatformParameters.mSelectedRendererApi = RendererApi::RENDERER_API_VULKAN;
        }

        if (arg == "--directx12")
        {
            gPlatformParameters.mSelectedRendererApi = RendererApi::RENDERER_API_D3D12;
        }
    }

    // FILE PATHS
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SHADER_BINARIES, "CompiledShaders");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_GPU_CONFIG, "GPUCfg");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_TEXTURES, "Textures");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_FONTS, "Fonts");
    fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SCREENSHOTS, "Screenshots");
    fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SCRIPTS, "Scripts");

    // window and renderer setup
    RendererDesc settings{
        .mD3D11Supported = false,
        .mGLESSupported = false,
    };

    initRenderer(GetName(), &settings, &pRenderer);
    if (!pRenderer)
    {
        return false;
    }

    QueueDesc queueDesc = {
        .mType = QUEUE_TYPE_GRAPHICS,
        .mFlag = QUEUE_FLAG_INIT_MICROPROFILE,
    };
    addQueue(pRenderer, &queueDesc, &pGraphicsQueue);

    GpuCmdRingDesc cmdRingDesc = {
        .pQueue = pGraphicsQueue,
        .mPoolCount = gDataBufferCount,
        .mCmdPerPoolCount = 1,
        .mAddSyncPrimitives = true,
    };
    addGpuCmdRing(pRenderer, &cmdRingDesc, &gGraphicsCmdRing);

    addSemaphore(pRenderer, &pImageAcquiredSemaphore);

    initResourceLoaderInterface(pRenderer);

    // Initialize micro profiler and its UI.
    ProfilerDesc profiler = {
        .pRenderer = pRenderer,
        .mWidthUI = static_cast<uint32_t>(mSettings.mWidth),
        .mHeightUI = static_cast<uint32_t>(mSettings.mHeight),
    };
    initProfiler(&profiler);

    // Gpu profiler can only be added after initProfile.
    gGpuProfileToken = addGpuProfiler(pRenderer, pGraphicsQueue, "Graphics");

    // Load fonts
    FontDesc font = {
        .pFontPath = "TitilliumText/TitilliumText-Bold.otf",
    };
    fntDefineFonts(&font, 1, &gFontID);

    FontSystemDesc fontRenderDesc = {
        .pRenderer = pRenderer,
    };
    if (!initFontSystem(&fontRenderDesc))
    {
        return false;
    }

    UserInterfaceDesc uiRenderDesc = {
        .pRenderer = pRenderer,
    };
    initUserInterface(&uiRenderDesc);

    UIComponentDesc guiDesc = {
        .mStartPosition = vec2(mSettings.mWidth * 0.01f, mSettings.mHeight * 0.2f),
    };
    uiCreateComponent(GetName(), &guiDesc, &pGuiWindow);

    // Take a screenshot with a button.
    ButtonWidget screenshot{};
    UIWidget *pScreenshot = uiCreateComponentWidget(pGuiWindow, "Screenshot", &screenshot, WIDGET_TYPE_BUTTON);

    waitForAllResourceLoads();

    InputSystemDesc inputDesc{
        .pRenderer = pRenderer,
        .pWindow = pWindow,
    };
    if (!initInputSystem(&inputDesc))
    {
        return false;
    }

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

    if (!Scene::Init())
    {
        return false;
    };

    return true;
}

void MainApp::Exit()
{
    Scene::Exit();
    exitInputSystem();
    exitUserInterface();
    exitFontSystem();

    exitProfiler();

    removeSemaphore(pRenderer, pImageAcquiredSemaphore);
    removeGpuCmdRing(pRenderer, &gGraphicsCmdRing);

    exitResourceLoaderInterface(pRenderer);
    removeQueue(pRenderer, pGraphicsQueue);
    exitRenderer(pRenderer);
    pRenderer = nullptr;
}

bool MainApp::Load(ReloadDesc *pReloadDesc)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        SwapChainDesc swapChainDesc = {
            .mWindowHandle = pWindow->handle,
            .ppPresentQueues = &pGraphicsQueue,
            .mPresentQueueCount = 1,
            .mImageCount = getRecommendedSwapchainImageCount(pRenderer, &pWindow->handle),
            .mWidth = static_cast<uint32_t>(mSettings.mWidth),
            .mHeight = static_cast<uint32_t>(mSettings.mHeight),
            .mColorFormat = getSupportedSwapchainFormat(pRenderer, &swapChainDesc, COLOR_SPACE_SDR_SRGB),
            .mFlags = SWAP_CHAIN_CREATION_FLAG_ENABLE_FOVEATED_RENDERING_VR,
            .mEnableVsync = mSettings.mVSyncEnabled,
            .mColorSpace = COLOR_SPACE_SDR_SRGB,
        };
        addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);

        if (pSwapChain == nullptr)
        {
            return false;
        }
    }

    FontSystemLoadDesc fontLoad = {
        .mLoadType = pReloadDesc->mType,
        .mColorFormat = static_cast<uint32_t>(pSwapChain->ppRenderTargets[0]->mFormat),
        .mWidth = static_cast<uint32_t>(mSettings.mWidth),
        .mHeight = static_cast<uint32_t>(mSettings.mHeight),
    };
    loadFontSystem(&fontLoad);

    UserInterfaceLoadDesc uiLoad = {
        .mLoadType = static_cast<uint32_t>(pReloadDesc->mType),
        .mColorFormat = static_cast<uint32_t>(pSwapChain->ppRenderTargets[0]->mFormat),
        .mWidth = static_cast<uint32_t>(mSettings.mWidth),
        .mHeight = static_cast<uint32_t>(mSettings.mHeight),
    };
    loadUserInterface(&uiLoad);

    initScreenshotInterface(pRenderer, pGraphicsQueue);

    if (!Scene::Load(pReloadDesc, pRenderer, pSwapChain->ppRenderTargets[0]))
    {
        return false;
    };

    waitForAllResourceLoads();

    return true;
}

void MainApp::Unload(ReloadDesc *pReloadDesc)
{
    waitQueueIdle(pGraphicsQueue);

    Scene::Unload(pReloadDesc, pRenderer);

    unloadFontSystem(pReloadDesc->mType);
    unloadUserInterface(pReloadDesc->mType);

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        removeSwapChain(pRenderer, pSwapChain);
    }

    exitScreenshotInterface();
}

void MainApp::Update(float deltaTime)
{
    updateInputSystem(deltaTime, mSettings.mWidth, mSettings.mHeight);
    Scene::Update(deltaTime, mSettings.mWidth, mSettings.mHeight);
}

void MainApp::Draw()
{
    if (pSwapChain->mEnableVsync != mSettings.mVSyncEnabled)
    {
        waitQueueIdle(pGraphicsQueue);
        ::toggleVSync(pRenderer, &pSwapChain);
    }

    uint32_t swapchainImageIndex;
    acquireNextImage(pRenderer, pSwapChain, pImageAcquiredSemaphore, nullptr, &swapchainImageIndex);

    RenderTarget *pRenderTarget = pSwapChain->ppRenderTargets[swapchainImageIndex];
    GpuCmdRingElement elem = getNextGpuCmdRingElement(&gGraphicsCmdRing, true, 1);

    // Stall if CPU is running "Swap Chain Buffer Count" frames ahead of GPU
    FenceStatus fenceStatus;
    getFenceStatus(pRenderer, elem.pFence, &fenceStatus);
    if (fenceStatus == FENCE_STATUS_INCOMPLETE)
    {
        waitForFences(pRenderer, 1, &elem.pFence);
    }

    Scene::PreDraw();

    // Reset cmd pool for this frame
    resetCmdPool(pRenderer, elem.pCmdPool);

    Cmd *cmd = elem.pCmds[0];
    beginCmd(cmd);

    cmdBeginGpuFrameProfile(cmd, gGpuProfileToken);

    RenderTargetBarrier barriers[]{
        {pRenderTarget, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET},
    };
    cmdResourceBarrier(cmd, 0, nullptr, 0, nullptr, 1, barriers);

    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw Scene");
    Scene::Draw(cmd, pRenderer, pRenderTarget);
    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    cmdSetViewport(cmd, 0, 0, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(cmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    BindRenderTargetsDesc bindRenderTargets = {
        .mRenderTargetCount = 1,
        .mRenderTargets = {{pRenderTarget, LOAD_ACTION_LOAD}},
    };

    cmdBindRenderTargets(cmd, &bindRenderTargets);

    cmdBeginGpuTimestampQuery(cmd, gGpuProfileToken, "Draw UI");

    FontDrawDesc gFrameTimeDraw{
        .mFontID = gFontID,
        .mFontColor = 0xff00ffff,
        .mFontSize = 18.0f,
    };
    float2 txtSizePx = cmdDrawCpuProfile(cmd, float2(8.f, 15.f), &gFrameTimeDraw);
    cmdDrawGpuProfile(cmd, float2(8.f, txtSizePx.y + 75.f), gGpuProfileToken, &gFrameTimeDraw);

    cmdDrawUserInterface(cmd);

    cmdEndGpuTimestampQuery(cmd, gGpuProfileToken);

    cmdBindRenderTargets(cmd, nullptr);
    barriers[0] = {
        pRenderTarget,
        RESOURCE_STATE_RENDER_TARGET,
        RESOURCE_STATE_PRESENT,
    };
    cmdResourceBarrier(cmd, 0, nullptr, 0, nullptr, 1, barriers);

    cmdEndGpuFrameProfile(cmd, gGpuProfileToken);
    endCmd(cmd);

    FlushResourceUpdateDesc flushUpdateDesc = {
        .mNodeIndex = 0,
    };
    flushResourceUpdates(&flushUpdateDesc);

    Semaphore *waitSemaphores[2] = {
        flushUpdateDesc.pOutSubmittedSemaphore,
        pImageAcquiredSemaphore,
    };

    QueueSubmitDesc submitDesc = {
        .ppCmds = &cmd,
        .pSignalFence = elem.pFence,
        .ppWaitSemaphores = waitSemaphores,
        .ppSignalSemaphores = &elem.pSemaphore,
        .mCmdCount = 1,
        .mWaitSemaphoreCount = 2,
        .mSignalSemaphoreCount = 1,
    };
    queueSubmit(pGraphicsQueue, &submitDesc);

    QueuePresentDesc presentDesc = {
        .pSwapChain = pSwapChain,
        .ppWaitSemaphores = waitSemaphores,
        .mWaitSemaphoreCount = 2,
        .mIndex = static_cast<uint8_t>(swapchainImageIndex),
        .mSubmitDone = true,
    };
    queuePresent(pGraphicsQueue, &presentDesc);
    
    flipProfiler();
}

DEFINE_APPLICATION_MAIN(MainApp);