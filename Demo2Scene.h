//
// Created by mr_ta on 3/4/2023.
//

#ifndef DEMO2SCENE_H
#define DEMO2SCENE_H

#include <IGraphics.h>

namespace Demo2Scene
{
    bool Init();
    void Exit();
    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    void Update(float deltaTime, uint32_t width, uint32_t height);
    void PreDraw(uint32_t frameIndex);
    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t frameIndex);
}; // namespace Demo2Scene

#endif // DEMO2SCENE_H
