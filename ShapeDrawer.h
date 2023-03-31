//
// Created by mr_ta on 3/31/2023.
//

#ifndef SHAPEDRAWER_H
#define SHAPEDRAWER_H


#include "IGraphics.h"

class ShapeDrawer
{
public:
    enum class Shape
    {
        Cube,
        Sphere,
        Bone,
    };

    void Init();
    void Exit();

    void Draw(Cmd *pCmd, Shape shape);

    VertexLayout GetVertexLayout();

private:
    int cubeVertexCount{};
    Buffer *pVbCube{};

    int sphereVertexCount{};
    Buffer *pVbSphere{};

    int boneVertexCount{};
    Buffer *pVbBone{};
};


#endif // SHAPEDRAWER_H
