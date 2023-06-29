#include "PostProcessing.h"
#include <array>
#include "ColorGrading.h"
#include "SMAA.h"

namespace PostProcessing
{
    Desc desc;

    enum class Stage
    {
        None,

        SMAA,
        ColorGrading,
    };

    constexpr int STAGE_COUNT = 8;

    int activeStageCount = 0;
    std::array<Stage, STAGE_COUNT> stages = {Stage::None};
    std::array<RenderTarget *, STAGE_COUNT> pStageRenderTargets{nullptr};

    Texture *pColorGradingLUT{nullptr};

    bool Init(const PostProcessing::Desc &desc, SyncToken *token)
    {
        activeStageCount = 0;

        if (desc.mEnableSMAA)
        {
            SMAA::Init(token);
            stages[activeStageCount] = Stage::SMAA;
            activeStageCount++;
        }

        if (desc.mEnableColorGrading)
        {
            ColorGrading::Init(token);
            stages[activeStageCount] = Stage::ColorGrading;
            pColorGradingLUT = desc.pColorGradingLUT;
            activeStageCount++;
        }

        return true;
    }

    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, Texture *pTexture)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
        {
            for (int i = 0; i < activeStageCount - 1; i++)
            {
                RenderTargetDesc desc = {};
                desc.mArraySize = 1;
                desc.mDepth = 1;
                desc.mClearValue.r = 0.0f;
                desc.mClearValue.g = 0.0f;
                desc.mClearValue.b = 0.0f;
                desc.mClearValue.a = 0.0f;
                desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                desc.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                desc.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
                desc.mHeight = pRenderTarget->mHeight;
                desc.mWidth = pRenderTarget->mWidth;
                desc.mSampleCount = SAMPLE_COUNT_1;
                desc.mSampleQuality = 0;
                desc.mFlags = TEXTURE_CREATION_FLAG_OWN_MEMORY_BIT;
                desc.pName = "post processing intermediate";

                addRenderTarget(pRenderer, &desc, &pStageRenderTargets[i]);
            }
        }

        for (int i = 0; i < activeStageCount; i++)
        {
            Texture *pInput = (i == 0) ? pTexture : pStageRenderTargets[i - 1]->pTexture;
            RenderTarget *pOutput = (i == activeStageCount - 1) ? pRenderTarget : pStageRenderTargets[i];

            switch (stages[i])
            {
            case Stage::SMAA:
                SMAA::Load(pReloadDesc, pRenderer, pOutput, pInput);
                break;

            case Stage::ColorGrading:
                ColorGrading::Load(pReloadDesc, pRenderer, pOutput, pInput, pColorGradingLUT);
                break;
            }
        }

        return true;
    }

    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
        {
            for (int i = 0; i < activeStageCount - 1; i++)
            {
                removeRenderTarget(pRenderer, pStageRenderTargets[i]);
            }
        }

        for (int i = 0; i < activeStageCount; i++)
        {
            switch (stages[i])
            {
            case Stage::SMAA:
                SMAA::Unload(pReloadDesc, pRenderer);
                break;

            case Stage::ColorGrading:
                ColorGrading::Unload(pReloadDesc, pRenderer);
                break;
            }
        }
    }
    void Exit()
    {
        for (int i = 0; i < activeStageCount; i++)
        {
            switch (stages[i])
            {
            case Stage::SMAA:
                SMAA::Exit();
                break;

            case Stage::ColorGrading:
                ColorGrading::Exit();
                break;
            }
        }

        activeStageCount = 0;
    }

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget)
    {
        for (int i = 0; i < activeStageCount; i++)
        {
            RenderTarget *pOutput = (i == activeStageCount - 1) ? pRenderTarget : pStageRenderTargets[i];

            if (pOutput != pRenderTarget)
            {
                cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
                std::array<RenderTargetBarrier, 1> barriers = {
                    RenderTargetBarrier{pOutput, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET},
                };
                cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
            }

            switch (stages[i])
            {
            case Stage::SMAA:
                SMAA::Draw(pCmd, pRenderer, pOutput);
                break;

            case Stage::ColorGrading:
                ColorGrading::Draw(pCmd, pRenderer, pOutput);
                break;
            }

            if (pOutput != pRenderTarget)
            {
                cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);

                std::array<RenderTargetBarrier, 1> barriers = {
                    RenderTargetBarrier{pOutput, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE},
                };
                cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
            }
        }
    }
} // namespace PostProcessing