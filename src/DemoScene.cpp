#include "DemoScene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IInput.h>
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
    std::array<float, MAX_SPHERE> size{1.0f};
    std::array<vec4, MAX_SPHERE> color{};
    std::array<vec3, MAX_SPHERE> speed{};

    struct SphereUniform
    {
        CameraMatrix projectView;
        CameraMatrix lightProjectView;
        std::array<mat4, MAX_SPHERE> world;
        std::array<vec4, MAX_SPHERE> color;
    } sphereUniform = {};

    struct QuadUniform
    {
        CameraMatrix projectView;
        CameraMatrix lightProjectView;
        mat4 world;
        vec4 color;
    } quadUniform = {};

    Shader *pShaderInstancing = nullptr;
    Shader *pShaderInstancingShadow = nullptr;
    RootSignature *pRSInstancing = nullptr;
    DescriptorSet *pDSSphereUniform = nullptr;
    Buffer *pBufferSphereUniform = nullptr;
    Buffer *pBufferSphereVertex = nullptr;
    Pipeline *pPipelineSphere = nullptr;
    Pipeline *pPipelineSphereShadow = nullptr;

    Shader *pShaderSingle = nullptr;
    Shader *pShaderSingleShadow = nullptr;
    RootSignature *pRSSingle = nullptr;
    DescriptorSet *pDSQuadUniform = nullptr;
    Buffer *pBufferQuadVertex = nullptr;
    Buffer *pBufferQuadIndex = nullptr;
    Buffer *pBufferQuadUniform = nullptr;
    Pipeline *pPipelineQuad = nullptr;
    Pipeline *pPipelineQuadShadow = nullptr;

    ICameraController *pCameraController = nullptr;

    RenderTarget *pRTDepth = nullptr;

    constexpr int SHADOW_MAP_SIZE = 2048;
    RenderTarget *pRTShadowMap = nullptr;
    DescriptorSet *pDSShadowMap = nullptr;

    CameraMatrix lightViewProj{};

    Sampler *pSampler;

    TinyImageFormat depthBufferFormat = TinyImageFormat_D32_SFLOAT;

    void AddSphereResources(Renderer *pRenderer);
    void RemoveSphereResources(Renderer *pRenderer);

    void AddQuadResources(Renderer *pRenderer);
    void RemoveQuadResources(Renderer *pRenderer);
} // namespace DemoScene

