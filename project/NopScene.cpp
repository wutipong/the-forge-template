#include "NopScene.h"

auto NopScene::Create() -> Scene {
    return {[](Renderer *pRenderer, SwapChain *pSwapChain) { return true; },
            [](float) {},
            [](Cmd *cmd, int imageIndex) {},
            []() {},
            [](Renderer *pRenderer) {},
            [](Renderer *pRenderer) {},
            [](Renderer *pRenderer) {}};
}
