#include "DemoScene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IOperatingSystem.h>
#include <IResourceLoader.h>
#include <IUI.h>
#include <Math/MathTypes.h>
#include <array>
#include "Settings.h"

namespace DemoScene
{
    int spherePoints = 0;
    int quadPoints = 0;

    constexpr size_t MAX_SPHERE = 768;
    std::array<vec3, MAX_SPHERE> position{};
    std::array<vec4, MAX_SPHERE> color{};
    vec3 lightPosition{1.0f, 0, 0};
    vec3 lightColor{0.9f, 0.9f, 0.7f};

    struct SphereUniformBlock
    {
        CameraMatrix projectView;
        std::array<mat4, MAX_SPHERE> world;
        std::array<vec4, MAX_SPHERE> color;

        vec3 lightPosition;
        vec3 lightColor;
    } sphereUniform = {};

    struct QuadUniformBlock
    {
        CameraMatrix projectView;
        mat4 world;
        vec4 color;
    } quadUniform = {};

    Shader *pShaderSphere{nullptr};
    RootSignature *pRSSphere{nullptr};
    DescriptorSet *pDSSphereUniform{nullptr};
    Buffer *pBufferSphereUniform{nullptr};
    Buffer *pBufferSphereVertex{nullptr};
    Pipeline *pPipelineSphere{nullptr};

    Shader *pShaderQuad{nullptr};
    RootSignature *pRSQuad{nullptr};
    DescriptorSet *pDSQuadUniform{nullptr};
    Buffer *pBufferQuadVertex{nullptr};
    Buffer *pBufferQuadIndex{nullptr};
    Buffer *pBufferQuadUniform{nullptr};
    Pipeline *pPipelineQuad{nullptr};

    ICameraController *pCameraController{nullptr};

    RenderTarget *pDepthBuffer{nullptr};
    TinyImageFormat depthBufferFormat = TinyImageFormat_D32_SFLOAT;

    void AddSphereResources(Renderer *pRenderer);
    void RemoveSphereResources(Renderer *pRenderer);

    void AddQuadResources(Renderer *pRenderer);
    void RemoveQuadResources(Renderer *pRenderer);
} // namespace DemoScene

bool DemoScene::Init()
{
    float *sphereVertices{};
    generateSpherePoints(&sphereVertices, &spherePoints, 12, 1.0f);

    float *quadVertices{};
    generateQuad(&quadVertices, &quadPoints);

    uint16_t quadIndices[6] = {0, 1, 2, 1, 3, 2};

    SyncToken token{};

    uint64_t sphereDataSize = spherePoints * sizeof(float);
    BufferLoadDesc sphereVbDesc{
        .ppBuffer = &pBufferSphereVertex,
        .pData = sphereVertices,
        .mDesc{
            .mSize = sphereDataSize,
            .mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY,
            .mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER,
        },
    };
    addResource(&sphereVbDesc, &token);

    uint64_t quadDataSize = quadPoints * sizeof(float);
    BufferLoadDesc quadVbDesc{
        .ppBuffer = &pBufferQuadVertex,
        .pData = quadVertices,
        .mDesc{
            .mSize = quadDataSize,
            .mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY,
            .mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER,
        },
    };
    addResource(&quadVbDesc, &token);

    BufferLoadDesc quadIdDesc{
        .ppBuffer = &pBufferQuadIndex,
        .pData = quadIndices,
        .mDesc{
            .mSize = sizeof(uint16_t) * 6,
            .mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY,
            .mDescriptors = DESCRIPTOR_TYPE_INDEX_BUFFER,
        },
    };
    addResource(&quadIdDesc, &token);

    BufferLoadDesc ubDesc = {
        .ppBuffer = &pBufferSphereUniform,
        .mDesc{
            .mSize = sizeof(SphereUniformBlock),
            .mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
            .mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT,
            .mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        },
    };
    addResource(&ubDesc, &token);

    BufferLoadDesc quadUniformDesc = {
        .ppBuffer = &pBufferQuadUniform,
        .mDesc{
            .mSize = sizeof(QuadUniformBlock),
            .mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
            .mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT,
            .mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        },
    };
    addResource(&quadUniformDesc, &token);

    for (size_t i = 0; i < MAX_SPHERE; i++)
    {
        position[i] = {randomFloat(-100, 100), randomFloat(-100, 100), randomFloat(-100, 100)};
        color[i] = {randomFloat01(), randomFloat01(), randomFloat01(), 1.0f};
    }

    CameraMotionParameters cmp{160.0f, 600.0f, 1000.0f};
    vec3 camPos{0.0f, 0.0f, 20.0f};
    vec3 lookAt{vec3(0)};

    pCameraController = initFpsCameraController(camPos, lookAt);
    pCameraController->setMotionParameters(cmp);

    waitForToken(&token);

    tf_free(sphereVertices);
    tf_free(quadVertices);

    return true;
}

