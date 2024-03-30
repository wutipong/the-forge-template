//
// Created by mr_ta on 3/31/2023.
//

#include "DrawShape.h"

#include <IResourceLoader.h>

namespace DrawShape
{
    int cubeVertexCount{};
    Buffer *pVbCube{};

    int sphereVertexCount{};
    Buffer *pVbSphere{};

    int boneVertexCount{};
    Buffer *pVbBone{};
} // namespace DrawShape

void DrawShape::Init()
{
    float *vertices{};

    generateCuboidPoints(&vertices, &cubeVertexCount);
    BufferLoadDesc bufferLoadDesc = {};
    bufferLoadDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    bufferLoadDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    bufferLoadDesc.mDesc.mSize = cubeVertexCount * sizeof(float);
    bufferLoadDesc.pData = vertices;
    bufferLoadDesc.ppBuffer = &pVbCube;
    addResource(&bufferLoadDesc, nullptr);

    tf_free(vertices);

    generateSpherePoints(&vertices, &sphereVertexCount, 64);
    bufferLoadDesc = {};
    bufferLoadDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    bufferLoadDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    bufferLoadDesc.mDesc.mSize = sphereVertexCount * sizeof(float);
    bufferLoadDesc.pData = vertices;
    bufferLoadDesc.ppBuffer = &pVbSphere;
    addResource(&bufferLoadDesc, nullptr);

    tf_free(vertices);

    generateBonePoints(&vertices, &boneVertexCount, 0.25f);
    bufferLoadDesc = {};
    bufferLoadDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    bufferLoadDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    bufferLoadDesc.mDesc.mSize = boneVertexCount * sizeof(float);
    bufferLoadDesc.pData = vertices;
    bufferLoadDesc.ppBuffer = &pVbBone;
    addResource(&bufferLoadDesc, nullptr);

    tf_free(vertices);
}

void DrawShape::Exit()
{
    removeResource(pVbCube);
    removeResource(pVbSphere);
    removeResource(pVbBone);
}

VertexLayout DrawShape::GetVertexLayout()
{
    // layout and pipeline for sphere draw
    VertexLayout vertexLayout = {};
    vertexLayout.mAttribCount = 2;
    
    vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
    vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
    vertexLayout.mAttribs[0].mBinding = 0;
    vertexLayout.mAttribs[0].mLocation = 0;
    vertexLayout.mAttribs[0].mOffset = 0;

    vertexLayout.mAttribs[1].mSemantic = SEMANTIC_NORMAL;
    vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
    vertexLayout.mAttribs[1].mBinding = 0;
    vertexLayout.mAttribs[1].mLocation = 1;
    vertexLayout.mAttribs[1].mOffset = 3 * sizeof(float);

    vertexLayout.mBindingCount = 1;
    vertexLayout.mBindings[0].mStride = sizeof(float) * 6;

    return vertexLayout;
}

void DrawShape::Draw(Cmd *pCmd, Shape shape)
{
    int vertexCount = 0;
    constexpr uint32_t stride = sizeof(float) * 6;

    switch (shape)
    {
    case Shape::Cube:
        cmdBindVertexBuffer(pCmd, 1, &pVbCube, &stride, nullptr);
        vertexCount = cubeVertexCount;
        break;

    case Shape::Sphere:
        cmdBindVertexBuffer(pCmd, 1, &pVbSphere, &stride, nullptr);
        vertexCount = sphereVertexCount;
        break;

    case Shape::Bone:
        cmdBindVertexBuffer(pCmd, 1, &pVbBone, &stride, nullptr);
        vertexCount = boneVertexCount;
        break;
    }

    cmdDraw(pCmd, vertexCount / 6, 0);
}
