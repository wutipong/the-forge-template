#ifndef MAIN_H
#define MAIN_H

#include <IApp.h>
#include <IFileSystem.h>
#include <IFont.h>
#include <IGraphics.h>
#include <IInput.h>
#include <IResourceLoader.h>
#include <IUI.h>
#include "IProfiler.h"

constexpr uint32_t gImageCount = 3;

class MainApp : public IApp
{
private:
    UIComponent *pGuiWindow = nullptr;
    Queue *pGraphicsQueue = nullptr;
    uint32_t gFontID = 0;
    Cmd *pCmds[gImageCount]{nullptr};
    CmdPool *pCmdPools[gImageCount]{nullptr};
    uint32_t gFrameIndex = 0;
    Fence *pRenderCompleteFences[gImageCount]{nullptr};
    Semaphore *pRenderCompleteSemaphores[gImageCount] = {nullptr};
    Semaphore *pImageAcquiredSemaphore = nullptr;

    ProfileToken gGpuProfileToken = PROFILE_INVALID_TOKEN;
    FontDrawDesc gFrameTimeDraw;

public:
    bool Init() override;
    void Exit() override;
    bool Load(ReloadDesc *pReloadDesc) override;
    void Unload(ReloadDesc *pReloadDesc) override;
    void Draw() override;
    void Update(float deltaTime) override;
    const char *GetName() override;
};

#endif // MAIN_H