void DemoScene::Exit()
{
    removeResource(pBufferSphereUniform);
    removeResource(pBufferSphereVertex);

    removeResource(pBufferQuadUniform);
    removeResource(pBufferQuadVertex);
    removeResource(pBufferQuadIndex);

    exitCameraController(pCameraController);
}

bool DemoScene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget)
{
    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        AddSphereResources(pRenderer);
        AddQuadResources(pRenderer);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        RenderTargetDesc desc{
            .mFlags = TEXTURE_CREATION_FLAG_ON_TILE | TEXTURE_CREATION_FLAG_VR_MULTIVIEW,
            .mWidth = pRenderTarget->mWidth,
            .mHeight = pRenderTarget->mHeight,
            .mDepth = 1,
            .mArraySize = 1,
            .mSampleCount = SAMPLE_COUNT_1,
            .mFormat = depthBufferFormat,
            .mStartState = RESOURCE_STATE_DEPTH_WRITE,
            .mClearValue = {},
            .mSampleQuality = 0,
        };
        addRenderTarget(pRenderer, &desc, &pDepthBuffer);
        ASSERT(pDepthBuffer);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        // layout and pipeline for sphere draw
        VertexLayout vertexLayout = {
            .mBindings{
                {
                    .mStride = sizeof(float) * 6,
                },
            },
            .mAttribs{
                {
                    .mSemantic = SEMANTIC_POSITION,
                    .mFormat = TinyImageFormat_R32G32B32_SFLOAT,
                    .mBinding = 0,
                    .mLocation = 0,
                    .mOffset = 0,
                },
                {
                    .mSemantic = SEMANTIC_NORMAL,
                    .mFormat = TinyImageFormat_R32G32B32_SFLOAT,
                    .mBinding = 0,
                    .mLocation = 1,
                    .mOffset = 3 * sizeof(float),
                },
            },
            .mBindingCount = 1,
            .mAttribCount = 2,
        };

        RasterizerStateDesc sphereRasterizerStateDesc = {};
        sphereRasterizerStateDesc.mCullMode = CULL_MODE_NONE;

        DepthStateDesc depthStateDesc{
            .mDepthTest = true,
            .mDepthWrite = true,
            .mDepthFunc = CMP_GEQUAL,
        };

        PipelineDesc desc = {
            .mGraphicsDesc{
                .pShaderProgram = pShaderSphere,
                .pRootSignature = pRSSphere,
                .pVertexLayout = &vertexLayout,
                .pDepthState = &depthStateDesc,
                .pRasterizerState = &sphereRasterizerStateDesc,
                .pColorFormats = &pRenderTarget->mFormat,
                .mRenderTargetCount = 1,
                .mSampleCount = pRenderTarget->mSampleCount,
                .mSampleQuality = pRenderTarget->mSampleQuality,
                .mDepthStencilFormat = depthBufferFormat,
                .mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST,
                .mVRFoveatedRendering = true,
            },
            .mType = PIPELINE_TYPE_GRAPHICS,
        };

        addPipeline(pRenderer, &desc, &pPipelineSphere);
        ASSERT(pPipelineSphere);

        desc.mGraphicsDesc = {
            .pShaderProgram = pShaderQuad,
            .pRootSignature = pRSQuad,
            .pVertexLayout = &vertexLayout,
            .pDepthState = &depthStateDesc,
            .pRasterizerState = &sphereRasterizerStateDesc,
            .pColorFormats = &pRenderTarget->mFormat,
            .mRenderTargetCount = 1,
            .mSampleCount = pRenderTarget->mSampleCount,
            .mSampleQuality = pRenderTarget->mSampleQuality,
            .mDepthStencilFormat = depthBufferFormat,
            .mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST,
            .mVRFoveatedRendering = true,
        };

        addPipeline(pRenderer, &desc, &pPipelineQuad);
        ASSERT(pPipelineQuad);
    }

    DescriptorData params = {
        .pName = "uniformBlock",
        .ppBuffers = &pBufferSphereUniform,
    };

    updateDescriptorSet(pRenderer, 0, pDSSphereUniform, 1, &params);

    params.ppBuffers = &pBufferQuadUniform;
    updateDescriptorSet(pRenderer, 0, pDSQuadUniform, 1, &params);

    return true;
}

void DemoScene::AddSphereResources(Renderer *pRenderer)
{
    ShaderLoadDesc shaderDesc{
        .mStages{
            {
                .pFileName = "sphere.vert",
            },
            {
                .pFileName = "basic.frag",
            },
        },
    };

    addShader(pRenderer, &shaderDesc, &pShaderSphere);
    ASSERT(pShaderSphere);

    RootSignatureDesc rootDesc{
        .ppShaders = &pShaderSphere,
        .mShaderCount = 1,
    };

    addRootSignature(pRenderer, &rootDesc, &pRSSphere);
    ASSERT(pRSSphere);

    DescriptorSetDesc dsDesc = {pRSSphere, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1};
    addDescriptorSet(pRenderer, &dsDesc, &pDSSphereUniform);

    ASSERT(pDSSphereUniform);
}

