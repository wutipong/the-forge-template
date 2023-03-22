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

    generateBonePoints(&vertices, &boneVertexCount, 0.25f);
    bufferLoadDesc = {};
    bufferLoadDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    bufferLoadDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    bufferLoadDesc.mDesc.mSize = boneVertexCount * sizeof(float);
    bufferLoadDesc.pData = vertices;
    bufferLoadDesc.ppBuffer = &pBoneVertexBuffer;
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

    ResetLightSettings();

    objectTypes[0] = ObjectType::Cube;
    objects[0].Color = {1.0f, 1.0f, 1.0f, 1.0f};
    objects[0].Transform = mat4::translation({0.0f, -2.0f, 0.0f}) * mat4::scale(vec3{1000.0f, 1.0f, 1000.0f});

    objectTypes[1] = ObjectType::Sphere;
    objects[1].Color = {1.0f, 0.0f, 0.0f, 1.0f};
    objects[1].Transform = mat4::translation({0.0f, 0.0f, 0.0f});

    objectTypes[2] = ObjectType::Cube;
    objects[2].Color = {0.0f, 0.70f, 0.4f, 1.0f};
    objects[2].Transform = mat4::translation({4.0f, 0.0f, 0.0f}) * mat4::rotationZ(0.75f * PI) *
        mat4::rotationX(0.75f * PI) * mat4::scale(vec3{3.0f});

    pCameraController = initFpsCameraController({0, 0.0f, -5.0f}, {0, 0, 0});

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

    desc = {DefaultInputActions::DefaultInputActions::RESET_CAMERA,
            [](InputActionContext *ctx) -> bool
            { return static_cast<Demo2Scene *>(ctx->pUserData)->OnInputAction(ctx); },
            this};

    addInputAction(&desc);

    UIComponentDesc guiDesc = {};
    uiCreateComponent("Objects", &guiDesc, &pObjectWindow);

    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        char str[] = "Object 0 Color";
        sprintf(str, "Object %d Color", i);

        ColorPickerWidget colorPickerWidget;
        colorPickerWidget.pData = &objects[i].Color;
        uiCreateComponentWidget(pObjectWindow, str, &colorPickerWidget, WIDGET_TYPE_COLOR_PICKER);
    }

    guiDesc = {};
    uiCreateComponent("Lighting", &guiDesc, &pObjectWindow);

    ButtonWidget resetBtnWidget = {};
    UIWidget *resetBtn = uiCreateComponentWidget(pObjectWindow, "Reset", &resetBtnWidget, WIDGET_TYPE_BUTTON);
    uiSetWidgetOnEditedCallback(resetBtn, this,
                                [](void *pUserData)
                                {
                                    auto *instance = reinterpret_cast<Demo2Scene *>(pUserData);
                                    instance->ResetLightSettings();
                                });

    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
        {
            char label[] = "Light 0 Color";
            sprintf(label, "Light %d Color", i);

            ColorPickerWidget widget;
            widget.pData = &scene.LightColor[i];
            uiCreateComponentWidget(pObjectWindow, label, &widget, WIDGET_TYPE_COLOR_PICKER);
        }

        {
            char label[] = "Light 0 Direction";
            sprintf(label, "Light %d Direction", i);

            SliderFloat4Widget widget;
            widget.pData = &scene.LightDirection[i];
            widget.mMax = float4{
                1.0f,
                1.0f,
                1.0f,
                1.0f,
            };
            widget.mMin = float4{
                -1.0f,
                -1.0f,
                -1.0f,
                -1.0f,
            };
            widget.mStep = float4{
                0.01f,
                0.01f,
                0.01f,
                0.01f,
            };
            uiCreateComponentWidget(pObjectWindow, label, &widget, WIDGET_TYPE_SLIDER_FLOAT4);
        }

        {
            char label[] = "Light 0 Ambient";
            sprintf(label, "Light %d Ambient", i);

            SliderFloatWidget widget;
            widget.pData = &scene.LightAmbient[i].x;
            widget.mMax = 1.0f;
            widget.mMin = 0.0f;
            widget.mStep = 0.1f;
            uiCreateComponentWidget(pObjectWindow, label, &widget, WIDGET_TYPE_SLIDER_FLOAT);
        }

        {
            char label[] = "Light 0 Diffuse";
            sprintf(label, "Light %d Diffuse", i);

            SliderFloatWidget widget;
            widget.pData = &scene.LightIntensity[i].x;
            widget.mMax = 1.0f;
            widget.mMin = 0.0f;
            widget.mStep = 0.1f;
            uiCreateComponentWidget(pObjectWindow, label, &widget, WIDGET_TYPE_SLIDER_FLOAT);
        }
    }
}

