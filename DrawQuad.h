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
    public:
        mat4 transform;
        Texture *pTexture;

        bool Init(SyncToken *token = nullptr);
        void Exit();

        bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer);
        void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);

        void PreDraw(const uint32_t &imageIndex);
        void Draw(Cmd *pCmd, Renderer *pRenderer, const uint32_t &imageIndex);

    private:
        DescriptorSet *_pDSTransform;
        std::array<Buffer *, IMAGE_COUNT> _pUniformBuffer;

        DescriptorSet *_pDSTexture;
    };

    void Init(SyncToken *token = nullptr);
    void Exit();

    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget);
    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer);
}; // namespace DrawQuad

#endif