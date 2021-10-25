#pragma once

#include <Common_3/OS/Interfaces/IApp.h>

class AppInterface : public IApp {
  public:
    virtual bool Init() override;
    virtual void Exit() override;

    virtual bool Load() override;
    virtual void Unload() override;

    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    virtual const char *GetName() override { return "The-Forge Template"; };
};