bool DemoScene::Init(Renderer *pRenderer)
{
    float *sphereVertices{};
    generateSpherePoints(&sphereVertices, &spherePoints, 12, 1.0f);

    float *quadVertices{};
    generateQuad(&quadVertices, &quadPoints);

    uint16_t quadIndices[6] = {0, 1, 2, 1, 3, 2};

    SyncToken token{};

    uint64_t sphereDataSize = spherePoints * sizeof(float);
    BufferLoadDesc sphereVbDesc = {};
    sphereVbDesc.ppBuffer = &pBufferSphereVertex;
    sphereVbDesc.pData = sphereVertices;
    sphereVbDesc.mDesc = {};
    sphereVbDesc.mDesc.mSize = sphereDataSize;
    sphereVbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    sphereVbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;

    addResource(&sphereVbDesc, &token);

    uint64_t quadDataSize = quadPoints * sizeof(float);
    BufferLoadDesc quadVbDesc = {};
    quadVbDesc.ppBuffer = &pBufferQuadVertex;
    quadVbDesc.pData = quadVertices;
    quadVbDesc.mDesc = {};
    quadVbDesc.mDesc.mSize = quadDataSize;
    quadVbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    quadVbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;

    addResource(&quadVbDesc, &token);

    BufferLoadDesc quadIdDesc = {};
    quadIdDesc.ppBuffer = &pBufferQuadIndex;
    quadIdDesc.pData = quadIndices;
    quadIdDesc.mDesc = {};
    quadIdDesc.mDesc.mSize = sizeof(uint16_t) * 6;
    quadIdDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    quadIdDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_INDEX_BUFFER;

    addResource(&quadIdDesc, &token);

    BufferLoadDesc ubDesc = {};
    ubDesc.ppBuffer = &pBufferSphereUniform;
    ubDesc.mDesc = {};
    ubDesc.mDesc.mSize = sizeof(SphereUniform);
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    addResource(&ubDesc, &token);

    BufferLoadDesc quadUniformDesc = {};
    quadUniformDesc.ppBuffer = &pBufferQuadUniform;
    quadUniformDesc.mDesc = {};
    quadUniformDesc.mDesc.mSize = sizeof(QuadUniform);
    quadUniformDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    quadUniformDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    quadUniformDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    addResource(&quadUniformDesc, &token);

    for (size_t i = 0; i < MAX_SPHERE; i++)
    {
        position[i] = {randomFloat(-200, 200), randomFloat(-200, 200), randomFloat(-200, 200)};
        color[i] = {randomFloat01(), randomFloat01(), randomFloat01(), 1.0f};
        size[i] = randomFloat(0, 10);
        speed[i] = {randomFloat(-10.0f, 10.0f), randomFloat(-10.0f, 10.0f), randomFloat(-10.0f, 10.0f)};
    }

    CameraMotionParameters cmp{160.0f, 600.0f, 1000.0f};
    vec3 camPos{0.0f, 0.0f, 20.0f};
    vec3 lookAt{vec3(0)};

    pCameraController = initFpsCameraController(camPos, lookAt);
    pCameraController->setMotionParameters(cmp);

    SamplerDesc samplerDesc = {};
    samplerDesc.mMinFilter = FILTER_LINEAR;
    samplerDesc.mMagFilter = FILTER_LINEAR;
    samplerDesc.mAddressU = ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.mAddressV = ADDRESS_MODE_CLAMP_TO_EDGE;

    addSampler(pRenderer, &samplerDesc, &pSampler);

    typedef bool (*CameraInputHandler)(InputActionContext *ctx, DefaultInputActions::DefaultInputAction action);
    static CameraInputHandler onCameraInput =
        [](InputActionContext *ctx, DefaultInputActions::DefaultInputAction action)
    {
        if (*(ctx->pCaptured))
        {
            float2 delta = uiIsFocused() ? float2(0.f, 0.f) : ctx->mFloat2;
            switch (action)
            {
            case DefaultInputActions::ROTATE_CAMERA:
                pCameraController->onRotate(delta);
                break;

            case DefaultInputActions::TRANSLATE_CAMERA:
                pCameraController->onMove(delta);
                break;

            case DefaultInputActions::TRANSLATE_CAMERA_VERTICAL:
                pCameraController->onMoveY(delta[0]);
                break;

            default:
                break;
            }
        }
        return true;
    };
    InputActionDesc actionDesc = {DefaultInputActions::CAPTURE_INPUT,
                                  [](InputActionContext *ctx)
                                  {
                                      setEnableCaptureInput(!uiIsFocused() &&
                                                            INPUT_ACTION_PHASE_CANCELED != ctx->mPhase);
                                      return true;
                                  },
                                  NULL};
    addInputAction(&actionDesc);
    actionDesc = {DefaultInputActions::ROTATE_CAMERA,
                  [](InputActionContext *ctx) { return onCameraInput(ctx, DefaultInputActions::ROTATE_CAMERA); }, NULL};
    addInputAction(&actionDesc);
    actionDesc = {DefaultInputActions::TRANSLATE_CAMERA, [](InputActionContext *ctx)
                  { return onCameraInput(ctx, DefaultInputActions::TRANSLATE_CAMERA); }, NULL};
    addInputAction(&actionDesc);
    actionDesc = {DefaultInputActions::TRANSLATE_CAMERA_VERTICAL, [](InputActionContext *ctx)
                  { return onCameraInput(ctx, DefaultInputActions::TRANSLATE_CAMERA_VERTICAL); }, NULL};
    addInputAction(&actionDesc);
    actionDesc = {DefaultInputActions::RESET_CAMERA, [](InputActionContext *ctx)
                  {
                      if (!uiWantTextInput())
                          pCameraController->resetView();
                      return true;
                  }};
    addInputAction(&actionDesc);

    waitForToken(&token);

    tf_free(sphereVertices);
    tf_free(quadVertices);

    return true;
}

void DemoScene::Exit(Renderer *pRenderer)
{
    removeResource(pBufferSphereUniform);
    removeResource(pBufferSphereVertex);

    removeResource(pBufferQuadUniform);
    removeResource(pBufferQuadVertex);
    removeResource(pBufferQuadIndex);

    removeSampler(pRenderer, pSampler);

    exitCameraController(pCameraController);
}

