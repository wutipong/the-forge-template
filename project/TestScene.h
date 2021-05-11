#pragma once

#include "Scene.h"

class TestScene : public Scene{
  public:
    virtual void Update(float deltaTime) override;
    virtual void Draw(Cmd *cmd) override;

    virtual bool Load() override;

    virtual void Unload() override;

    virtual void DoUI() override;
};
