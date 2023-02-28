#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene.h"

#include <memory>

class SceneManager : public Scene
{
private:
    std::unique_ptr<Scene> currentScene;

public:
    template <typename StartScene>
    void InitManager(uint32_t imageCount)
    {
        currentScene = std::make_unique<StartScene>();
        currentScene->Init(imageCount);
    }

    template <typename Scene>
    void ChangeScene(uint32_t imageCount)
    {
        ReloadDesc reloadDesc = {};
        reloadDesc.mType = RELOAD_TYPE_ALL;

        currentScene->Unload(&reloadDesc, pRenderer);
        currentScene->Exit();

        currentScene = std::make_unique<Scene>();
        currentScene->Init(imageCount);
        currentScene->Load(reloadDesc, pRenderer, pRenderTarget, pDepthBuffer, imageCount);
    }

    void Init(uint32_t imageCount) override;
    void Exit() override;
    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer,
              uint32_t imageCount) override;
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer) override;
    void Update(float deltaTime, uint32_t width, uint32_t height) override;
    void PreDraw(uint32_t frameIndex) override;
    void Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex) override;
};


#endif // SCENE_MANAGER_H