void DemoScene::AddQuadResources(Renderer *pRenderer)
{
    ShaderLoadDesc shaderDesc{
        .mStages{
            {
                .pFileName = "quad.vert",
            },
            {
                .pFileName = "basic.frag",
            },
        },
    };

    addShader(pRenderer, &shaderDesc, &pShaderQuad);
    ASSERT(pShaderQuad);

    RootSignatureDesc rootDesc{
        .ppShaders = &pShaderQuad,
        .mShaderCount = 1,
    };

    addRootSignature(pRenderer, &rootDesc, &pRSQuad);
    ASSERT(pRSQuad);

    DescriptorSetDesc dsDesc = {pRSQuad, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1};
    addDescriptorSet(pRenderer, &dsDesc, &pDSQuadUniform);

    ASSERT(pDSQuadUniform);
}

void DemoScene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(pRenderer, pPipelineSphere);
        removePipeline(pRenderer, pPipelineQuad);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        RemoveSphereResources(pRenderer);
        RemoveQuadResources(pRenderer);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        removeRenderTarget(pRenderer, pDepthBuffer);
    }
}

void DemoScene::RemoveSphereResources(Renderer *pRenderer)
{
    removeDescriptorSet(pRenderer, pDSSphereUniform);
    removeRootSignature(pRenderer, pRSSphere);
    removeShader(pRenderer, pShaderSphere);
}

void DemoScene::RemoveQuadResources(Renderer *pRenderer)
{
    removeDescriptorSet(pRenderer, pDSQuadUniform);
    removeRootSignature(pRenderer, pRSQuad);
    removeShader(pRenderer, pShaderQuad);
}

void DemoScene::Update(float deltaTime, uint32_t width, uint32_t height)
{
    pCameraController->update(deltaTime);
    const float aspectInverse = (float)height / (float)width;
    const float horizontal_fov = PI / 2.0f;
    CameraMatrix projMat = CameraMatrix::perspective(horizontal_fov, aspectInverse, 1000.0f, 0.1f);
    CameraMatrix mProjectView = projMat * pCameraController->getViewMatrix();

    sphereUniform.projectView = mProjectView;
    for (int i = 0; i < MAX_SPHERE; i++)
    {
        position[i].setZ(position[i].getZ() + deltaTime * 100.0f);
        if (position[i].getZ() > 100)
        {
            position[i].setX(randomFloat(-100, 100));
            position[i].setY(randomFloat(-100, 100));
            position[i].setZ(randomFloat(-100, 100));

            color[i] = {randomFloat01(), randomFloat01(), randomFloat01(), 1.0};
        }
        sphereUniform.color[i] = color[i];
        sphereUniform.world[i] = mat4(0).identity().translation(position[i]);
    }
    sphereUniform.lightColor = lightColor;
    sphereUniform.lightPosition = lightPosition;

    quadUniform.projectView = mProjectView;
    quadUniform.color = {1.0f, 1.0f, 1.0f, 1.0f};
    quadUniform.world = mat4::translation({0, -100, 0}) * mat4::rotationX(degToRad(-90)) * mat4::scale({200, 200, 200});
}

void DemoScene::Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget)
{
    BindRenderTargetsDesc bindRenderTargets = {
        .mRenderTargetCount = 1,
        .mRenderTargets =
            {
                {pRenderTarget, LOAD_ACTION_CLEAR},
            },
        .mDepthStencil = {pDepthBuffer, LOAD_ACTION_CLEAR},
    };

    cmdBindRenderTargets(pCmd, &bindRenderTargets);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdBindPipeline(pCmd, pPipelineSphere);
    cmdBindDescriptorSet(pCmd, 0, pDSSphereUniform);

    constexpr uint32_t stride = sizeof(float) * 6;
    cmdBindVertexBuffer(pCmd, 1, &pBufferSphereVertex, &stride, nullptr);

    cmdDrawInstanced(pCmd, spherePoints / 6, 0, MAX_SPHERE, 0);

    cmdBindPipeline(pCmd, pPipelineQuad);
    cmdBindDescriptorSet(pCmd, 0, pDSQuadUniform);
    cmdBindVertexBuffer(pCmd, 1, &pBufferQuadVertex, &stride, nullptr);
    cmdBindIndexBuffer(pCmd, pBufferQuadIndex, INDEX_TYPE_UINT16, 0);
    cmdDrawIndexed(pCmd, 6, 0, 0);
}

void DemoScene::PreDraw()
{
    BufferUpdateDesc sphereUniformUpdateDesc = {pBufferSphereUniform};
    beginUpdateResource(&sphereUniformUpdateDesc);
    *(SphereUniformBlock *)sphereUniformUpdateDesc.pMappedData = sphereUniform;
    endUpdateResource(&sphereUniformUpdateDesc);

    BufferUpdateDesc quadUniformUpdateDesc = {pBufferQuadUniform};
    beginUpdateResource(&quadUniformUpdateDesc);
    *(QuadUniformBlock *)quadUniformUpdateDesc.pMappedData = quadUniform;
    endUpdateResource(&quadUniformUpdateDesc);
}
