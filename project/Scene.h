#pragma once

#include "MainApp.h"

#include <memory>

struct Cmd;

class Scene {
  public:
    virtual bool Load() = 0;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw(Cmd *cmd) = 0;
    virtual void DoUI() = 0;
    virtual void Unload() = 0;

    static bool Init();
    static bool LoadCurrent();
    static void UpdateCurrent(float deltaTime);
    static void UnloadCurrent();
    static void DrawCurrent(Cmd *cmd);
    static void Exit();

    static void EnableUI(bool b) { IsShowingUI = b; }

    template <class SceneClass> static bool ChangeScene() {
        if (currentScene != nullptr) {
            currentScene->Unload();
            currentScene->Exit();
        }

        currentScene = std::make_unique<SceneClass>();

        return true;
    }

  private:
    static std::unique_ptr<Scene> currentScene;
    static bool IsShowingUI;
};
