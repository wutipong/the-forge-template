#ifndef POST_PROCESSING_H
#define POST_PROCESSING_H

#include <IGraphics.h>
#include <IResourceLoader.h>

namespace PostProcessing
{
    struct Desc
    {
        bool mEnableSMAA;
        bool mEnableColorGrading;

        Texture *pColorGradingLUT;
    };

    bool Init(const Desc &desc, SyncToken *token = nullptr);
    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, Texture *pTexture);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
    void Exit();

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget);
} // namespace PostProcessing

#endif