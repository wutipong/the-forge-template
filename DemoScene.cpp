#include "DemoScene.h"
#include <IResourceLoader.h>
#include <stb_ds.h>

void DemoScene::Init(uint32_t imageCount)
{
    float *vertices{};
    generateSpherePoints(&vertices, &vertexCount, 24, 1.0f);

    uint64_t sphereDataSize = vertexCount * sizeof(float);
    BufferLoadDesc sphereVbDesc = {};
    sphereVbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    sphereVbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    sphereVbDesc.mDesc.mSize = sphereDataSize;
    sphereVbDesc.pData = vertices;
    sphereVbDesc.ppBuffer = &pSphereVertexBuffer;
    addResource(&sphereVbDesc, nullptr);

    tf_free(vertices);

    arrsetlen(pProjViewUniformBuffer, imageCount);

    BufferLoadDesc ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(UniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        ubDesc.ppBuffer = &pProjViewUniformBuffer[i];
        addResource(&ubDesc, nullptr);
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
}

void DemoScene::Exit()
{
    for (int i = 0; i < arrlen(pProjViewUniformBuffer); i++)
    {
        removeResource(pProjViewUniformBuffer[i]);
    }

    arrfree(pProjViewUniformBuffer);

    removeResource(pSphereVertexBuffer);

    exitCameraController(pCameraController);
}

void DemoScene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
                    RenderTarget *pDepthBuffer, uint32_t imageCount)
{
    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        ShaderLoadDesc basicShader = {};
        basicShader.mStages[0] = {"basic.vert", nullptr, 0, nullptr, SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW};
        basicShader.mStages[1] = {"basic.frag", nullptr, 0};

        addShader(pRenderer, &basicShader, &pShader);

        RootSignatureDesc rootDesc = {};
        rootDesc.mShaderCount = 1;
        rootDesc.ppShaders = &pShader;

        addRootSignature(pRenderer, &rootDesc, &pRootSignature);

        DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, imageCount};
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetUniforms);
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
        pipelineSettings.mDepthStencilFormat = pDepthBuffer->mFormat;
        pipelineSettings.pRootSignature = pRootSignature;
        pipelineSettings.pShaderProgram = pShader;
        pipelineSettings.pVertexLayout = &vertexLayout;
        pipelineSettings.pRasterizerState = &sphereRasterizerStateDesc;
        pipelineSettings.mVRFoveatedRendering = true;
        addPipeline(pRenderer, &desc, &pSpherePipeline);
    }

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        DescriptorData params = {};
        params.pName = "uniformBlock";
        params.ppBuffers = &pProjViewUniformBuffer[i];

        updateDescriptorSet(pRenderer, i, pDescriptorSetUniforms, 1, &params);
    }
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

void DemoScene::Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex)
{
    constexpr uint32_t sphereVbStride = sizeof(float) * 6;

    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
    loadActions.mClearDepth.depth = 0.0f;
    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);

    cmdBindPipeline(pCmd, pSpherePipeline);
    cmdBindDescriptorSet(pCmd, frameIndex, pDescriptorSetUniforms);
    cmdBindVertexBuffer(pCmd, 1, &pSphereVertexBuffer, &sphereVbStride, nullptr);

    // cmdDraw(pCmd, vertexCount / 6, 1);
    cmdDrawInstanced(pCmd, vertexCount / 6, 0, MAX_STARS, 0);
}

void DemoScene::PreDraw(uint32_t frameIndex)
{
    BufferUpdateDesc viewProjCbv = {pProjViewUniformBuffer[frameIndex]};
    beginUpdateResource(&viewProjCbv);
    *(UniformBlock *)viewProjCbv.pMappedData = uniform;
    endUpdateResource(&viewProjCbv, nullptr);
}
