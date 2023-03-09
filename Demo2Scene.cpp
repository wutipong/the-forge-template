//
// Created by mr_ta on 3/4/2023.
//

#include "Demo2Scene.h"
#include "IInput.h"
#include "IResourceLoader.h"
#include "IUI.h"
#include "stb_ds.h"

void Demo2Scene::Init(uint32_t imageCount)
{
    float *vertices{};

    generateCuboidPoints(&vertices, &cubeVertexCount);
    BufferLoadDesc bufferLoadDesc = {};
    bufferLoadDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    bufferLoadDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    bufferLoadDesc.mDesc.mSize = cubeVertexCount * sizeof(float);
    bufferLoadDesc.pData = vertices;
    bufferLoadDesc.ppBuffer = &pCubeVertexBuffer;
    addResource(&bufferLoadDesc, nullptr);

    tf_free(vertices);

    generateSpherePoints(&vertices, &sphereVertexCount, 64);
    bufferLoadDesc = {};
    bufferLoadDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    bufferLoadDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    bufferLoadDesc.mDesc.mSize = sphereVertexCount * sizeof(float);
    bufferLoadDesc.pData = vertices;
    bufferLoadDesc.ppBuffer = &pSphereVertexBuffer;
    addResource(&bufferLoadDesc, nullptr);

    tf_free(vertices);

    BufferLoadDesc ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(ObjectUniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;

    arrsetlen(pObjectUniformBuffers, imageCount * OBJECT_COUNT);
    for (uint32_t i = 0; i < imageCount * OBJECT_COUNT; ++i)
    {
        ubDesc.ppBuffer = &pObjectUniformBuffers[i];
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

    scene.AmbientLight = {1.0f, 0.05f, 1.0f, 0.05f};

    scene.DirectionalLightDirection[0] = vec4{0.5f, -0.25f, -0.5f, 1.0f};
    scene.DirectionalLightColor[0] = {1.0f, 0.5f, 0.25f, 0.2f};

    scene.DirectionalLightDirection[1] = vec4{-10.0f, -0.65f, -0.5f, 1.0f};
    scene.DirectionalLightColor[1] = {0.0f, 0.5f, 0.75f, 0.1f};

    objectTypes[0] = ObjectType::Cube;
    objects[0].Color = {1.0f, 1.0f, 1.0f, 1.0f};
    objects[0].Transform = mat4::translation({0.0f, 0.0f, 2.0f}) * mat4::scale(vec3{100.0f, 100.0f, 1.0f});

    objectTypes[1] = ObjectType::Sphere;
    objects[1].Color = {1.0f, 0.0f, 0.0f, 1.0f};
    objects[1].Transform = mat4::translation({0.0f, 0.0f, 0.0f});

    objectTypes[2] = ObjectType::Cube;
    objects[2].Color = {0.0f, 0.70f, 0.4f, 1.0f};
    objects[2].Transform =
        mat4::translation({3.0f, 0.0f, 0.0f}) * mat4::rotationZ(0.75f * PI) * mat4::scale(vec3{3.0f});

    pCameraController = initFpsCameraController({0, -5.0f, -2}, {0, 0, 0});

    InputActionDesc desc{DefaultInputActions::ROTATE_CAMERA,
                         [](InputActionContext *ctx) -> bool
                         { return static_cast<Demo2Scene *>(ctx->pUserData)->OnInputAction(ctx); },
                         this};

    addInputAction(&desc);

    desc = {DefaultInputActions::DefaultInputActions::TRANSLATE_CAMERA,
            [](InputActionContext *ctx) -> bool
            { return static_cast<Demo2Scene *>(ctx->pUserData)->OnInputAction(ctx); },
            this};

    addInputAction(&desc);
}

void Demo2Scene::Exit()
{
    removeResource(pCubeVertexBuffer);
    removeResource(pSphereVertexBuffer);

    for (int i = 0; i < arrlen(pObjectUniformBuffers); i++)
    {
        removeResource(pObjectUniformBuffers[i]);
    }
    tf_free(pObjectUniformBuffers);

    for (int i = 0; i < arrlen(pSceneUniformBuffer); i++)
    {
        removeResource(pSceneUniformBuffer[i]);
    }
    tf_free(pSceneUniformBuffer);

    exitCameraController(pCameraController);
}
void Demo2Scene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget,
                      RenderTarget *pDepthBuffer, uint32_t imageCount)
{
    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        ShaderLoadDesc basicShader = {};
        basicShader.mStages[0] = {"demo2_object.vert", nullptr, 0, nullptr, SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW};
        basicShader.mStages[1] = {"demo2_object.frag", nullptr, 0};

        addShader(pRenderer, &basicShader, &pShader);

        RootSignatureDesc rootDesc = {};
        rootDesc.mShaderCount = 1;
        rootDesc.ppShaders = &pShader;

        addRootSignature(pRenderer, &rootDesc, &pRootSignature);

        DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, imageCount};
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetSceneUniform);

        desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_DRAW, static_cast<uint32_t>(imageCount * OBJECT_COUNT)};
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetObjectUniform);
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

    for (int i = 0; i < imageCount * OBJECT_COUNT; i++)
    {
        DescriptorData params = {};
        params.pName = "uniformObjectBlock";
        params.ppBuffers = &pObjectUniformBuffers[i];
        updateDescriptorSet(pRenderer, i, pDescriptorSetObjectUniform, 1, &params);
    }

    for (int i = 0; i < imageCount; i++)
    {
        DescriptorData params = {};
        params.pName = "uniformSceneBlock";
        params.ppBuffers = &pSceneUniformBuffer[i];

        updateDescriptorSet(pRenderer, i, pDescriptorSetSceneUniform, 1, &params);
    }
}
void Demo2Scene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(pRenderer, pPipeline);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        removeDescriptorSet(pRenderer, pDescriptorSetSceneUniform);
        removeDescriptorSet(pRenderer, pDescriptorSetObjectUniform);

        removeRootSignature(pRenderer, pRootSignature);
        removeShader(pRenderer, pShader);
    }
}

