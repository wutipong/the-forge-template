//
// Created by mr_ta on 3/31/2023.
//

#ifndef SHAPEDRAWER_H
#define SHAPEDRAWER_H

#include "IGraphics.h"

namespace DrawShape
{
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
}; // namespace DrawShape


#endif // SHAPEDRAWER_H
