#ifndef COLOR_GRADING_H
#define COLOR_GRADING_H

#include <IGraphics.h>
#include <IResourceLoader.h>

namespace ColorGrading
{
    void Init(SyncToken *token = nullptr);
    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, Texture *pTexture, Texture* pLUT);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    void Exit();

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget);
} // namespace ColorGrading

#endif