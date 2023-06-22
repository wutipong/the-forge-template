#ifndef MSAA_H
#define MSAA_H

#include <IResourceLoader.h>

namespace SMAA
{
    void Init(SyncToken *token);
    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, Texture *pTexture);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    void Exit();

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget);
} // namespace SMAA
#endif