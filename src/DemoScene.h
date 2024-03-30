#ifndef DEMO_SCENE_H
#define DEMO_SCENE_H

#include <IGraphics.h>

namespace DemoScene
{
    bool Init();
    void Exit();
    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    void Update(float deltaTime, uint32_t width, uint32_t height);
    void PreDraw(uint32_t imageIndex);
    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageIndex);
}; // namespace DemoScene


#endif // DEMO_SCENE_H