bool DemoScene::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget)
{
    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        AddSphereResources(pRenderer);
        AddQuadResources(pRenderer);

        DescriptorSetDesc dsDesc = {pRSInstancing, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
        addDescriptorSet(pRenderer, &dsDesc, &pDSShadowMap);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        RenderTargetDesc desc{};
        desc.mFlags = TEXTURE_CREATION_FLAG_ON_TILE | TEXTURE_CREATION_FLAG_VR_MULTIVIEW;
        desc.mWidth = pRenderTarget->mWidth;
        desc.mHeight = pRenderTarget->mHeight;
        desc.mDepth = 1;
        desc.mArraySize = 1;
        desc.mSampleCount = SAMPLE_COUNT_1;
        desc.mFormat = depthBufferFormat;
        desc.mStartState = RESOURCE_STATE_DEPTH_WRITE;
        desc.mClearValue = {};
        desc.mSampleQuality = 0;

        addRenderTarget(pRenderer, &desc, &pRTDepth);
        ASSERT(pRTDepth);

        desc = {};
        desc.mFlags = TEXTURE_CREATION_FLAG_OWN_MEMORY_BIT;
        desc.mWidth = SHADOW_MAP_SIZE;
        desc.mHeight = SHADOW_MAP_SIZE;
        desc.mDepth = 1;
        desc.mArraySize = 1;
        desc.mSampleCount = SAMPLE_COUNT_1;
        desc.mFormat = depthBufferFormat;
        desc.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
        desc.mClearValue = {};
        desc.mSampleQuality = 0;
        desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;

        addRenderTarget(pRenderer, &desc, &pRTShadowMap);
        ASSERT(pRTShadowMap);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        // layout and pipeline for sphere draw
        VertexLayout vertexLayout = {};
        vertexLayout.mBindingCount = 1;
        vertexLayout.mBindings[0].mStride = sizeof(float) * 6;

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
        sphereRasterizerStateDesc.mCullMode = CULL_MODE_NONE;

        DepthStateDesc depthStateDesc = {};
        depthStateDesc.mDepthTest = true;
        depthStateDesc.mDepthWrite = true;
        depthStateDesc.mDepthFunc = CMP_GEQUAL;

        PipelineDesc desc = {};
        desc.mType = PIPELINE_TYPE_GRAPHICS;

        desc.mGraphicsDesc = {};
        desc.mGraphicsDesc.pShaderProgram = pShaderInstancing;
        desc.mGraphicsDesc.pRootSignature = pRSInstancing;
        desc.mGraphicsDesc.pVertexLayout = &vertexLayout;
        desc.mGraphicsDesc.pDepthState = &depthStateDesc;
        desc.mGraphicsDesc.pRasterizerState = &sphereRasterizerStateDesc;
        desc.mGraphicsDesc.pColorFormats = &pRenderTarget->mFormat;
        desc.mGraphicsDesc.mRenderTargetCount = 1;
        desc.mGraphicsDesc.mSampleCount = pRenderTarget->mSampleCount;
        desc.mGraphicsDesc.mSampleQuality = pRenderTarget->mSampleQuality;
        desc.mGraphicsDesc.mDepthStencilFormat = depthBufferFormat;
        desc.mGraphicsDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        desc.mGraphicsDesc.mVRFoveatedRendering = true;


        addPipeline(pRenderer, &desc, &pPipelineSphere);
        ASSERT(pPipelineSphere);

        desc = {};
        desc.mType = PIPELINE_TYPE_GRAPHICS;
        desc.mGraphicsDesc = {};
        desc.mGraphicsDesc.pShaderProgram = pShaderInstancingShadow;
        desc.mGraphicsDesc.pRootSignature = pRSInstancing;
        desc.mGraphicsDesc.pVertexLayout = &vertexLayout;
        desc.mGraphicsDesc.pDepthState = &depthStateDesc;
        desc.mGraphicsDesc.pRasterizerState = &sphereRasterizerStateDesc;
        desc.mGraphicsDesc.mSampleCount = pRenderTarget->mSampleCount;
        desc.mGraphicsDesc.mSampleQuality = pRenderTarget->mSampleQuality;
        desc.mGraphicsDesc.mDepthStencilFormat = depthBufferFormat;
        desc.mGraphicsDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        desc.mGraphicsDesc.mVRFoveatedRendering = true;

        addPipeline(pRenderer, &desc, &pPipelineSphereShadow);
        ASSERT(pPipelineSphereShadow);

        desc = {};
        desc.mType = PIPELINE_TYPE_GRAPHICS;
        desc.mGraphicsDesc = {};
        desc.mGraphicsDesc.pShaderProgram = pShaderSingle;
        desc.mGraphicsDesc.pRootSignature = pRSSingle;
        desc.mGraphicsDesc.pVertexLayout = &vertexLayout;
        desc.mGraphicsDesc.pDepthState = &depthStateDesc;
        desc.mGraphicsDesc.pRasterizerState = &sphereRasterizerStateDesc;
        desc.mGraphicsDesc.pColorFormats = &pRenderTarget->mFormat;
        desc.mGraphicsDesc.mRenderTargetCount = 1;
        desc.mGraphicsDesc.mSampleCount = pRenderTarget->mSampleCount;
        desc.mGraphicsDesc.mSampleQuality = pRenderTarget->mSampleQuality;
        desc.mGraphicsDesc.mDepthStencilFormat = depthBufferFormat;
        desc.mGraphicsDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        desc.mGraphicsDesc.mVRFoveatedRendering = true;

        addPipeline(pRenderer, &desc, &pPipelineQuad);
        ASSERT(pPipelineQuad);

        desc = {};
        desc.mType = PIPELINE_TYPE_GRAPHICS;
        desc.mGraphicsDesc = {};
        desc.mGraphicsDesc.pShaderProgram = pShaderSingleShadow;
        desc.mGraphicsDesc.pRootSignature = pRSSingle;
        desc.mGraphicsDesc.pVertexLayout = &vertexLayout;
        desc.mGraphicsDesc.pDepthState = &depthStateDesc;
        desc.mGraphicsDesc.pRasterizerState = &sphereRasterizerStateDesc;
        desc.mGraphicsDesc.mSampleCount = pRenderTarget->mSampleCount;
        desc.mGraphicsDesc.mSampleQuality = pRenderTarget->mSampleQuality;
        desc.mGraphicsDesc.mDepthStencilFormat = depthBufferFormat;
        desc.mGraphicsDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        desc.mGraphicsDesc.mVRFoveatedRendering = true;

        addPipeline(pRenderer, &desc, &pPipelineQuadShadow);
        ASSERT(pPipelineQuadShadow);
    }

    DescriptorData params = {};
    params.pName = "uniformBlock";
    params.ppBuffers = &pBufferSphereUniform;
    updateDescriptorSet(pRenderer, 0, pDSSphereUniform, 1, &params);

    params = {};
    params.pName = "uniformBlock";
    params.ppBuffers = &pBufferQuadUniform;
    updateDescriptorSet(pRenderer, 0, pDSQuadUniform, 1, &params);

    params = {};
    params.pName = "lightMap";
    params.ppTextures = &pRTShadowMap->pTexture;
    updateDescriptorSet(pRenderer, 0, pDSShadowMap, 1, &params);

    return true;
}