void Demo2Scene::Update(float deltaTime, uint32_t width, uint32_t height)
{
    pCameraController->update(deltaTime);

    const float aspectInverse = (float)height / (float)width;
    const float horizontal_fov = PI / 2.0f;
    CameraMatrix projection = CameraMatrix::perspective(horizontal_fov, aspectInverse, 1000.0f, 0.1f);
    CameraMatrix mProjectView = projection * pCameraController->getViewMatrix();

    scene.mProjectView = mProjectView;
}

void Demo2Scene::PreDraw(uint32_t frameIndex)
{
    BufferUpdateDesc updateDesc = {pSceneUniformBuffer[frameIndex]};
    beginUpdateResource(&updateDesc);
    *(SceneUniformBlock *)updateDesc.pMappedData = scene;
    endUpdateResource(&updateDesc, nullptr);

    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        BufferUpdateDesc updateDesc = {pObjectUniformBuffers[frameIndex * OBJECT_COUNT + i]};
        beginUpdateResource(&updateDesc);
        *(ObjectUniformBlock *)updateDesc.pMappedData = objects[i];
        endUpdateResource(&updateDesc, nullptr);
    }
}
void Demo2Scene::Draw(Cmd *pCmd, RenderTarget *pRenderTarget, RenderTarget *pDepthBuffer, uint32_t frameIndex)
{
    constexpr uint32_t stride = sizeof(float) * 6;

    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
    loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
    loadActions.mClearDepth.depth = 0.0f;
    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);

    cmdBindPipeline(pCmd, pPipeline);

    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        cmdBindDescriptorSet(pCmd, (frameIndex * OBJECT_COUNT) + i, pDescriptorSetObjectUniform);
        cmdBindDescriptorSet(pCmd, frameIndex, pDescriptorSetSceneUniform);

        int vertexCount = 0;

        switch (objectTypes[i])
        {
        case ObjectType::Cube:
            cmdBindVertexBuffer(pCmd, 1, &pCubeVertexBuffer, &stride, nullptr);
            vertexCount = cubeVertexCount;
            break;

        case ObjectType::Sphere:
            cmdBindVertexBuffer(pCmd, 1, &pSphereVertexBuffer, &stride, nullptr);
            vertexCount = sphereVertexCount;
            break;

        default:
            continue;
        }

        cmdDraw(pCmd, vertexCount / 6, 0);
    }
}
bool Demo2Scene::OnInputAction(InputActionContext *ctx)
{
    if (uiIsFocused())
        return false;

    switch (ctx->mActionId)
    {
    case DefaultInputActions::ROTATE_CAMERA:
        pCameraController->onRotate(ctx->mFloat2);
        break;

    case DefaultInputActions ::TRANSLATE_CAMERA:
        pCameraController->onMove(ctx->mFloat2);
        break;
    }
    return true;
}
