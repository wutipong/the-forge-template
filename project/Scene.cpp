#include "Scene.h"

std::unique_ptr<Scene> Scene::currentScene = nullptr;
bool Scene::IsShowingUI = false;

bool Scene::Init() { return true; }

bool Scene::LoadCurrent() { return currentScene->Load(); }
void Scene::UpdateCurrent(float delta) { currentScene->Update(delta); }
void Scene::DrawCurrent(Cmd *cmd) { currentScene->Draw(cmd); }
void Scene::UnloadCurrent() { currentScene->Unload(); }

void Scene::Exit() { }