void DemoScene::AddSphereResources(Renderer *pRenderer)
{
    ShaderLoadDesc shaderDesc{};

    shaderDesc.mStages[0].pFileName = "sphere.vert";
    shaderDesc.mStages[1].pFileName = "basic.frag";
    addShader(pRenderer, &shaderDesc, &pShaderInstancing);
    ASSERT(pShaderInstancing);

    shaderDesc = {};
    shaderDesc.mStages[0].pFileName = "sphere_shadow.vert";
    shaderDesc.mStages[1].pFileName = "shadow.frag";
    addShader(pRenderer, &shaderDesc, &pShaderInstancingShadow);
    ASSERT(pShaderInstancingShadow);

    std::array<Shader *, 2> shaders = {pShaderInstancing, pShaderInstancingShadow};
    RootSignatureDesc rootDesc = {};
    rootDesc.ppShaders = shaders.data();
    rootDesc.mShaderCount = shaders.size();
    addRootSignature(pRenderer, &rootDesc, &pRSInstancing);
    ASSERT(pRSInstancing);

    DescriptorSetDesc dsDesc = {pRSInstancing, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1};
    addDescriptorSet(pRenderer, &dsDesc, &pDSSphereUniform);

    ASSERT(pDSSphereUniform);
}

void DemoScene::AddQuadResources(Renderer *pRenderer)
{
    ShaderLoadDesc shaderDesc{};
    shaderDesc.mStages[0].pFileName = "quad.vert";
    shaderDesc.mStages[1].pFileName = "basic.frag";
    addShader(pRenderer, &shaderDesc, &pShaderSingle);
    ASSERT(pShaderSingle);

    shaderDesc = {};
    shaderDesc.mStages[0].pFileName = "quad_shadow.vert";
    shaderDesc.mStages[1].pFileName = "shadow.frag";
    addShader(pRenderer, &shaderDesc, &pShaderSingleShadow);
    ASSERT(pShaderSingleShadow);

    std::array<Shader *, 2> shaders = {pShaderSingle, pShaderSingleShadow};
    RootSignatureDesc rootDesc{};
    rootDesc.ppShaders = shaders.data();
    rootDesc.mShaderCount = shaders.size();

    addRootSignature(pRenderer, &rootDesc, &pRSSingle);
    ASSERT(pRSSingle);

    DescriptorSetDesc dsDesc = {pRSSingle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1};
    addDescriptorSet(pRenderer, &dsDesc, &pDSQuadUniform);

    ASSERT(pDSQuadUniform);
}

