//
// Created by mr_ta on 3/4/2023.
//

#include "Demo2Scene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IInput.h>
#include <IResourceLoader.h>
#include <IUI.h>
#include <stb_ds.h>

#include "DrawShape.h"

namespace Demo2Scene
{
    Buffer **pUbObjects{};
    Buffer **pUbScene{};
    Buffer **pUbLightSources{};

    ICameraController *pCameraController{};

    Shader *pShObjects{};
    Shader *pShShadow{};
    Shader *pShLightSources{};
    Shader *pShShadowViewport{};

    RootSignature *pRootSignature{};

    DescriptorSet *pDsSceneUniform{};
    DescriptorSet *pDsObjectUniform{};
    DescriptorSet *pDsLightSourcesUniform{};
    DescriptorSet *pDsTexture{};

    Pipeline *pPlObjects{};
    Pipeline *pPlShadow{};
    Pipeline *pPlLightSources{};
    Pipeline *pPlShadowViewport{};

    struct ObjectUniformBlock
    {
        mat4 Transform;
        float4 Color;
    };

    constexpr size_t OBJECT_COUNT = 3;
    ObjectUniformBlock objects[OBJECT_COUNT]{};
    DrawShape::Shape objectTypes[OBJECT_COUNT]{};

    constexpr size_t DIRECTIONAL_LIGHT_COUNT = 2;
    struct SceneUniformBlock
    {
        // Camera
        vec4 CameraPosition;
        CameraMatrix ProjectView;

        // Directional Light;
        float4 LightDirection[DIRECTIONAL_LIGHT_COUNT];
        float4 LightColor[DIRECTIONAL_LIGHT_COUNT];
        float4 LightAmbient[DIRECTIONAL_LIGHT_COUNT];
        float4 LightIntensity[DIRECTIONAL_LIGHT_COUNT];

        // Shadow
        mat4 ShadowTransform;
    } scene{};

    ObjectUniformBlock lightSources[DIRECTIONAL_LIGHT_COUNT] = {};

    UIComponent *pObjectWindow{};

    constexpr float SHADOW_MAP_DIMENSION = 1024;
    constexpr float SHADOW_VIEWPORT = 512;

    RenderTarget *pRtShadow;

    void ResetLightSettings();
    void DrawShadowRT(Cmd *&pCmd, uint32_t frameIndex);
    void DrawShadowViewport(Cmd *&pCmd, RenderTarget *&pRenderTarget, uint32_t frameIndex);

    void InitUI();
    bool OnInputAction(InputActionContext *ctx);

    float3 cameraPosition;
    float3 lightPosition;

    RenderTarget *pDepthBuffer;
} // namespace Demo2Scene

