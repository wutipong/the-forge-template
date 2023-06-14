#ifndef DRAW_QUAD_H
#define DRAW_QUAD_H

#include <IGraphics.h>
#include <IResourceLoader.h>
#include <array>
#include <utility>
#include "Settings.h"

namespace DrawQuad
{
    struct Quad
    {
        mat4 transform;
        Texture *pTexture;

        // internal-use only
        DescriptorSet *_pDSTransform;
        std::array<Buffer *, IMAGE_COUNT> _pUniformBuffer;

        DescriptorSet *_pDSTexture;
    };

    void Init(SyncToken *token = nullptr);
    void Exit();

    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);

    bool InitQuad(SyncToken *token, Quad &q);
    void ExitQuad(Quad &q);

    bool LoadQuad(ReloadDesc *pReloadDesc, Renderer *pRenderer, Quad &q);
    void UnloadQuad(ReloadDesc *pReloadDesc, Renderer *pRenderer, Quad &q);

    void PreDrawQuad(Quad &quad, const uint32_t &imageIndex);
    void DrawQuad(Cmd *pCmd, Renderer *pRenderer, Quad &quad, const uint32_t &imageIndex);
}; // namespace DrawQuad

#endif