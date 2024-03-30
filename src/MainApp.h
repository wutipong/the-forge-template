#ifndef MAIN_H
#define MAIN_H

#include <IApp.h>

class MainApp : public IApp
{
public:
    bool Init() override;
    void Exit() override;
    bool Load(ReloadDesc *pReloadDesc) override;
    void Unload(ReloadDesc *pReloadDesc) override;
    void Draw() override;
    void Update(float deltaTime) override;
    const char *GetName() override;
};

#endif // MAIN_H
