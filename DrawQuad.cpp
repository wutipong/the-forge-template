#include "DrawQuad.h"
#include <array>

namespace DrawQuad
{
    struct Vertex
    {
        float2 position;
        float2 texcoord;
    };

    std::array<Vertex, 4> vertices{
        Vertex{{-1.0f, -1.0f}, {0.0f, 1.0f}},
        Vertex{{1.0f, -1.0f}, {1.0f, 1.0f}},
        Vertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},
        Vertex{{1.0f, 1.0f}, {1.0f, 0.0f}},
    };

    Buffer *pVertexBuffer{nullptr};
    Shader *pShader{nullptr};
    RootSignature *pRootSignature{nullptr};
    Pipeline *pPipeline{nullptr};

    void Init(SyncToken *token)
    {
        BufferLoadDesc vbDesc = {};
        vbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
        vbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        vbDesc.mDesc.mSize = sizeof(vertices);
        vbDesc.pData = vertices.data();
        vbDesc.ppBuffer = &pVertexBuffer;
        addResource(&vbDesc, token);
    }

    void Exit() { removeResource(pVertexBuffer); }

    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            ShaderLoadDesc shaderLoadDesc = {};
            shaderLoadDesc.mStages[0] = {"quad.vert"};
            shaderLoadDesc.mStages[1] = {"quad.frag"};

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
            vertexLayout.mAttribs[1].mOffset = sizeof(float2);

            BlendStateDesc blendStateDesc = {};
            blendStateDesc.mSrcFactors[0] = BC_SRC_ALPHA;
            blendStateDesc.mDstFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
            blendStateDesc.mSrcAlphaFactors[0] = BC_SRC_ALPHA;
            blendStateDesc.mDstAlphaFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
            blendStateDesc.mMasks[0] = ALL;
            blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_ALL;
            blendStateDesc.mIndependentBlend = false;

            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc &pipelineSettings = pipelineDesc.mGraphicsDesc;
            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_STRIP;
            pipelineSettings.mRenderTargetCount = 1;
            pipelineSettings.mSampleCount = pRenderTarget->mSampleCount;
            pipelineSettings.mSampleQuality = pRenderTarget->mSampleQuality;
            pipelineSettings.pBlendState = &blendStateDesc;
            pipelineSettings.pColorFormats = &pRenderTarget->mFormat;
            pipelineSettings.pDepthState = nullptr;
            pipelineSettings.pRasterizerState = &rasterizerStateDesc;
            pipelineSettings.pRootSignature = pRootSignature;
            pipelineSettings.pShaderProgram = pShader;
            pipelineSettings.pVertexLayout = &vertexLayout;

            addPipeline(pRenderer, &pipelineDesc, &pPipeline);
            ASSERT(pPipeline);
        }

        return true;
    }

    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            removePipeline(pRenderer, pPipeline);
            removeShader(pRenderer, pShader);
            removeRootSignature(pRenderer, pRootSignature);
        }
    }

    bool Quad::Init(SyncToken *token)
    {
        BufferLoadDesc desc = {};
        desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        desc.mDesc.mSize = sizeof(mat4);
        desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
        desc.pData = nullptr;

        for (auto &u : _pUniformBuffer)
        {
            desc.ppBuffer = &u;
            addResource(&desc, token);
        }

        return true;
    }

    void Quad::Exit()
    {
        for (auto &u : _pUniformBuffer)
        {
            removeResource(u);
        }
    }

    bool Quad::Load(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, IMAGE_COUNT};
            addDescriptorSet(pRenderer, &desc, &_pDSTransform);
            ASSERT(_pDSTransform);

            desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
            addDescriptorSet(pRenderer, &desc, &_pDSTexture);
            ASSERT(_pDSTexture);
        }

        {
            DescriptorData params = {};
            params.pName = "texture";
            params.ppTextures = &pTexture;

            updateDescriptorSet(pRenderer, 0, _pDSTexture, 1, &params);
        }

        for (int i = 0; i < IMAGE_COUNT; i++)
        {
            DescriptorData params = {};
            params.pName = "uniformBlock";
            params.ppBuffers = &_pUniformBuffer[i];
            updateDescriptorSet(pRenderer, i, _pDSTransform, 1, &params);
        }

        return true;
    }

    void Quad::Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            removeDescriptorSet(pRenderer, _pDSTransform);
            removeDescriptorSet(pRenderer, _pDSTexture);
        }
    }

    void Quad::PreDraw(const uint32_t &imageIndex)
    {
        BufferUpdateDesc updateDesc = {_pUniformBuffer[imageIndex]};
        beginUpdateResource(&updateDesc);
        *static_cast<mat4 *>(updateDesc.pMappedData) = transform;
        endUpdateResource(&updateDesc, nullptr);
    }

    void Quad::Draw(Cmd *pCmd, Renderer *pRenderer, const uint32_t &imageIndex)
    {
        cmdBindPipeline(pCmd, pPipeline);
        cmdBindDescriptorSet(pCmd, 0, _pDSTexture);
        cmdBindDescriptorSet(pCmd, imageIndex, _pDSTransform);

        constexpr uint32_t stride = sizeof(Vertex);
        cmdBindVertexBuffer(pCmd, 1, &pVertexBuffer, &stride, nullptr);

        cmdDraw(pCmd, vertices.size(), 0);
    }
} // namespace DrawQuad