#pragma once

#include "Scene.h"

#include "Common_3/Renderer/IResourceLoader.h"

class TestScene : public Scene {
  public:
    virtual void Update(float deltaTime) override;
    virtual void Draw(Cmd *cmd) override;

    virtual bool Load() override;

    virtual void Unload() override;

    virtual void DoUI() override;

  private:
    Geometry *pGeometry;
    Texture *pTexture;
};
