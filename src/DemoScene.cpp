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
    int vertexCount{};

    constexpr size_t MAX_STARS = 768;
    std::array<vec3, MAX_STARS> position{};
    std::array<vec4, MAX_STARS> color{};
    vec3 lightPosition{1.0f, 0, 0};
    vec3 lightColor{0.9f, 0.9f, 0.7f};

    struct UniformBlock
    {
        CameraMatrix mProjectView;
        std::array<mat4, MAX_STARS> mToWorldMat;
        std::array<vec4, MAX_STARS> mColor;

        vec3 mLightPosition;
        vec3 mLightColor;
    } uniform = {};

    Shader *pShader{nullptr};
    RootSignature *pRootSignature{nullptr};
    DescriptorSet *pDSUniform{nullptr};
    Buffer *pProjViewUniformBuffer{nullptr};
    Buffer *pSphereVertexBuffer{nullptr};
    Pipeline *pPLSphere{nullptr};

    ICameraController *pCameraController{nullptr};

    RenderTarget *pDepthBuffer{nullptr};
    TinyImageFormat depthBufferFormat = TinyImageFormat_D32_SFLOAT;
} // namespace DemoScene

bool DemoScene::Init()
{
    float *vertices{};
    generateSpherePoints(&vertices, &vertexCount, 24, 1.0f);

    SyncToken token{};

    uint64_t sphereDataSize = vertexCount * sizeof(float);
    BufferLoadDesc sphereVbDesc{
        .ppBuffer = &pSphereVertexBuffer,
        .pData = vertices,
        .mDesc{
            .mSize = sphereDataSize,
            .mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY,
            .mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER,
        },
    };
    addResource(&sphereVbDesc, &token);

    BufferLoadDesc ubDesc = {
        .ppBuffer = &pProjViewUniformBuffer,
        .mDesc{
            .mSize = sizeof(UniformBlock),
            .mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
            .mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT,
            .mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        },
    };
    addResource(&ubDesc, &token);

    for (size_t i = 0; i < MAX_STARS; i++)
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
    tf_free(vertices);

    return true;
}

void DemoScene::Exit()
{
    removeResource(pProjViewUniformBuffer);
    removeResource(pSphereVertexBuffer);

    exitCameraController(pCameraController);
}

bool DemoScene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget)
{
    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        ShaderLoadDesc basicShader{
            .mStages{
                {
                    .pFileName = "basic.vert",
                },
                {
                    .pFileName = "basic.frag",
                },
            },
        };

        addShader(pRenderer, &basicShader, &pShader);
        ASSERT(pShader);

        RootSignatureDesc rootDesc{
            .ppShaders = &pShader,
            .mShaderCount = 1,
        };

        addRootSignature(pRenderer, &rootDesc, &pRootSignature);
        ASSERT(pRootSignature);

        DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1};
        addDescriptorSet(pRenderer, &desc, &pDSUniform);

        ASSERT(pDSUniform);
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
        sphereRasterizerStateDesc.mCullMode = CULL_MODE_FRONT;

        DepthStateDesc depthStateDesc{
            .mDepthTest = true,
            .mDepthWrite = true,
            .mDepthFunc = CMP_GEQUAL,
        };

        PipelineDesc desc = {
            .mGraphicsDesc{
                .pShaderProgram = pShader,
                .pRootSignature = pRootSignature,
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

        GraphicsPipelineDesc &pipelineSettings = desc.mGraphicsDesc;

        addPipeline(pRenderer, &desc, &pPLSphere);
        ASSERT(pPLSphere);
    }

    DescriptorData params = {
        .pName = "uniformBlock",
        .ppBuffers = &pProjViewUniformBuffer,
    };

    updateDescriptorSet(pRenderer, 0, pDSUniform, 1, &params);

    return true;
}

void DemoScene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(pRenderer, pPLSphere);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        removeDescriptorSet(pRenderer, pDSUniform);
        removeRootSignature(pRenderer, pRootSignature);
        removeShader(pRenderer, pShader);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        removeRenderTarget(pRenderer, pDepthBuffer);
    }
}

void DemoScene::Update(float deltaTime, uint32_t width, uint32_t height)
{
    pCameraController->update(deltaTime);
    const float aspectInverse = (float)height / (float)width;
    const float horizontal_fov = PI / 2.0f;
    CameraMatrix projMat = CameraMatrix::perspective(horizontal_fov, aspectInverse, 1000.0f, 0.1f);
    CameraMatrix mProjectView = projMat * pCameraController->getViewMatrix();

    uniform.mProjectView = mProjectView;
    for (int i = 0; i < MAX_STARS; i++)
    {
        position[i].setZ(position[i].getZ() + deltaTime * 100.0f);
        if (position[i].getZ() > 100)
        {
            position[i].setX(randomFloat(-100, 100));
            position[i].setY(randomFloat(-100, 100));
            position[i].setZ(randomFloat(-100, 100));

            color[i] = {randomFloat01(), randomFloat01(), randomFloat01(), 1.0};
        }
        uniform.mColor[i] = color[i];
        uniform.mToWorldMat[i] = mat4(0).identity().translation(position[i]);
    }
    uniform.mLightColor = lightColor;
    uniform.mLightPosition = lightPosition;
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

    cmdBindPipeline(pCmd, pPLSphere);
    cmdBindDescriptorSet(pCmd, 0, pDSUniform);

    constexpr uint32_t sphereVbStride = sizeof(float) * 6;
    cmdBindVertexBuffer(pCmd, 1, &pSphereVertexBuffer, &sphereVbStride, nullptr);

    cmdDrawInstanced(pCmd, vertexCount / 6, 0, MAX_STARS, 0);
}

void DemoScene::PreDraw()
{
    BufferUpdateDesc viewProjCbv = {pProjViewUniformBuffer};
    beginUpdateResource(&viewProjCbv);
    *(UniformBlock *)viewProjCbv.pMappedData = uniform;
    endUpdateResource(&viewProjCbv);
}
