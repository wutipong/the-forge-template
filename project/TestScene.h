#pragma once

#include "Scene.h"

#include "Common_3/Renderer/IResourceLoader.h"

class TestScene : public Scene {
  public:
    void Update(float deltaTime) override;
    void Draw(Cmd *cmd) override;

    auto Load(Renderer *pRenderer) -> bool override;

    void Unload(Renderer *pRenderer) override;

    void DoUI() override;

  private:
    Geometry *pGeometry;
    Texture *pTexture;
    Shader *pShader;
};
