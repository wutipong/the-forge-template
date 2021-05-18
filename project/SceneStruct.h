#pragma once

#include <functional>

#include "Scene.h"

struct SceneStruct : Scene {
  public:
    std::function<bool(Renderer *pRenderer, SwapChain *pSwapChain)> DoLoad;
    std::function<void(float)> DoUpdate;
    std::function<void(Cmd *cmd, int imageIndex)> DoDraw;
    std::function<void()> DoUI;
    std::function<void(Renderer *pRenderer)> DoUnload;

    virtual auto Load(Renderer *pRenderer, SwapChain *pSwapChain) -> bool sealed{ return DoLoad(pRenderer, pSwapChain); } ;
    virtual void Update(float deltaTime) sealed { DoUpdate(deltaTime); };
    virtual void Draw(Cmd *cmd, int imageIndex) sealed { DoDraw(cmd, imageIndex); };
    virtual void DrawUI() sealed { DrawUI(); };
    virtual void Unload(Renderer *pRenderer) sealed { DoUnload(pRenderer); };

    virtual ~SceneStruct(){};
};
