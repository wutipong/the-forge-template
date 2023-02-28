#ifndef SCENE_H
#define SCENE_H

#include <ICameraController.h>
#include <IGraphics.h>
#include <IOperatingSystem.h>
#include <Math/MathTypes.h>

class Scene
{
public:
    virtual void Init(uint32_t imageCount) = 0;
    virtual void Exit() = 0;
    virtual void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
                      RenderTarget *pDepthBuffer, uint32_t imageCount) = 0;
    virtual void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer) = 0;
    virtual void Update(float deltaTime, uint32_t width, uint32_t height) = 0;
    virtual void PreDraw(uint32_t frameIndex) = 0;
    virtual void Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex) = 0;
};

#endif