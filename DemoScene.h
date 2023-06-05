#ifndef DEMO_SCENE_H
#define DEMO_SCENE_H

#include <IGraphics.h>

namespace DemoScene
{
    void Init(uint32_t imageCount);
    void Exit();
    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageCount);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    void Update(float deltaTime, uint32_t width, uint32_t height);
    void PreDraw(uint32_t frameIndex);
    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t frameIndex);
}; // namespace DemoScene


#endif // DEMO_SCENE_H
