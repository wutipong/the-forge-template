#include "QuadDemoScene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IOperatingSystem.h>
#include <IResourceLoader.h>
#include <IUI.h>
#include <Math/MathTypes.h>
#include <array>
#include "Settings.h"

namespace ChessScene
{
    const char *pFileName = "Chess.gltf";
    Geometry *pGeometry{nullptr};

    Texture *pTextures[2];

    bool Init()
    {
        SyncToken token = {};

        VertexLayout vertexLayout = {};
        vertexLayout.mAttribCount = 3;
        vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
        vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
        vertexLayout.mAttribs[0].mBinding = 0;
        vertexLayout.mAttribs[0].mLocation = 0;
        vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
        vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R16G16_SFLOAT;
        vertexLayout.mAttribs[1].mBinding = 1;
        vertexLayout.mAttribs[1].mLocation = 1;
        vertexLayout.mAttribs[2].mSemantic = SEMANTIC_NORMAL;
        vertexLayout.mAttribs[2].mFormat = TinyImageFormat_R16G16_UNORM;
        vertexLayout.mAttribs[2].mBinding = 2;
        vertexLayout.mAttribs[2].mLocation = 2;

        GeometryLoadDesc loadDesc = {};
        loadDesc.pFileName = pFileName;
        loadDesc.ppGeometry = &pGeometry;
        loadDesc.pVertexLayout = &vertexLayout;
        loadDesc.mFlags = GEOMETRY_LOAD_FLAG_SHADOWED;
        
        addResource(&loadDesc, &token);

        waitForAllResourceLoads();

        return true;
    }

    void Exit() { removeResource(pGeometry); }

    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget) { return true; }

    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer) {}

    void Update(float deltaTime, uint32_t width, uint32_t height) {}

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageIndex) {}

    void PreDraw(uint32_t imageIndex) {}
} // namespace ChessScene