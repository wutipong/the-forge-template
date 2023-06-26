#include "ColorGrading.h"

#include <array>

namespace ColorGrading
{
    struct screenVertex
    {
        float2 position;
        float2 texcoord;
    }; 
    
    std::array<screenVertex, 4> quad{
        screenVertex{{-1.0f, -1.0f}, {0.0f, 1.0f}},
        screenVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},
        screenVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},
        screenVertex{{1.0f, 1.0f}, {1.0f, 0.0f}},
    };

    Buffer *pVertexBuffer{nullptr};
    Shader *pShader{nullptr};
    Pipeline *pPipeline{nullptr};
    DescriptorSet *pDescriptorSet{nullptr};
    RootSignature *pRootSignature{nullptr};

    void Init(SyncToken *token)
    {
        BufferLoadDesc vbDesc = {};
        vbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
        vbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        vbDesc.mDesc.mSize = sizeof(quad);
        vbDesc.pData = quad.data();
        vbDesc.ppBuffer = &pVertexBuffer;
        addResource(&vbDesc, token);
    }

    void Exit()
    {
        removeResource(pVertexBuffer);
    }

    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, Texture *pTexture)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            ShaderLoadDesc shaderLoadDesc = {};
            shaderLoadDesc.mStages[0] = {"color_grading.vert"};
            shaderLoadDesc.mStages[1] = {"color_grading.frag"};

            addShader(pRenderer, &shaderLoadDesc, &pShader);

            RootSignatureDesc rootDesc = {};
            rootDesc.mShaderCount = 1;
            rootDesc.ppShaders = &pShader;

            addRootSignature(pRenderer, &rootDesc, &pRootSignature);

            RasterizerStateDesc rasterizerStateDesc = {};
            rasterizerStateDesc.mCullMode = CULL_MODE_NONE;

            VertexLayout vertexLayout = {};
            vertexLayout.mAttribCount = 2;
            vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
            vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32_SFLOAT;
            vertexLayout.mAttribs[0].mBinding = 0;
            vertexLayout.mAttribs[0].mLocation = 0;
            vertexLayout.mAttribs[0].mOffset = 0;

            vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
            vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
            vertexLayout.mAttribs[1].mBinding = 0;
            vertexLayout.mAttribs[1].mLocation = 1;
            vertexLayout.mAttribs[1].mOffset = 2 * sizeof(float);

            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc &pipelineSettings = pipelineDesc.mGraphicsDesc;
            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_STRIP;
            pipelineSettings.mRenderTargetCount = 1;
            pipelineSettings.pDepthState = nullptr;
            pipelineSettings.pColorFormats = &pRenderTarget->mFormat;
            pipelineSettings.mSampleCount = pRenderTarget->mSampleCount;
            pipelineSettings.mSampleQuality = pRenderTarget->mSampleQuality;
            pipelineSettings.pRootSignature = pRootSignature;
            pipelineSettings.pShaderProgram = pShader;
            pipelineSettings.pVertexLayout = &vertexLayout;
            pipelineSettings.pRasterizerState = &rasterizerStateDesc;

            addPipeline(pRenderer, &pipelineDesc, &pPipeline);

            DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
            addDescriptorSet(pRenderer, &desc, &pDescriptorSet);
        }

        {
            std::array<DescriptorData, 1> params = {};
            params[0].pName = "texture";
            params[0].ppTextures = &pTexture;

            updateDescriptorSet(pRenderer, 0, pDescriptorSet, params.size(), params.data());
        }
    }

    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            removePipeline(pRenderer, pPipeline);
            removeShader(pRenderer, pShader);
            removeDescriptorSet(pRenderer, pDescriptorSet);
            removeRootSignature(pRenderer, pRootSignature);
        }
    }


    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget) {
        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;

        cmdBindRenderTargets(pCmd, 1, &pRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(pCmd, 0, 0, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
        cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

        cmdBindPipeline(pCmd, pPipeline);
        cmdBindDescriptorSet(pCmd, 0, pDescriptorSet);

        constexpr uint32_t stride = sizeof(screenVertex);
        cmdBindVertexBuffer(pCmd, 1, &pVertexBuffer, &stride, nullptr);
        cmdDraw(pCmd, quad.size(), 0);
    }
} // namespace ColorGrading