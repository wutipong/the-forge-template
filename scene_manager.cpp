//
// Created by mr_ta on 2/28/2023.
//

#include "scene_manager.h"
void SceneManager::Exit() { currentScene->Exit(); }

void SceneManager::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
                        RenderTarget *pDepthBuffer, uint32_t imageCount)
{
    currentScene->Load(pReloadDesc, pRenderer, pRenderTarget, pDepthBuffer, imageCount);
}

void SceneManager::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    currentScene->Unload(pReloadDesc, pRenderer);
}

void SceneManager::Update(float deltaTime, uint32_t width, uint32_t height)
{
    currentScene->Update(deltaTime, width, height);
}
void SceneManager::PreDraw(uint32_t frameIndex) { currentScene->PreDraw(frameIndex); }
void SceneManager::Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex)
{
    currentScene->Draw(pCmd, pRenderTarget, pDepthBuffer, frameIndex);
}
void SceneManager::Init(uint32_t imageCount) {
    currentScene->Init(imageCount);
}
