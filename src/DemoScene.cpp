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
    vec3 position[MAX_STARS]{};
    vec4 color[MAX_STARS]{};
    vec3 lightPosition{1.0f, 0, 0};
    vec3 lightColor{0.9f, 0.9f, 0.7f};

    struct UniformBlock
    {
        CameraMatrix mProjectView;
        mat4 mToWorldMat[MAX_STARS];
        vec4 mColor[MAX_STARS];

        vec3 mLightPosition;
        vec3 mLightColor;
    } uniform = {};

    Shader *pShader{nullptr};
    RootSignature *pRootSignature{nullptr};
    DescriptorSet *pDescriptorSetUniforms{nullptr};
    std::array<Buffer *, IMAGE_COUNT> pProjViewUniformBuffers{nullptr};
    Buffer *pSphereVertexBuffer{nullptr};
    Pipeline *pSpherePipeline{nullptr};

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
    BufferLoadDesc sphereVbDesc = {};
    sphereVbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    sphereVbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    sphereVbDesc.mDesc.mSize = sphereDataSize;
    sphereVbDesc.pData = vertices;
    sphereVbDesc.ppBuffer = &pSphereVertexBuffer;
    addResource(&sphereVbDesc, &token);

    BufferLoadDesc ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(UniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;

    for (auto &buffer : pProjViewUniformBuffers)
    {
        ubDesc.ppBuffer = &buffer;
        addResource(&ubDesc, &token);
    }

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
    for (auto &buffer : pProjViewUniformBuffers)
    {
        removeResource(buffer);
    }

    removeResource(pSphereVertexBuffer);

    exitCameraController(pCameraController);
}

bool DemoScene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget)
{
    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        ShaderLoadDesc basicShader = {};
        basicShader.mStages[0] = {"basic.vert"};
        basicShader.mStages[1] = {"basic.frag"};

        addShader(pRenderer, &basicShader, &pShader);
        ASSERT(pShader);

        RootSignatureDesc rootDesc = {};
        rootDesc.mShaderCount = 1;
        rootDesc.ppShaders = &pShader;

        addRootSignature(pRenderer, &rootDesc, &pRootSignature);
        ASSERT(pRootSignature);

        DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, IMAGE_COUNT};
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetUniforms);

        ASSERT(pDescriptorSetUniforms);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        RenderTargetDesc desc = {};
        desc.mArraySize = 1;
        desc.mClearValue.depth = 0.0f;
        desc.mClearValue.stencil = 0;
        desc.mDepth = 1;
        desc.mFlags = TEXTURE_CREATION_FLAG_ON_TILE | TEXTURE_CREATION_FLAG_VR_MULTIVIEW;
        desc.mFormat = depthBufferFormat;
        desc.mWidth = pRenderTarget->mWidth;
        desc.mHeight = pRenderTarget->mHeight;
        desc.mSampleCount = SAMPLE_COUNT_1;
        desc.mSampleQuality = 0;
        desc.mStartState = RESOURCE_STATE_DEPTH_WRITE;
        addRenderTarget(pRenderer, &desc, &pDepthBuffer);
        ASSERT(pDepthBuffer);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
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
        vertexLayout.mBindings[0] = {
            .mStride = sizeof(float) * 6,
        };

        RasterizerStateDesc sphereRasterizerStateDesc = {};
        sphereRasterizerStateDesc.mCullMode = CULL_MODE_FRONT;

        DepthStateDesc depthStateDesc = {};
        depthStateDesc.mDepthTest = true;
        depthStateDesc.mDepthWrite = true;
        depthStateDesc.mDepthFunc = CMP_GEQUAL;

        PipelineDesc desc = {};
        desc.mType = PIPELINE_TYPE_GRAPHICS;
        GraphicsPipelineDesc &pipelineSettings = desc.mGraphicsDesc;
        pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        pipelineSettings.mRenderTargetCount = 1;
        pipelineSettings.pDepthState = &depthStateDesc;
        pipelineSettings.pColorFormats = &pRenderTarget->mFormat;
        pipelineSettings.mSampleCount = pRenderTarget->mSampleCount;
        pipelineSettings.mSampleQuality = pRenderTarget->mSampleQuality;
        pipelineSettings.mDepthStencilFormat = depthBufferFormat;
        pipelineSettings.pRootSignature = pRootSignature;
        pipelineSettings.pShaderProgram = pShader;
        pipelineSettings.pVertexLayout = &vertexLayout;
        pipelineSettings.pRasterizerState = &sphereRasterizerStateDesc;
        pipelineSettings.mVRFoveatedRendering = true;
        addPipeline(pRenderer, &desc, &pSpherePipeline);
        ASSERT(pSpherePipeline);
    }

    for (uint32_t i = 0; i < pProjViewUniformBuffers.size(); ++i)
    {
        DescriptorData params = {};
        params.pName = "uniformBlock";
        params.ppBuffers = &pProjViewUniformBuffers[i];

        updateDescriptorSet(pRenderer, i, pDescriptorSetUniforms, 1, &params);
    }

    return true;
}

void DemoScene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(pRenderer, pSpherePipeline);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        removeDescriptorSet(pRenderer, pDescriptorSetUniforms);
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
            position[i].setZ(randomFloat(-100, 100));
        }
        uniform.mColor[i] = color[i];
        uniform.mToWorldMat[i] = mat4(0).identity().translation(position[i]);
    }
    uniform.mLightColor = lightColor;
    uniform.mLightPosition = lightPosition;
}

void DemoScene::Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageIndex)
{
    BindRenderTargetsDesc bindRenderTargets = {};
    bindRenderTargets.mRenderTargetCount = 1;
    bindRenderTargets.mRenderTargets[0] = {pRenderTarget, LOAD_ACTION_CLEAR};
    bindRenderTargets.mDepthStencil = {pDepthBuffer, LOAD_ACTION_CLEAR};

    cmdBindRenderTargets(pCmd, &bindRenderTargets);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdBindPipeline(pCmd, pSpherePipeline);
    cmdBindDescriptorSet(pCmd, imageIndex, pDescriptorSetUniforms);

    constexpr uint32_t sphereVbStride = sizeof(float) * 6;
    cmdBindVertexBuffer(pCmd, 1, &pSphereVertexBuffer, &sphereVbStride, nullptr);

    cmdDrawInstanced(pCmd, vertexCount / 6, 0, MAX_STARS, 0);
}

void DemoScene::PreDraw(uint32_t imageIndex)
{
    BufferUpdateDesc viewProjCbv = {pProjViewUniformBuffers[imageIndex]};
    beginUpdateResource(&viewProjCbv);
    *(UniformBlock *)viewProjCbv.pMappedData = uniform;
    endUpdateResource(&viewProjCbv);
}