void DemoScene::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
{
    if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(pRenderer, pPipelineSphere);
        removePipeline(pRenderer, pPipelineSphereShadow);
        removePipeline(pRenderer, pPipelineQuad);
        removePipeline(pRenderer, pPipelineQuadShadow);
    }

    if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        RemoveSphereResources(pRenderer);
        RemoveQuadResources(pRenderer);

        removeDescriptorSet(pRenderer, pDSShadowMap);
    }

    if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
    {
        removeRenderTarget(pRenderer, pRTDepth);
        removeRenderTarget(pRenderer, pRTShadowMap);
    }
}

void DemoScene::RemoveSphereResources(Renderer *pRenderer)
{
    removeDescriptorSet(pRenderer, pDSSphereUniform);
    removeRootSignature(pRenderer, pRSInstancing);
    removeShader(pRenderer, pShaderInstancing);
    removeShader(pRenderer, pShaderInstancingShadow);
}

void DemoScene::RemoveQuadResources(Renderer *pRenderer)
{
    removeDescriptorSet(pRenderer, pDSQuadUniform);
    removeRootSignature(pRenderer, pRSSingle);
    removeShader(pRenderer, pShaderSingle);
    removeShader(pRenderer, pShaderSingleShadow);
}

void DemoScene::Update(float deltaTime, uint32_t width, uint32_t height)
{
    const float aspectInverse = (float)height / (float)width;
    const float horizontal_fov = PI / 2.0f;

    Point3 lightPos{0, 300, 500};
    Point3 lightLookAt{0, -200, 0};
    mat4 lightView = mat4::lookAtLH(lightPos, lightLookAt, {0, 1, 0});
    lightViewProj = CameraMatrix::orthographic(-200, 200, -200, 200, 1000, 0.1) * lightView;

    sphereUniform.lightProjectView = lightViewProj;
    quadUniform.lightProjectView = lightViewProj;

    pCameraController->update(deltaTime);
    CameraMatrix projMat = CameraMatrix::perspective(horizontal_fov, aspectInverse, 1000.0f, 0.1f);
    CameraMatrix mProjectView = projMat * pCameraController->getViewMatrix();

    sphereUniform.projectView = mProjectView;

    for (int i = 0; i < MAX_SPHERE; i++)
    {
        position[i] = position[i] + (deltaTime * speed[i]);
        if (fabs(position[i].getX()) > 200 || fabs(position[i].getY()) > 200 || fabs(position[i].getZ()) > 200)
        {
            position[i].setX(randomFloat(-200, 200));
            position[i].setY(randomFloat(-200, 200));
            position[i].setZ(randomFloat(-200, 200));

            color[i] = {randomFloat01(), randomFloat01(), randomFloat01(), 1.0};
            size[i] = randomFloat(1.0f, 10.0f);
        }
        sphereUniform.color[i] = color[i];
        sphereUniform.world[i] = mat4::translation(position[i]) * mat4::scale({size[i], size[i], size[i]});
    }

    quadUniform.projectView = mProjectView;
    quadUniform.color = {1.0f, 1.0f, 1.0f, 1.0f};
    quadUniform.world = mat4::translation({0, -200, 0}) * mat4::rotationX(degToRad(-90)) * mat4::scale({200, 200, 200});
}