void Demo2Scene::ResetLightSettings()
{
    scene.LightDirection[0] = {0.5f, -0.25f, -0.5f, 1.0f};
    scene.LightColor[0] = {1.0f, 0.5f, 0.25f, 0.4f};
    scene.LightAmbient[0].x = 0.1f;
    scene.LightIntensity[0].x = 0.4f;

    scene.LightDirection[1] = {-1.0f, -0.5f, 0.0f, 1.0f};
    scene.LightColor[1] = {0.0f, 0.5f, 0.75f, 0.4f};
    scene.LightAmbient[1].x = 0.1f;
    scene.LightIntensity[1].x = 0.4f;
}

void Demo2Scene::Exit()
{
    uiDestroyComponent(pObjectWindow);

    removeResource(pCubeVertexBuffer);
    removeResource(pSphereVertexBuffer);
    removeResource(pBoneVertexBuffer);

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
        ShaderLoadDesc shaderLoadDesc = {};
        shaderLoadDesc.mStages[0] = {"demo2_object.vert", nullptr, 0, nullptr,
                                     SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW};
        shaderLoadDesc.mStages[1] = {"demo2_object.frag", nullptr, 0};

        addShader(pRenderer, &shaderLoadDesc, &pObjectShader);

        shaderLoadDesc = {};
        shaderLoadDesc.mStages[0] = {"demo2_shadow.vert", nullptr, 0, nullptr,
                                     SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW};
        shaderLoadDesc.mStages[1] = {"demo2_shadow.frag", nullptr, 0};

        addShader(pRenderer, &shaderLoadDesc, &pShadowShader);

        Shader *pShaders[]{pObjectShader, pShadowShader};

        RootSignatureDesc rootDesc = {};
        rootDesc.mShaderCount = 2;
        rootDesc.ppShaders = pShaders;

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

        RasterizerStateDesc rasterizerStateDesc = {};
        rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;

        DepthStateDesc depthStateDesc = {};
        depthStateDesc.mDepthTest = true;
        depthStateDesc.mDepthWrite = true;
        depthStateDesc.mDepthFunc = CMP_GEQUAL;

        {
            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc &pipelineSettings = pipelineDesc.mGraphicsDesc;
            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
            pipelineSettings.mRenderTargetCount = 1;
            pipelineSettings.pDepthState = &depthStateDesc;
            pipelineSettings.pColorFormats = &pRenderTarget->mFormat;
            pipelineSettings.mSampleCount = pRenderTarget->mSampleCount;
            pipelineSettings.mSampleQuality = pRenderTarget->mSampleQuality;
            pipelineSettings.mDepthStencilFormat = pDepthBuffer->mFormat;
            pipelineSettings.pRootSignature = pRootSignature;
            pipelineSettings.pShaderProgram = pObjectShader;
            pipelineSettings.pVertexLayout = &vertexLayout;
            pipelineSettings.pRasterizerState = &rasterizerStateDesc;
            pipelineSettings.mVRFoveatedRendering = true;
            addPipeline(pRenderer, &pipelineDesc, &pObjectPipeline);
        }

        RenderTargetDesc renderTargetDesc = {};
        renderTargetDesc.mArraySize = 1;
        renderTargetDesc.mClearValue.depth = 1.0f;
        renderTargetDesc.mClearValue.stencil = 0;
        renderTargetDesc.mDepth = 1;
        renderTargetDesc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
        renderTargetDesc.mFormat = TinyImageFormat_D32_SFLOAT;
        renderTargetDesc.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
        renderTargetDesc.mHeight = SHADOW_MAP_DIMENSION;
        renderTargetDesc.mWidth = SHADOW_MAP_DIMENSION;
        renderTargetDesc.mSampleCount = SAMPLE_COUNT_1;
        renderTargetDesc.mSampleQuality = 0;
        renderTargetDesc.mFlags = TEXTURE_CREATION_FLAG_OWN_MEMORY_BIT;
        renderTargetDesc.pName = "Shadow Map Render Target";
        addRenderTarget(pRenderer, &renderTargetDesc, &pShadowRenderTarget);

        {
            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc &pipelineSettings = pipelineDesc.mGraphicsDesc;
            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
            pipelineSettings.mRenderTargetCount = 0;
            pipelineSettings.pDepthState = &depthStateDesc;
            pipelineSettings.mSampleCount = pShadowRenderTarget->mSampleCount;
            pipelineSettings.mSampleQuality = pShadowRenderTarget->mSampleQuality;
            pipelineSettings.mDepthStencilFormat = pDepthBuffer->mFormat;
            pipelineSettings.pRootSignature = pRootSignature;
            pipelineSettings.pShaderProgram = pShadowShader;
            pipelineSettings.pVertexLayout = &vertexLayout;
            pipelineSettings.pRasterizerState = &rasterizerStateDesc;
            pipelineSettings.mVRFoveatedRendering = true;

            addPipeline(pRenderer, &pipelineDesc, &pShadowPipeline);
        }
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
        removePipeline(pRenderer, pObjectPipeline);
        removePipeline(pRenderer, pShadowPipeline);
        removeRenderTarget(pRenderer, pShadowRenderTarget);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        removeDescriptorSet(pRenderer, pDescriptorSetSceneUniform);
        removeDescriptorSet(pRenderer, pDescriptorSetObjectUniform);

        removeRootSignature(pRenderer, pRootSignature);
        removeShader(pRenderer, pObjectShader);
        removeShader(pRenderer, pShadowShader);
    }
}

