#pragma once

#include <memory>

struct Cmd;
struct Renderer;

class Scene {
  public:
    virtual auto Load(Renderer *pRenderer) -> bool = 0;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw(Cmd *cmd) = 0;
    virtual void DoUI() = 0;
    virtual void Unload(Renderer *pRenderer) = 0;
};
