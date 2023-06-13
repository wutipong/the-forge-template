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
        mat4 Transform;
        Texture *pTexture;
        std::array<DescriptorSet *, IMAGE_COUNT> pDescriptorSets;
    };

    bool Init();
    void Exit();

    bool Load();
    void Unload();

    bool LoadQuad(Quad &q);
    void UnloadQuad(Quad &q);

    std::pair<Quad, bool> CreateQuad(Texture *pTexture);
    void RemoveQuad(Quad &q);

    void DrawQuad(Quad &q);
} // namespace DrawQuad

#endif