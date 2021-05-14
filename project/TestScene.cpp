#include "TestScene.h"

namespace {
#pragma pack(push, 1)
struct UniformBlock {
    mat4 world;
    mat4 projectView;
};
#pragma pack(pop)
} // namespace

void TestScene::Update(float deltaTime) { pCameraController->update(deltaTime); }

void TestScene::Draw(Cmd *cmd, int imageIndex) {
    mat4 viewMat = pCameraController->getViewMatrix();
    const float aspectInverse =
        (float)MainApp::Instance()->mSettings.mHeight / (float)MainApp::Instance()->mSettings.mWidth;
    const float horizontal_fov = PI / 2.0f;
    mat4 projMat = mat4::perspective(horizontal_fov, aspectInverse, 1000.0f, 0.1f);

    UniformBlock uniform;
    uniform.world = mat4::identity();
    uniform.projectView = projMat * viewMat;

    BufferUpdateDesc viewProjCbv = {pUniformBuffers[imageIndex]};
    beginUpdateResource(&viewProjCbv);
    *(UniformBlock *)viewProjCbv.pMappedData = uniform;
    endUpdateResource(&viewProjCbv, NULL);

    for (int i = 0; i < pGeometry->mVertexBufferCount; i++) {
        cmdBindPipeline(cmd, pPipeline);
        cmdBindDescriptorSet(cmd, 0, pDescriptorSetTexture);
        cmdBindDescriptorSet(cmd, imageIndex, pDescriptorSetUniforms);
        cmdBindVertexBuffer(cmd, 1, &pGeometry->pVertexBuffers[i], &pGeometry->mVertexStrides[i], NULL);
        cmdBindIndexBuffer(cmd, &pGeometry->pIndexBuffer[i], pGeometry->mIndexType, 0);
        cmdDrawIndexed(cmd, pGeometry->mIndexCount, 0, 0);
    }
}

void TestScene::DoUI() {}

