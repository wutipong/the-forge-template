//
// Created by mr_ta on 3/4/2023.
//

#include "Demo2Scene.h"
#include "IResourceLoader.h"
#include "stb_ds.h"

void Demo2Scene::Init(uint32_t imageCount)
{
    float *vertices{};

    generateCuboidPoints(&vertices, &cubeVertexCount);
    uint64_t sphereDataSize = cubeVertexCount * sizeof(float);
    BufferLoadDesc bufferLoadDesc = {};
    bufferLoadDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    bufferLoadDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    bufferLoadDesc.mDesc.mSize = sphereDataSize;
    bufferLoadDesc.pData = vertices;
    bufferLoadDesc.ppBuffer = &pCubeVertexBuffer;
    addResource(&bufferLoadDesc, nullptr);

    tf_free(vertices);

    arrsetlen(pCameraUniformBuffer, imageCount);

    pCameraController = initFpsCameraController({0, -100.0, 0}, {0, 0, 0});

    BufferLoadDesc ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(CameraUniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        ubDesc.ppBuffer = &pCameraUniformBuffer[i];
        addResource(&ubDesc, nullptr);
    }

    cubes[0].Color = {1.0f, 1.0f, 1.0f};
    cubes[0].Transform = mat4().identity().scale({10.0f, 10.0f, 10.0f}).translation({0.0f, 5.0f, 0.0f});

    cubes[1].Color = {1.0f, 0.0f, 0.0f};
    cubes[1].Transform = mat4().identity().translation({0.0f, 10.0f, 0.0f});

    ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(CubeUniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;

    arrsetlen(pCubeUniformBuffers, imageCount * CUBE_COUNT);
    for (uint32_t i = 0; i < imageCount * CUBE_COUNT; ++i)
    {
        ubDesc.ppBuffer = &pCubeUniformBuffers[i];
        addResource(&ubDesc, nullptr);
    }

    ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(SceneUniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;
    arrsetlen(pSceneUniformBuffer, imageCount);

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        ubDesc.ppBuffer = &pSceneUniformBuffer[i];
        addResource(&ubDesc, nullptr);
    }
}
void Demo2Scene::Exit()
{
    removeResource(pCubeVertexBuffer);

    for (int i = 0; i < arrlen(pCameraUniformBuffer); i++)
    {
        removeResource(pCameraUniformBuffer[i]);
    }
    tf_free(pCameraUniformBuffer);

    for (int i = 0; i < arrlen(pCubeUniformBuffers); i++)
    {
        removeResource(pCubeUniformBuffers[i]);
    }
    tf_free(pCubeUniformBuffers);

    for (int i = 0; i < arrlen(pSceneUniformBuffer); i++)
    {
        removeResource(pSceneUniformBuffer[i]);
    }
    tf_free(pSceneUniformBuffer);

    removeResource(pCubeVertexBuffer);

    exitCameraController(pCameraController);
}
void Demo2Scene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
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
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetCamera);
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetScene);

        desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_DRAW, static_cast<uint32_t>(imageCount * CUBE_COUNT)};
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetCube);
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
        addPipeline(pRenderer, &desc, &pPipeline);
    }

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        DescriptorData params = {};
        params.pName = "uniformCameraBlock";
        params.ppBuffers = &pCameraUniformBuffer[i];

        updateDescriptorSet(pRenderer, i, pDescriptorSetCamera, 1, &params);
    }
}
void Demo2Scene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer) {
    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(pRenderer, pPipeline);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        removeDescriptorSet(pRenderer, pDescriptorSetCamera);
        removeDescriptorSet(pRenderer, pDescriptorSetScene);
        removeDescriptorSet(pRenderer, pDescriptorSetCube);

        removeRootSignature(pRenderer, pRootSignature);
        removeShader(pRenderer, pShader);
    }
}
void Demo2Scene::Update(float deltaTime, uint32_t width, uint32_t height) { pCameraController->update(deltaTime); }
void Demo2Scene::PreDraw(uint32_t frameIndex) {
    BufferUpdateDesc updateDesc = {pCameraUniformBuffer[frameIndex]};
    beginUpdateResource(&updateDesc);
    *(CameraUniformBlock *)updateDesc.pMappedData = camera;
    endUpdateResource(&updateDesc, nullptr);

    updateDesc = {pSceneUniformBuffer[frameIndex]};
    beginUpdateResource(&updateDesc);
    *(SceneUniformBlock *)updateDesc.pMappedData = scene;
    endUpdateResource(&updateDesc, nullptr);
}
void Demo2Scene::Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex) {
    constexpr uint32_t sphereVbStride = sizeof(float) * 6;

    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
    loadActions.mClearDepth.depth = 0.0f;
    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);

    cmdBindPipeline(pCmd, pPipeline);
    cmdBindDescriptorSet(pCmd, frameIndex, pDescriptorSetCamera);
    cmdBindDescriptorSet(pCmd, frameIndex, pDescriptorSetScene);
    cmdBindDescriptorSet(pCmd, (frameIndex * CUBE_COUNT) + 0, pDescriptorSetCube);
    cmdBindVertexBuffer(pCmd, 1, &pCubeVertexBuffer, &sphereVbStride, nullptr);
    cmdDraw(pCmd, cubeVertexCount / 6, 0);

    cmdBindPipeline(pCmd, pPipeline);
    cmdBindDescriptorSet(pCmd, frameIndex, pDescriptorSetCamera);
    cmdBindDescriptorSet(pCmd, frameIndex, pDescriptorSetScene);
    cmdBindDescriptorSet(pCmd, (frameIndex * CUBE_COUNT) + 1, pDescriptorSetCube);
    cmdBindVertexBuffer(pCmd, 1, &pCubeVertexBuffer, &sphereVbStride, nullptr);
    cmdDraw(pCmd, cubeVertexCount / 6, 0);
}
