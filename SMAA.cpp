#include "SMAA.h"

#include "ThirdParty/smaa/Textures/AreaTex.h"
#include "ThirdParty/smaa/Textures/SearchTex.h"

#include <IGraphics.h>
#include <IResourceLoader.h>

#include <array>

namespace SMAA
{
    Texture *areaTexture{nullptr};
    Texture *searchTexture{nullptr};

    Sampler *linearSampler{nullptr};
    Sampler *pointSampler{nullptr};

    Shader *edgeDetectShader{nullptr};
    Shader *blendingWeightShader{nullptr};
    Shader *neighborhoodBlendShader{nullptr};

    Pipeline *edgeDetectPipeline{nullptr};
    Pipeline *blendingWeightPipeline{nullptr};
    Pipeline *neighborhoodBlendPipeline{nullptr};

    RenderTarget *edgesRenderTarget{nullptr};
    RenderTarget *blendRenderTarget{nullptr};

    DescriptorSet *pDsEdges{nullptr};
    DescriptorSet *pDsBlendingWeight{nullptr};
    DescriptorSet *pDsNeighborhoodBlend{nullptr};

    RootSignature *pRootSignature{nullptr};

    struct screenVertex
    {
        float2 position;
        float2 texcoord;
    } quads[] = {
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f}, {1.0f, 0.0f}},
    };

    Buffer *pVertexBuffer{nullptr};

    Texture *CreateTexture(const uint32_t &width, const uint32_t &height, const TinyImageFormat &format,
                           const char *name, const unsigned char *data, SyncToken *token = nullptr)
    {
        Texture *texture;
        TextureDesc textureDesc = {};
        textureDesc.mWidth = width;
        textureDesc.mHeight = height;
        textureDesc.mSampleCount = SAMPLE_COUNT_1;
        textureDesc.mFormat = format;
        textureDesc.mStartState = RESOURCE_STATE_COMMON;
        textureDesc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
        textureDesc.pName = name;
        textureDesc.mArraySize = 1;
        textureDesc.mMipLevels = 1;
        textureDesc.mDepth = 1;

        TextureLoadDesc textureLoadDesc = {};
        textureLoadDesc.pDesc = &textureDesc;
        textureLoadDesc.ppTexture = &texture;
        addResource(&textureLoadDesc, token);

        TextureUpdateDesc updateDesc = {};
        updateDesc.pTexture = texture;

        beginUpdateResource(&updateDesc);
        for (int r = 0; r < updateDesc.mRowCount; r++)
        {
            std::memcpy(updateDesc.pMappedData + r * updateDesc.mDstRowStride, data + r * updateDesc.mSrcRowStride,
                        updateDesc.mSrcRowStride);
        }

        endUpdateResource(&updateDesc, token);
        return texture;
    }

    Pipeline *AddPipeline(Shader *shader, Renderer *pRenderer, RenderTarget *pRenderTarget)
    {
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
        pipelineSettings.pShaderProgram = shader;
        pipelineSettings.pVertexLayout = &vertexLayout;
        pipelineSettings.pRasterizerState = &rasterizerStateDesc;

        Pipeline *pipeline;

        addPipeline(pRenderer, &pipelineDesc, &pipeline);

        return pipeline;
    }

    void Init(SyncToken *token)
    {
        areaTexture =
            CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, TinyImageFormat_R8G8_UNORM, "areaTex", areaTexBytes, nullptr);
        searchTexture = CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, TinyImageFormat_R8_UNORM, "searchTex",
                                      searchTexBytes, nullptr);

        BufferLoadDesc vbDesc = {};
        vbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
        vbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        vbDesc.mDesc.mSize = sizeof(quads);
        vbDesc.pData = quads;
        vbDesc.ppBuffer = &pVertexBuffer;
        addResource(&vbDesc, token);
    }

    void Exit()
    {
        removeResource(areaTexture);
        removeResource(searchTexture);
        removeResource(pVertexBuffer);
    }

    void Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget, Texture *pTexture)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
        {
            {
                RenderTargetDesc desc = {};
                desc.mArraySize = 1;
                desc.mDepth = 1;
                desc.mClearValue.r = 0.0f;
                desc.mClearValue.g = 0.0f;
                desc.mClearValue.b = 0.0f;
                desc.mClearValue.a = 0.0f;
                desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                desc.mFormat = TinyImageFormat_R8G8_UNORM;
                desc.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
                desc.mHeight = pRenderTarget->mHeight;
                desc.mWidth = pRenderTarget->mWidth;
                desc.mSampleCount = SAMPLE_COUNT_1;
                desc.mSampleQuality = 0;
                desc.mFlags = TEXTURE_CREATION_FLAG_OWN_MEMORY_BIT;
                desc.pName = "edgesTex";
                addRenderTarget(pRenderer, &desc, &edgesRenderTarget);
            }

            {
                RenderTargetDesc desc = {};
                desc.mArraySize = 1;
                desc.mDepth = 1;
                desc.mClearValue.r = 0.0f;
                desc.mClearValue.g = 0.0f;
                desc.mClearValue.b = 0.0f;
                desc.mClearValue.a = 0.0f;
                desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                desc.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                desc.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
                desc.mHeight = pRenderTarget->mHeight;
                desc.mWidth = pRenderTarget->mWidth;
                desc.mSampleCount = SAMPLE_COUNT_1;
                desc.mSampleQuality = 0;
                desc.mFlags = TEXTURE_CREATION_FLAG_OWN_MEMORY_BIT;
                desc.pName = "blendTex";
                addRenderTarget(pRenderer, &desc, &blendRenderTarget);
            }
        }

        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            SamplerDesc samplerDesc = {
                FILTER_LINEAR,
                FILTER_LINEAR,
                MIPMAP_MODE_LINEAR,
                ADDRESS_MODE_CLAMP_TO_EDGE,
                ADDRESS_MODE_CLAMP_TO_EDGE,
                ADDRESS_MODE_CLAMP_TO_EDGE,
            };

            addSampler(pRenderer, &samplerDesc, &linearSampler);

            samplerDesc = {
                FILTER_NEAREST,
                FILTER_NEAREST,
                MIPMAP_MODE_NEAREST,
                ADDRESS_MODE_CLAMP_TO_EDGE,
                ADDRESS_MODE_CLAMP_TO_EDGE,
                ADDRESS_MODE_CLAMP_TO_EDGE,
            };
            addSampler(pRenderer, &samplerDesc, &pointSampler);

            ShaderLoadDesc shaderLoadDesc = {};
            shaderLoadDesc.mStages[0] = {"smaa-edges.vert"};
            shaderLoadDesc.mStages[1] = {"smaa-edges.frag"};

            addShader(pRenderer, &shaderLoadDesc, &edgeDetectShader);

            shaderLoadDesc = {};
            shaderLoadDesc.mStages[0] = {"smaa-blendingweight.vert"};
            shaderLoadDesc.mStages[1] = {"smaa-blendingweight.frag"};

            addShader(pRenderer, &shaderLoadDesc, &blendingWeightShader);

            shaderLoadDesc = {};
            shaderLoadDesc.mStages[0] = {"smaa-neighborhoodblend.vert"};
            shaderLoadDesc.mStages[1] = {"smaa-neighborhoodblend.frag"};

            addShader(pRenderer, &shaderLoadDesc, &neighborhoodBlendShader);

            Shader *pShaders[] = {edgeDetectShader, blendingWeightShader, neighborhoodBlendShader};
            Sampler *pSamplers[] = {linearSampler, pointSampler};
            const char *samplerNames[] = {"LinearSampler", "PointSampler"};

            RootSignatureDesc rootDesc = {};
            rootDesc.mShaderCount = 3;
            rootDesc.ppShaders = pShaders;
            rootDesc.mStaticSamplerCount = 2;
            rootDesc.ppStaticSamplerNames = samplerNames;
            rootDesc.ppStaticSamplers = pSamplers;

            addRootSignature(pRenderer, &rootDesc, &pRootSignature);

            ASSERT(edgeDetectPipeline = AddPipeline(edgeDetectShader, pRenderer, edgesRenderTarget));
            ASSERT(blendingWeightPipeline = AddPipeline(blendingWeightShader, pRenderer, blendRenderTarget));
            ASSERT(neighborhoodBlendPipeline = AddPipeline(neighborhoodBlendShader, pRenderer, pRenderTarget));

            DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
            addDescriptorSet(pRenderer, &desc, &pDsEdges);

            desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
            addDescriptorSet(pRenderer, &desc, &pDsBlendingWeight);

            desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
            addDescriptorSet(pRenderer, &desc, &pDsNeighborhoodBlend);
        }

        {
            std::array<DescriptorData, 1> params = {};
            params[0].pName = "colorTex";
            params[0].ppTextures = &pTexture;

            updateDescriptorSet(pRenderer, 0, pDsEdges, params.size(), params.data());
        }

        {
            std::array<DescriptorData, 3> params = {};
            params[0].pName = "areaTex";
            params[0].ppTextures = &areaTexture;

            params[1].pName = "searchTex";
            params[1].ppTextures = &searchTexture;

            params[2].pName = "edgesTex";
            params[2].ppTextures = &edgesRenderTarget->pTexture;

            updateDescriptorSet(pRenderer, 0, pDsBlendingWeight, params.size(), params.data());
        }

        {
            std::array<DescriptorData, 2> params = {};
            params[0].pName = "colorTex";
            params[0].ppTextures = &pTexture;

            params[1].pName = "blendTex";
            params[1].ppTextures = &blendRenderTarget->pTexture;

            updateDescriptorSet(pRenderer, 0, pDsNeighborhoodBlend, params.size(), params.data());
        }
    }

    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER))
        {
            removeSampler(pRenderer, linearSampler);
            removeSampler(pRenderer, pointSampler);

            removePipeline(pRenderer, edgeDetectPipeline);
            removePipeline(pRenderer, blendingWeightPipeline);
            removePipeline(pRenderer, neighborhoodBlendPipeline);

            removeShader(pRenderer, edgeDetectShader);
            removeShader(pRenderer, blendingWeightShader);
            removeShader(pRenderer, neighborhoodBlendShader);

            removeDescriptorSet(pRenderer, pDsEdges);
            removeDescriptorSet(pRenderer, pDsBlendingWeight);
            removeDescriptorSet(pRenderer, pDsNeighborhoodBlend);

            removeRootSignature(pRenderer, pRootSignature);
        }

        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
        {
            removeRenderTarget(pRenderer, blendRenderTarget);
            removeRenderTarget(pRenderer, edgesRenderTarget);
        }
    }

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget)
    {
        cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
        {
            RenderTargetBarrier barriers[] = {
                {edgesRenderTarget, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET}};
            cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, 1, barriers);
        }
        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;

        cmdBindRenderTargets(pCmd, 1, &edgesRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(pCmd, 0, 0, (float)edgesRenderTarget->mWidth, (float)edgesRenderTarget->mHeight, 0.0f, 1.0f);
        cmdSetScissor(pCmd, 0, 0, edgesRenderTarget->mWidth, edgesRenderTarget->mHeight);

        cmdBindPipeline(pCmd, edgeDetectPipeline);
        cmdBindDescriptorSet(pCmd, 0, pDsEdges);

        constexpr uint32_t stride = sizeof(screenVertex);
        cmdBindVertexBuffer(pCmd, 1, &pVertexBuffer, &stride, nullptr);
        cmdDraw(pCmd, 4, 0);

        cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
        {
            std::array<RenderTargetBarrier, 2> barriers = {
                RenderTargetBarrier{edgesRenderTarget, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE},
                RenderTargetBarrier{blendRenderTarget, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET},
            };
            cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
        }

        loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;

        cmdBindRenderTargets(pCmd, 1, &blendRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(pCmd, 0, 0, (float)blendRenderTarget->mWidth, (float)blendRenderTarget->mHeight, 0.0f, 1.0f);
        cmdSetScissor(pCmd, 0, 0, blendRenderTarget->mWidth, blendRenderTarget->mHeight);

        cmdBindPipeline(pCmd, blendingWeightPipeline);
        cmdBindDescriptorSet(pCmd, 0, pDsBlendingWeight);

        cmdBindVertexBuffer(pCmd, 1, &pVertexBuffer, &stride, nullptr);
        cmdDraw(pCmd, 4, 0);

        cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
        {
            std::array<RenderTargetBarrier, 1> barriers = {
                RenderTargetBarrier{blendRenderTarget, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE},
            };
            cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
        }

        loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;

        cmdBindRenderTargets(pCmd, 1, &pRenderTarget, nullptr, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(pCmd, 0, 0, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
        cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

        cmdBindPipeline(pCmd, neighborhoodBlendPipeline);
        cmdBindDescriptorSet(pCmd, 0, pDsNeighborhoodBlend);

        cmdBindVertexBuffer(pCmd, 1, &pVertexBuffer, &stride, nullptr);
        cmdDraw(pCmd, 4, 0);
    }
} // namespace SMAA