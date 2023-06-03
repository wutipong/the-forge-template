//
// Created by mr_ta on 3/4/2023.
//

#ifndef DEMO2SCENE_H
#define DEMO2SCENE_H

#include "IInput.h"
#include "IUI.h"
#include "ShapeDrawer.h"

namespace Demo2Scene
{
    void Init(uint32_t imageCount);
    void Exit();
    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer,
              uint32_t imageCount);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    void Update(float deltaTime, uint32_t width, uint32_t height);
    void PreDraw(uint32_t frameIndex);
    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer,
              uint32_t frameIndex);

    bool OnInputAction(InputActionContext *ctx);
}; // namespace Demo2Scene

#endif // DEMO2SCENE_H
