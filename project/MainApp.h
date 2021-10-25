#pragma once

#include <Common_3/OS/Interfaces/IApp.h>

constexpr int ImageCount = 3;

auto Init(IApp *app) -> bool;
void Exit();

auto Load() -> bool;
void Unload();

void Update(float deltaTime);
void Draw();

auto AppInstance() -> IApp *;