void Demo2Scene::Init(uint32_t imageCount)
{
    DrawShape::Init();

    BufferLoadDesc ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(ObjectUniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;

    arrsetlen(pUbObjects, imageCount * OBJECT_COUNT);
    for (uint32_t i = 0; i < imageCount * OBJECT_COUNT; ++i)
    {
        ubDesc.ppBuffer = &pUbObjects[i];
        addResource(&ubDesc, nullptr);
    }

    ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(ObjectUniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;

    arrsetlen(pUbLightSources, imageCount * DIRECTIONAL_LIGHT_COUNT);
    for (uint32_t i = 0; i < imageCount * DIRECTIONAL_LIGHT_COUNT; ++i)
    {
        ubDesc.ppBuffer = &pUbLightSources[i];
        addResource(&ubDesc, nullptr);
    }

    ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mSize = sizeof(SceneUniformBlock);
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = nullptr;
    arrsetlen(pUbScene, imageCount);

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        ubDesc.ppBuffer = &pUbScene[i];
        addResource(&ubDesc, nullptr);
    }

    ResetLightSettings();

    objectTypes[0] = DrawShape::Shape::Cube;
    objects[0].Color = {1.0f, 1.0f, 1.0f, 1.0f};
    objects[0].Transform = mat4::translation({0.0f, -2.0f, 0.0f}) * mat4::scale(vec3{1000.0f, 1.0f, 1000.0f});

    objectTypes[1] = DrawShape::Shape::Sphere;
    objects[1].Color = {1.0f, 0.0f, 0.0f, 1.0f};
    objects[1].Transform = mat4::translation({0.0f, 0.0f, 0.0f});

    objectTypes[2] = DrawShape::Shape::Cube;
    objects[2].Color = {0.0f, 0.70f, 0.4f, 1.0f};
    objects[2].Transform = mat4::translation({4.0f, 0.0f, 0.0f}) * mat4::rotationZ(0.75f * PI) *
        mat4::rotationX(0.75f * PI) * mat4::scale(vec3{3.0f});

    pCameraController = initFpsCameraController({0, 0.0f, -5.0f}, {0, 0, 0});

    InputActionDesc desc{
        DefaultInputActions::ROTATE_CAMERA,
        OnInputAction,
    };

    addInputAction(&desc);

    desc = {
        DefaultInputActions::DefaultInputActions::TRANSLATE_CAMERA,
        OnInputAction,
    };

    addInputAction(&desc);

    desc = {
        DefaultInputActions::DefaultInputActions::RESET_CAMERA,
        OnInputAction,
    };

    addInputAction(&desc);

    InitUI();
}
void Demo2Scene::InitUI()
{
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
    uiSetWidgetOnEditedCallback(resetBtn, nullptr, [](void *pUserData) { ResetLightSettings(); });

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

    guiDesc = {};
    uiCreateComponent("Camera", &guiDesc, &pObjectWindow);

    SliderFloat3Widget cameraPosWidget{};
    cameraPosWidget.pData = &cameraPosition;
    uiCreateComponentWidget(pObjectWindow, "Position", &cameraPosWidget, WIDGET_TYPE_SLIDER_FLOAT3);

    ButtonWidget moveToLightSrcBtnWidget = {};
    UIWidget *moveToLightSrcBtn =
        uiCreateComponentWidget(pObjectWindow, "Move to Light Source 0", &moveToLightSrcBtnWidget, WIDGET_TYPE_BUTTON);

    uiSetWidgetOnEditedCallback(moveToLightSrcBtn, nullptr,
                                [](void *pUserData)
                                {
                                    pCameraController->moveTo(f3Tov3(lightPosition));
                                    pCameraController->lookAt({0, 0, 0});
                                });
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

    DrawShape::Exit();

    for (int i = 0; i < arrlen(pUbObjects); i++)
    {
        removeResource(pUbObjects[i]);
    }
    tf_free(pUbObjects);

    for (int i = 0; i < arrlen(pUbLightSources); i++)
    {
        removeResource(pUbLightSources[i]);
    }
    tf_free(pUbLightSources);

    for (int i = 0; i < arrlen(pUbScene); i++)
    {
        removeResource(pUbScene[i]);
    }
    tf_free(pUbScene);

    exitCameraController(pCameraController);
}

void Demo2Scene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageCount)
{
    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        ShaderLoadDesc shaderLoadDesc = {};
        shaderLoadDesc.mStages[0] = {"demo2_object.vert", nullptr, 0, nullptr,
                                     SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW};
        shaderLoadDesc.mStages[1] = {"demo2_object.frag", nullptr, 0};

        addShader(pRenderer, &shaderLoadDesc, &pShObjects);

        shaderLoadDesc = {};
        shaderLoadDesc.mStages[0] = {"demo2_shadow.vert", nullptr};
        shaderLoadDesc.mStages[1] = {"demo2_shadow.frag", nullptr};

        addShader(pRenderer, &shaderLoadDesc, &pShShadow);

        shaderLoadDesc = {};
        shaderLoadDesc.mStages[0] = {"demo2_object.vert", nullptr};
        shaderLoadDesc.mStages[1] = {"demo2_lit.frag", nullptr};

        addShader(pRenderer, &shaderLoadDesc, &pShLightSources);

        shaderLoadDesc = {};
        shaderLoadDesc.mStages[0] = {"demo2_shadow.vert", nullptr};
        shaderLoadDesc.mStages[1] = {"demo2_object.frag", nullptr};

        addShader(pRenderer, &shaderLoadDesc, &pShShadowViewport);

        Shader *pShaders[]{pShObjects, pShShadow, pShLightSources, pShShadowViewport};

        RootSignatureDesc rootDesc = {};
        rootDesc.mShaderCount = 4;
        rootDesc.ppShaders = pShaders;

        addRootSignature(pRenderer, &rootDesc, &pRootSignature);

        DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, imageCount};
        addDescriptorSet(pRenderer, &desc, &pDsSceneUniform);

        desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_DRAW, static_cast<uint32_t>(imageCount * OBJECT_COUNT)};
        addDescriptorSet(pRenderer, &desc, &pDsObjectUniform);

        desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_DRAW,
                static_cast<uint32_t>(imageCount * DIRECTIONAL_LIGHT_COUNT)};
        addDescriptorSet(pRenderer, &desc, &pDsLightSourcesUniform);

        desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
        addDescriptorSet(pRenderer, &desc, &pDsTexture);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        VertexLayout vertexLayout = DrawShape::GetVertexLayout();

        RasterizerStateDesc rasterizerStateDesc = {};
        rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;

        RasterizerStateDesc rasterizerStateCullNoneDesc = {};
        rasterizerStateCullNoneDesc.mCullMode = CULL_MODE_NONE;

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
            pipelineSettings.pShaderProgram = pShObjects;
            pipelineSettings.pVertexLayout = &vertexLayout;
            pipelineSettings.pRasterizerState = &rasterizerStateDesc;
            pipelineSettings.mVRFoveatedRendering = true;
            addPipeline(pRenderer, &pipelineDesc, &pPlObjects);
        }

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
            pipelineSettings.pShaderProgram = pShLightSources;
            pipelineSettings.pVertexLayout = &vertexLayout;
            pipelineSettings.pRasterizerState = &rasterizerStateDesc;
            pipelineSettings.mVRFoveatedRendering = true;
            addPipeline(pRenderer, &pipelineDesc, &pPlLightSources);
        }

        {
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
            addRenderTarget(pRenderer, &renderTargetDesc, &pRtShadow);
        }

        {
            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc &pipelineSettings = pipelineDesc.mGraphicsDesc;
            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
            pipelineSettings.mRenderTargetCount = 0;
            pipelineSettings.pDepthState = &depthStateDesc;
            pipelineSettings.mSampleCount = pRtShadow->mSampleCount;
            pipelineSettings.mSampleQuality = pRtShadow->mSampleQuality;
            pipelineSettings.mDepthStencilFormat = pDepthBuffer->mFormat;
            pipelineSettings.pRootSignature = pRootSignature;
            pipelineSettings.pShaderProgram = pShShadow;
            pipelineSettings.pVertexLayout = &vertexLayout;
            pipelineSettings.pRasterizerState = &rasterizerStateCullNoneDesc;
            pipelineSettings.mVRFoveatedRendering = true;

            addPipeline(pRenderer, &pipelineDesc, &pPlShadow);
        }

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
            pipelineSettings.pShaderProgram = pShShadowViewport;
            pipelineSettings.pVertexLayout = &vertexLayout;
            pipelineSettings.pRasterizerState = &rasterizerStateDesc;
            pipelineSettings.mVRFoveatedRendering = true;

            addPipeline(pRenderer, &pipelineDesc, &pPlShadowViewport);
        }
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        RenderTargetDesc desc = {};
        desc.mArraySize = 1;
        desc.mClearValue.depth = 0.0f;
        desc.mClearValue.stencil = 0;
        desc.mDepth = 1;
        desc.mFlags = TEXTURE_CREATION_FLAG_ON_TILE | TEXTURE_CREATION_FLAG_VR_MULTIVIEW;
        desc.mFormat = TinyImageFormat_D32_SFLOAT;
        desc.mWidth = pRenderTarget->mWidth;
        desc.mHeight = pRenderTarget->mHeight;
        desc.mSampleCount = SAMPLE_COUNT_1;
        desc.mSampleQuality = 0;
        desc.mStartState = RESOURCE_STATE_DEPTH_WRITE;
        addRenderTarget(pRenderer, &desc, &pDepthBuffer);
    }

    for (int i = 0; i < imageCount * OBJECT_COUNT; i++)
    {
        DescriptorData params = {};
        params.pName = "uniformObjectBlock";
        params.ppBuffers = &pUbObjects[i];
        updateDescriptorSet(pRenderer, i, pDsObjectUniform, 1, &params);
    }

    for (int i = 0; i < imageCount * DIRECTIONAL_LIGHT_COUNT; i++)
    {
        DescriptorData params = {};
        params.pName = "uniformObjectBlock";
        params.ppBuffers = &pUbLightSources[i];
        updateDescriptorSet(pRenderer, i, pDsLightSourcesUniform, 1, &params);
    }

    for (int i = 0; i < imageCount; i++)
    {
        DescriptorData params = {};
        params.pName = "uniformSceneBlock";
        params.ppBuffers = &pUbScene[i];

        updateDescriptorSet(pRenderer, i, pDsSceneUniform, 1, &params);
    }

    DescriptorData params = {};
    params.pName = "shadowMap";
    params.ppTextures = &pRtShadow->pTexture;
    updateDescriptorSet(pRenderer, 0, pDsTexture, 1, &params);
}

