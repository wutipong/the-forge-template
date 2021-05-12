#pragma once

#include <memory>

struct Cmd;

class Scene {
  public:
    virtual auto Load() -> bool = 0;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw(Cmd *cmd) = 0;
    virtual void DoUI() = 0;
    virtual void Unload() = 0;
};
