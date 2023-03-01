#include "scene_manager.h"
#include "main.h"

extern Renderer *pRenderer;
extern SwapChain *pSwapChain;
extern RenderTarget *pDepthBuffer;

static std::unique_ptr<Scene> currentScene{nullptr};
static std::unique_ptr<Scene> nextScene{nullptr};

void SetCurrentSceneInternal(Scene *pScene) { currentScene.reset(pScene); };
void SetNextSceneInternal(Scene *pScene) { nextScene.reset(pScene);}

void UpdateCurrentScene(float deltaTime, uint32_t width, uint32_t height) {
    if (nextScene != nullptr){
        
        ReloadDesc reloadDesc = {};
        reloadDesc.mType = RELOAD_TYPE_ALL;
        
        currentScene->Unload(&reloadDesc, pRenderer);
        currentScene->Exit();
        
        currentScene = std::move(nextScene);
        nextScene.reset(nullptr);
        
        currentScene->Init(gImageCount);
        currentScene->Load(&reloadDesc, pRenderer, pSwapChain->ppRenderTargets[0], pDepthBuffer, gImageCount);
    }
    
    currentScene->Update(deltaTime, width, height);
}

void ExitCurrentScene() { currentScene->Exit(); }

void LoadCurrentScene(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
                        RenderTarget *pDepthBuffer, uint32_t imageCount)
{
    currentScene->Load(pReloadDesc, pRenderer, pRenderTarget, pDepthBuffer, imageCount);
}

void UnloadCurrentScene(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    currentScene->Unload(pReloadDesc, pRenderer);
}

void PreDrawCurrentScene(uint32_t frameIndex) { currentScene->PreDraw(frameIndex); }
void DrawCurrentScene(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex)
{
    currentScene->Draw(pCmd, pRenderTarget, pDepthBuffer, frameIndex);
}
void InitCurrentScene(uint32_t imageCount) { currentScene->Init(imageCount); }
