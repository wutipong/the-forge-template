#include "AppInterface.h"

#include "MainApp.h"

DEFINE_APPLICATION_MAIN(AppInterface)

bool AppInterface::Init() { return ::Init(this); }
void AppInterface::Exit() { return ::Exit(); }
bool AppInterface::Load() { return ::Load(); }
void AppInterface::Unload() { return ::Unload(); }
void AppInterface::Update(float deltaTime) { return ::Update(deltaTime); }
void AppInterface::Draw() { return ::Draw(); }