void Demo2Scene::Update(float deltaTime, uint32_t width, uint32_t height)
{
    pCameraController->update(deltaTime);

    const float aspectInverse = (float)height / (float)width;
    const float horizontal_fov = PI / 2.0f;
    CameraMatrix projection = CameraMatrix::perspective(horizontal_fov, aspectInverse, 1000.0f, 0.1f);
    CameraMatrix mProjectView = projection * pCameraController->getViewMatrix();

    scene.CameraPosition = {pCameraController->getViewPosition(), 1.0f};
    scene.ProjectView = mProjectView;

    for (auto &dir : scene.LightDirection)
    {
        dir = v4ToF4({normalize(f3Tov3(dir.getXYZ())), 1.0f});
    }

    Point3 lightPos = toPoint3(-f4Tov4(scene.LightDirection[0]) * SHADOW_MAP_DIMENSION);

    scene.ShadowTransform =
        mat4::orthographic(0, SHADOW_MAP_DIMENSION, 0, SHADOW_MAP_DIMENSION, 0, SHADOW_MAP_DIMENSION) *
        mat4::lookAt(lightPos, {0, 0, 0}, {0, 1, 0});
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

    cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);

    RenderTargetBarrier barriers[] = {
        {pShadowRenderTarget, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_DEPTH_WRITE}};
    cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, 1, barriers);

    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_DONTCARE;
    loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
    loadActions.mClearDepth.depth = 1.0f;
    loadActions.mClearDepth.stencil = 0;

    cmdBindRenderTargets(pCmd, 0, nullptr, pShadowRenderTarget, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pShadowRenderTarget->mWidth, (float)pShadowRenderTarget->mHeight, 0.0f,
                   1.0f);
    cmdSetScissor(pCmd, 0, 0, pShadowRenderTarget->mWidth, pShadowRenderTarget->mHeight);
    cmdBindPipeline(pCmd, pShadowPipeline);

    cmdBindPipeline(pCmd, pShadowPipeline);

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

        case ObjectType::Bone:
            cmdBindVertexBuffer(pCmd, 1, &pBoneVertexBuffer, &stride, nullptr);
            vertexCount = boneVertexCount;
            break;

        default:
            continue;
        }

        cmdDraw(pCmd, vertexCount / 6, 0);
    }

    cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
    barriers[0] = {pShadowRenderTarget, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_SHADER_RESOURCE};
    cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, 1, barriers);

    loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
    loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
    loadActions.mClearDepth.depth = 0.0f;

    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdBindPipeline(pCmd, pObjectPipeline);

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

        case ObjectType::Bone:
            cmdBindVertexBuffer(pCmd, 1, &pBoneVertexBuffer, &stride, nullptr);
            vertexCount = boneVertexCount;
            break;

        default:
            continue;
        }

        cmdDraw(pCmd, vertexCount / 6, 0);
    }
}

bool Demo2Scene::OnInputAction(InputActionContext *ctx)
{
    if ((*ctx->pCaptured))
    {
        if (uiIsFocused())
        {
            return true;
        }

        switch (ctx->mActionId)
        {
        case DefaultInputActions::ROTATE_CAMERA:
            pCameraController->onRotate(ctx->mFloat2);
            break;

        case DefaultInputActions::TRANSLATE_CAMERA:
            pCameraController->onMove(ctx->mFloat2);
            break;

        case DefaultInputActions::RESET_CAMERA:
            pCameraController->resetView();
            ResetLightSettings();
            break;
        }
    }

    return true;
}
