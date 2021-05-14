#pragma once

#include <memory>

struct Cmd;
struct Renderer;
struct SwapChain;

class Scene {
  public:
    virtual auto Load(Renderer *pRenderer, SwapChain *pSwapChain) -> bool = 0;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw(Cmd *cmd, int imageIndex) = 0;
    virtual void DoUI() = 0;
    virtual void Unload(Renderer *pRenderer) = 0;

    virtual ~Scene(){};
};