void Demo2Scene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(pRenderer, pPlObjects);
        removePipeline(pRenderer, pPlShadow);
        removePipeline(pRenderer, pPlLightSources);
        removePipeline(pRenderer, pPlShadowViewport);
        removeRenderTarget(pRenderer, pRtShadow);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        removeDescriptorSet(pRenderer, pDsSceneUniform);
        removeDescriptorSet(pRenderer, pDsObjectUniform);
        removeDescriptorSet(pRenderer, pDsTexture);
        removeDescriptorSet(pRenderer, pDsLightSourcesUniform);

        removeRootSignature(pRenderer, pRootSignature);
        removeShader(pRenderer, pShObjects);
        removeShader(pRenderer, pShShadow);
        removeShader(pRenderer, pShLightSources);
        removeShader(pRenderer, pShShadowViewport);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        removeRenderTarget(pRenderer, pDepthBuffer);
    }
}

void Demo2Scene::Update(float deltaTime, uint32_t width, uint32_t height)
{
    pCameraController->update(deltaTime);

    cameraPosition = v3ToF3(pCameraController->getViewPosition());

    constexpr float lightDistant = 30.0f;
    constexpr float horizontal_fov = PI / 2.0f;

    const float aspectInverse = (float)height / (float)width;

    CameraMatrix projection = CameraMatrix::perspective(horizontal_fov, aspectInverse, 1000.0f, 0.1f);
    mat4 view = pCameraController->getViewMatrix();
    CameraMatrix mProjectView = projection * view;

    scene.CameraPosition = {pCameraController->getViewPosition(), 1.0f};
    scene.ProjectView = mProjectView;

    lightPosition = -scene.LightDirection[0].getXYZ() * lightDistant;
    auto lightView = mat4::lookAt(Point3(f3Tov3(lightPosition)), {0, 0, 0}, {0, 1, 0});

    scene.ShadowTransform = mat4::perspective(horizontal_fov, 1, 1000.0f, 0.1f) * lightView;

    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
        lightSources[i].Color = scene.LightColor[i];
        lightSources[i].Transform = mat4::translation(f3Tov3(-scene.LightDirection[i].getXYZ() * lightDistant));
    }
}

