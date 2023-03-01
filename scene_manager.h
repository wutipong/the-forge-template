#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene.h"

#include <memory>
void SetCurrentSceneInternal(Scene *pScene);
void SetNextSceneInternal(Scene *pScene);

template <typename SceneClass, typename... ParamsClass>
void SetStartScene(ParamsClass... args)
{
    auto scene = new SceneClass(args...);
    SetCurrentSceneInternal(scene);
}

template <typename SceneClass, typename... ParamsClass>
void SetNextScene(ParamsClass... args)
{
    auto scene = new SceneClass(args...);
    SetNextSceneInternal(scene);
}

void InitCurrentScene(uint32_t imageCount);
void ExitCurrentScene();
void LoadCurrentScene(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
                      RenderTarget *pDepthBuffer, uint32_t imageCount);

void UnloadCurrentScene(ReloadDesc *pReloadDesc, Renderer *pRenderer);
void UpdateCurrentScene(float deltaTime, uint32_t width, uint32_t height);
void PreDrawCurrentScene(uint32_t frameIndex);
void DrawCurrentScene(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex);

#endif // SCENE_MANAGER_H