bool TestScene::Load(Renderer *pRenderer, SwapChain *pSwapChain) {

    {
        TextureLoadDesc desc{};
        desc.pFileName = "texture";
        desc.ppTexture = &pTexture;

        addResource(&desc, nullptr);
    }

    {
        ShaderLoadDesc desc{};
        desc.mStages[0] = {"model.vert", nullptr, 0};
        desc.mStages[1] = {"model.frag", nullptr, 0};

        addShader(pRenderer, &desc, &pShader);
    }


    VertexLayout gVertexLayoutDefault{};
    {
        gVertexLayoutDefault.mAttribCount = 3;
        gVertexLayoutDefault.mAttribs[0].mSemantic = SEMANTIC_POSITION;
        gVertexLayoutDefault.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
        gVertexLayoutDefault.mAttribs[0].mBinding = 0;
        gVertexLayoutDefault.mAttribs[0].mLocation = 0;
        gVertexLayoutDefault.mAttribs[0].mOffset = 0;
        gVertexLayoutDefault.mAttribs[1].mSemantic = SEMANTIC_NORMAL;
        gVertexLayoutDefault.mAttribs[1].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
        gVertexLayoutDefault.mAttribs[1].mLocation = 1;
        gVertexLayoutDefault.mAttribs[1].mBinding = 0;
        gVertexLayoutDefault.mAttribs[1].mOffset = 3 * sizeof(float);
        gVertexLayoutDefault.mAttribs[2].mSemantic = SEMANTIC_TEXCOORD0;
        gVertexLayoutDefault.mAttribs[2].mFormat = TinyImageFormat_R32G32_SFLOAT;
        gVertexLayoutDefault.mAttribs[2].mLocation = 2;
        gVertexLayoutDefault.mAttribs[2].mBinding = 0;
        gVertexLayoutDefault.mAttribs[2].mOffset = 6 * sizeof(float);

        GeometryLoadDesc desc{};
        desc.pFileName = "model.glb";
        desc.ppGeometry = &pGeometry;
        desc.pVertexLayout = &gVertexLayoutDefault;

        addResource(&desc, nullptr);
    }

    {
        BufferLoadDesc ubDesc = {};
        ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        ubDesc.mDesc.mSize = sizeof(UniformBlock);
        ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
        ubDesc.pData = NULL;

        for (auto &buffer : pUniformBuffers) {
            ubDesc.ppBuffer = &buffer;
            addResource(&ubDesc, NULL);
        }
    }

    waitForAllResourceLoads();

    SamplerDesc samplerDesc = {FILTER_LINEAR,
                               FILTER_LINEAR,
                               MIPMAP_MODE_NEAREST,
                               ADDRESS_MODE_CLAMP_TO_EDGE,
                               ADDRESS_MODE_CLAMP_TO_EDGE,
                               ADDRESS_MODE_CLAMP_TO_EDGE};
    addSampler(pRenderer, &samplerDesc, &pSampler);

    const char *pStaticSamplers[] = {"uSampler0"};
    RootSignatureDesc rootDesc = {};
    rootDesc.mStaticSamplerCount = 1;
    rootDesc.ppStaticSamplerNames = pStaticSamplers;
    rootDesc.ppStaticSamplers = &pSampler;
    rootDesc.mShaderCount = 1;
    rootDesc.ppShaders = &pShader;
    addRootSignature(pRenderer, &rootDesc, &pRootSignature);

    {
        DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetTexture);

        desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, MainApp::ImageCount};
        addDescriptorSet(pRenderer, &desc, &pDescriptorSetUniforms);
    }

    {
        DescriptorData params = {};
        params.pName = "uTex";
        params.ppTextures = &pTexture;
        updateDescriptorSet(pRenderer, 0, pDescriptorSetTexture, 1, &params);
    }
    

    for (uint32_t i = 0; i < MainApp::ImageCount; ++i) {
        DescriptorData params{};
        auto size = sizeof(UniformBlock);
        params.pName = "uniformBlock";
        params.ppBuffers = &pUniformBuffers[i];

        updateDescriptorSet(pRenderer, i, pDescriptorSetUniforms, 1, &params);
    }

    {
        RasterizerStateDesc rasterizerStateDesc = {};
        rasterizerStateDesc.mCullMode = CULL_MODE_BACK;

        DepthStateDesc depthStateDesc = {};
        depthStateDesc.mDepthTest = true;
        depthStateDesc.mDepthWrite = true;
        depthStateDesc.mDepthFunc = CMP_GEQUAL;

        BlendStateDesc blendStateAlphaDesc = {};
        blendStateAlphaDesc.mSrcFactors[0] = BC_SRC_ALPHA;
        blendStateAlphaDesc.mDstFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
        blendStateAlphaDesc.mBlendModes[0] = BM_ADD;
        blendStateAlphaDesc.mSrcAlphaFactors[0] = BC_ONE;
        blendStateAlphaDesc.mDstAlphaFactors[0] = BC_ZERO;
        blendStateAlphaDesc.mBlendAlphaModes[0] = BM_ADD;
        blendStateAlphaDesc.mMasks[0] = ALL;
        blendStateAlphaDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
        blendStateAlphaDesc.mIndependentBlend = false;

        PipelineDesc desc = {};

        desc.mType = PIPELINE_TYPE_GRAPHICS;
        GraphicsPipelineDesc &pipelineSettings = desc.mGraphicsDesc;
        pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        pipelineSettings.mRenderTargetCount = 1;
        pipelineSettings.pDepthState = NULL;
        pipelineSettings.pBlendState = &blendStateAlphaDesc;
        pipelineSettings.pColorFormats = &pSwapChain->ppRenderTargets[0]->mFormat;
        pipelineSettings.mSampleCount = pSwapChain->ppRenderTargets[0]->mSampleCount;
        pipelineSettings.mSampleQuality = pSwapChain->ppRenderTargets[0]->mSampleQuality;
        pipelineSettings.pRootSignature = pRootSignature;
        pipelineSettings.pVertexLayout = &gVertexLayoutDefault;
        pipelineSettings.pRasterizerState = &rasterizerStateDesc;
        pipelineSettings.pShaderProgram = pShader;
        addPipeline(pRenderer, &desc, &pPipeline);
    }

    CameraMotionParameters cmp{160.0f, 600.0f, 200.0f};
    vec3 camPos{2.0f, 2.0f, 2.0f};
    vec3 lookAt{vec3(0)};

    pCameraController = createFpsCameraController(camPos, lookAt);

    pCameraController->setMotionParameters(cmp);

    return true;
}

void TestScene::Unload(Renderer *pRenderer) {
    removeRootSignature(pRenderer, pRootSignature);
    removePipeline(pRenderer, pPipeline);

    removeResource(pTexture);
    removeResource(pGeometry);
    removeShader(pRenderer, pShader);

    for (auto &buffer : pUniformBuffers) {
        removeResource(buffer);
    }

    removeSampler(pRenderer, pSampler);
    removeDescriptorSet(pRenderer, pDescriptorSetTexture);
    removeDescriptorSet(pRenderer, pDescriptorSetUniforms);

    destroyCameraController(pCameraController);
}