void Demo2Scene::PreDraw(uint32_t frameIndex)
{
    {
        BufferUpdateDesc updateDesc = {pUbScene[frameIndex]};
        beginUpdateResource(&updateDesc);
        *static_cast<SceneUniformBlock *>(updateDesc.pMappedData) = scene;
        endUpdateResource(&updateDesc, nullptr);
    }

    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        BufferUpdateDesc updateDesc = {pUbObjects[frameIndex * OBJECT_COUNT + i]};
        beginUpdateResource(&updateDesc);
        *static_cast<ObjectUniformBlock *>(updateDesc.pMappedData) = objects[i];
        endUpdateResource(&updateDesc, nullptr);
    }

    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
        BufferUpdateDesc updateDesc = {pUbLightSources[frameIndex * DIRECTIONAL_LIGHT_COUNT + i]};
        beginUpdateResource(&updateDesc);
        *static_cast<ObjectUniformBlock *>(updateDesc.pMappedData) = lightSources[i];
        endUpdateResource(&updateDesc, nullptr);
    }
}

void Demo2Scene::Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t frameIndex)
{
    // simply record the screen cleaning command
    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
    loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
    loadActions.mClearDepth.depth = 0.0f;
    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);

    DrawShadowRT(pCmd, frameIndex);

    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
    loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
    loadActions.mClearDepth.depth = 0.0f;

    cmdBindRenderTargets(pCmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(pCmd, 0, 0, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdBindPipeline(pCmd, pPlObjects);

    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        cmdBindDescriptorSet(pCmd, (frameIndex * OBJECT_COUNT) + i, pDsObjectUniform);
        cmdBindDescriptorSet(pCmd, frameIndex, pDsSceneUniform);
        cmdBindDescriptorSet(pCmd, 0, pDsTexture);

        DrawShape::Draw(pCmd, objectTypes[i]);
    }

    cmdBindPipeline(pCmd, pPlLightSources);

    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
        cmdBindDescriptorSet(pCmd, (frameIndex * DIRECTIONAL_LIGHT_COUNT) + i, pDsLightSourcesUniform);
        cmdBindDescriptorSet(pCmd, frameIndex, pDsSceneUniform);

        DrawShape::Draw(pCmd, DrawShape::Shape::Cube);
    }

    DrawShadowViewport(pCmd, pRenderTarget, frameIndex);
}

