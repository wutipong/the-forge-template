#include "QuadDemoScene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IOperatingSystem.h>
#include <IResourceLoader.h>
#include <IUI.h>
#include <Math/MathTypes.h>
#include <array>
#include "DrawQuad.h"
#include "Settings.h"

namespace QuadDemoScene
{
    Texture *pTextures[2];
    DrawQuad::Quad quads[2];

} // namespace QuadDemoScene

bool QuadDemoScene::Init()
{
    DrawQuad::Init();

    SyncToken token = {};
    TextureLoadDesc desc{};
    desc.ppTexture = &pTextures[0];
    desc.pFileName = "Quad0";

    addResource(&desc, &token);

    desc = {};
    desc.ppTexture = &pTextures[1];
    desc.pFileName = "Quad1";

    addResource(&desc, &token);

    waitForToken(&token);

    quads[0].pTexture = pTextures[0];
    quads[0].transform = mat4::translation({0.25f, 0.25f, 0.0f}) * mat4::scale({0.25f, 0.25f, 1.0f});

    quads[1].pTexture = pTextures[1];
    quads[1].transform = mat4::translation({-0.25f, -0.25f, 0.0f}) * mat4::scale({0.25f, 0.25f, 1.0f});

    for (auto &quad : quads)
    {
        quad.Init();
    }

    return true;
}

void QuadDemoScene::Exit()
{
    for (auto &pTexture : pTextures)
    {
        removeResource(pTexture);
    }

    for (auto &quad : quads)
    {
        quad.Exit();
    }

    DrawQuad::Exit();
}

bool QuadDemoScene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget)
{
    DrawQuad::Load(pReloadDesc, pRenderer, pRenderTarget);
    for (auto &quad : quads)
    {
        quad.Load(pReloadDesc, pRenderer);
    }

    return true;
}

void QuadDemoScene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    for (auto &quad : quads)
    {
        quad.Unload(pReloadDesc, pRenderer);
    }

    DrawQuad::Unload(pReloadDesc, pRenderer);
}

void QuadDemoScene::Update(float deltaTime, uint32_t width, uint32_t height) {}

void QuadDemoScene::Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageIndex)
{
    // simply record the screen cleaning command
    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
    loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;

    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;
    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);

    for (auto &quad : quads)
    {
        quad.Draw(pCmd, pRenderer, imageIndex);
    }
}

void QuadDemoScene::PreDraw(uint32_t imageIndex)
{
    for (auto &quad : quads)
    {
        quad.PreDraw(imageIndex);
    }
}
