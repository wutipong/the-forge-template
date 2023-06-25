#include "PostProcessing.h"
#include "ColorGrading.h"
#include "SMAA.h"

namespace PostProcessing
{
    Desc desc;

    bool Init(const Desc &desc, SyncToken *token = nullptr)
    {
        PostProcessing::desc = desc;

        if (desc.mEnableSMAA)
        {
            SMAA::Init(token);
        }
        if (desc.mEnableColorGrading)
        {
            ColorGrading::Init(token);
        }

        return true;
    }

    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, Texture *pTexture)
    {
        if (desc.mEnableSMAA)
        {
            SMAA::Load(pReloadDesc, pRenderer, pRenderTarget, pTexture);
        }

        if (desc.mEnableColorGrading)
        {
            ColorGrading::Load(pReloadDesc, pRenderer, pRenderTarget, pTexture);
        }

        return true;
    }
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer){}
    void Exit(){}

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget){}
} // namespace PostProcessing