void DemoScene::Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget)
{
    constexpr uint32_t stride = sizeof(float) * 6;
    {
        RenderTargetBarrier barriers[]{
            {pRTShadowMap, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_DEPTH_WRITE},
        };
        cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, 1, barriers);
    }

    {
        BindRenderTargetsDesc bindRenderTargets = {};
        bindRenderTargets.mDepthStencil = {pRTShadowMap, LOAD_ACTION_CLEAR};
        cmdBindRenderTargets(pCmd, &bindRenderTargets);
    }
    cmdSetViewport(pCmd, 0.0f, 0.0f, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    cmdBindPipeline(pCmd, pPipelineSphereShadow);
    cmdBindDescriptorSet(pCmd, 0, pDSSphereUniform);
    cmdBindVertexBuffer(pCmd, 1, &pBufferSphereVertex, &stride, nullptr);
    cmdDrawInstanced(pCmd, spherePoints / 6, 0, MAX_SPHERE, 0);

    cmdBindPipeline(pCmd, pPipelineQuadShadow);
    cmdBindDescriptorSet(pCmd, 0, pDSQuadUniform);
    cmdBindVertexBuffer(pCmd, 1, &pBufferQuadVertex, &stride, nullptr);
    cmdBindIndexBuffer(pCmd, pBufferQuadIndex, INDEX_TYPE_UINT16, 0);
    cmdDrawIndexed(pCmd, 6, 0, 0);

    cmdBindRenderTargets(pCmd, nullptr);
    {
        RenderTargetBarrier barriers[]{
            {pRTShadowMap, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_SHADER_RESOURCE},
        };
        cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, 1, barriers);
    }

    BindRenderTargetsDesc bindRenderTargets = {};
    bindRenderTargets.mRenderTargetCount = 1;
    bindRenderTargets.mRenderTargets[0] = {pRenderTarget, LOAD_ACTION_CLEAR};
    bindRenderTargets.mDepthStencil = {pRTDepth, LOAD_ACTION_CLEAR},

    cmdBindRenderTargets(pCmd, &bindRenderTargets);
    cmdSetViewport(pCmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

    cmdBindPipeline(pCmd, pPipelineSphere);
    cmdBindDescriptorSet(pCmd, 0, pDSSphereUniform);
    cmdBindDescriptorSet(pCmd, 0, pDSShadowMap);
    cmdBindVertexBuffer(pCmd, 1, &pBufferSphereVertex, &stride, nullptr);
    cmdDrawInstanced(pCmd, spherePoints / 6, 0, MAX_SPHERE, 0);

    cmdBindPipeline(pCmd, pPipelineQuad);
    cmdBindDescriptorSet(pCmd, 0, pDSQuadUniform);
    cmdBindDescriptorSet(pCmd, 0, pDSShadowMap);
    cmdBindVertexBuffer(pCmd, 1, &pBufferQuadVertex, &stride, nullptr);
    cmdBindIndexBuffer(pCmd, pBufferQuadIndex, INDEX_TYPE_UINT16, 0);
    cmdDrawIndexed(pCmd, 6, 0, 0);
}

void DemoScene::PreDraw()
{
    BufferUpdateDesc sphereUniformUpdateDesc = {pBufferSphereUniform};
    beginUpdateResource(&sphereUniformUpdateDesc);
    *(SphereUniform *)sphereUniformUpdateDesc.pMappedData = sphereUniform;
    endUpdateResource(&sphereUniformUpdateDesc);

    BufferUpdateDesc quadUniformUpdateDesc = {pBufferQuadUniform};
    beginUpdateResource(&quadUniformUpdateDesc);
    *(QuadUniform *)quadUniformUpdateDesc.pMappedData = quadUniform;
    endUpdateResource(&quadUniformUpdateDesc);
}