void Demo2Scene::DrawShadowRT(Cmd *&pCmd, uint32_t frameIndex)
{
    {
        RenderTargetBarrier barriers[] = {{pRtShadow, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_DEPTH_WRITE}};
        cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, 1, barriers);
    }

    LoadActionsDesc loadActions = {};
    loadActions.mLoadActionsColor[0] = LOAD_ACTION_DONTCARE;
    loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
    loadActions.mClearDepth.depth = 0.0f;
    // loadActions.mClearDepth.stencil = 0;

    cmdBindRenderTargets(pCmd, 0, nullptr, pRtShadow, &loadActions, nullptr, nullptr, -1, -1);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pRtShadow->mWidth, (float)pRtShadow->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRtShadow->mWidth, pRtShadow->mHeight);
    cmdBindPipeline(pCmd, pPlShadow);

    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        cmdBindDescriptorSet(pCmd, (frameIndex * OBJECT_COUNT) + i, pDsObjectUniform);
        cmdBindDescriptorSet(pCmd, frameIndex, pDsSceneUniform);

        DrawShape::Draw(pCmd, objectTypes[i]);
    }

    cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);

    {
        RenderTargetBarrier barriers[] = {
            {pRtShadow, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_SHADER_RESOURCE},
        };
        cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, 1, barriers);
    }
}

void Demo2Scene::DrawShadowViewport(Cmd *&pCmd, RenderTarget *&pRenderTarget, uint32_t frameIndex)
{
    cmdSetViewport(pCmd, (float)pRenderTarget->mWidth - SHADOW_VIEWPORT,
                   (float)pRenderTarget->mHeight - SHADOW_VIEWPORT, SHADOW_VIEWPORT, SHADOW_VIEWPORT, 0.0f, 1.0f);

    cmdSetScissor(pCmd, pRenderTarget->mWidth - SHADOW_VIEWPORT, pRenderTarget->mHeight - SHADOW_VIEWPORT,
                  SHADOW_VIEWPORT, SHADOW_VIEWPORT);

    cmdBindPipeline(pCmd, pPlShadowViewport);

    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        cmdBindDescriptorSet(pCmd, (frameIndex * OBJECT_COUNT) + i, pDsObjectUniform);
        cmdBindDescriptorSet(pCmd, frameIndex, pDsSceneUniform);
        cmdBindDescriptorSet(pCmd, 0, pDsTexture);

        DrawShape::Draw(pCmd, objectTypes[i]);
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
