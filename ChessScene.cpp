#include "QuadDemoScene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IOperatingSystem.h>
#include <IResourceLoader.h>
#include <IUI.h>
#include <Math/MathTypes.h>
#include <array>
#include <map>
#include <string>
#include "Settings.h"

namespace ChessScene
{
    std::string geomFileName = "Chess.gltf";
    Geometry *pGeometry{nullptr};

    std::map<std::string, Texture *> baseColorTextures{
        {"bishop_black_base_color", nullptr}, {"bishop_white_base_color", nullptr},
        {"castle_black_base_color", nullptr}, {"castle_white_base_color", nullptr},
        {"chessboard_base_color", nullptr},   {"king_black_base_color", nullptr},
        {"king_white_base_color", nullptr},   {"knight_black_base_color", nullptr},
        {"knight_white_base_color", nullptr}, {"pawn_black_base_color", nullptr},
        {"pawn_white_base_color", nullptr},   {"queen_black_base_color", nullptr},
        {"queen_white_base_color", nullptr},
    };

    std::map<std::string, Texture *> normalTextures{
        {"bishop_black_normal", nullptr}, {"bishop_white_normal", nullptr}, {"Castle_normal", nullptr},
        {"chessboard_normal", nullptr},   {"King_black_normal", nullptr},   {"King_white_normal", nullptr},
        {"Knight_normal", nullptr},       {"Pawn_normal", nullptr},         {"Queen_black_normal", nullptr},
        {"Queen_white_normal", nullptr},
    };

    std::map<std::string, Texture *> ormTextures{
        {"Bishop_black_ORM", nullptr}, {"Bishop_white_ORM", nullptr}, {"Castle_ORM", nullptr},
        {"Chessboard_ORM", nullptr},   {"King_black_ORM", nullptr},   {"King_white_ORM", nullptr},
        {"Knight_ORM", nullptr},       {"Pawn_ORM", nullptr},         {"Queen_black_ORM", nullptr},
        {"Queen_white_ORM", nullptr},
    };

    struct ObjectUniform
    {
        mat4 mWorld = mat4::identity();
    };

    struct ObjectResources
    {
        DescriptorSet *pDescriptorSet{nullptr};
        std::array<Buffer *, IMAGE_COUNT> pUniformBuffers{nullptr};

        Texture *pColorTexture{nullptr};
        Texture *pNormalTexture{nullptr};
        Texture *pOrmTexture{nullptr};

        uint32_t mStartIndex{0};
        uint32_t mIndexCount{0};
    };

    struct SceneUniform
    {
        mat4 mViewProjection{mat4::identity()};
    };

    constexpr int OBJECT_COUNT = 1;

    std::array<ObjectUniform, OBJECT_COUNT> objectsUniforms{};
    std::array<ObjectResources, OBJECT_COUNT> objectResources{};

    SceneUniform scene{};
    DescriptorSet *pSceneDS{nullptr};
    std::array<Buffer *, IMAGE_COUNT> pSceneBuffers;

    RootSignature *pRootSignature{nullptr};
    Sampler *pLinearSampler{nullptr};
    Sampler *pPointSampler{nullptr};
    Shader *pAlbedoShader{nullptr};
    Pipeline *pAlbedoPipeline{nullptr};
    RenderTarget *pDepthBuffer{nullptr};

    Buffer *pPositionBuffer{nullptr};
    Buffer *pTexCoordBuffer{nullptr};
    Buffer *pNormalBuffer{nullptr};
    Buffer *pIndexBuffer{nullptr};

    bool Init()
    {
        SyncToken token = {};

        {
            VertexLayout vertexLayout = {};
            vertexLayout.mAttribCount = 3;
            vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
            vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
            vertexLayout.mAttribs[0].mBinding = 0;
            vertexLayout.mAttribs[0].mLocation = 0;
            vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
            vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R16G16_SFLOAT;
            vertexLayout.mAttribs[1].mBinding = 1;
            vertexLayout.mAttribs[1].mLocation = 1;
            vertexLayout.mAttribs[2].mSemantic = SEMANTIC_NORMAL;
            vertexLayout.mAttribs[2].mFormat = TinyImageFormat_R16G16_UNORM;
            vertexLayout.mAttribs[2].mBinding = 2;
            vertexLayout.mAttribs[2].mLocation = 2;

            GeometryLoadDesc loadDesc = {};
            loadDesc.pFileName = geomFileName.c_str();
            loadDesc.ppGeometry = &pGeometry;
            loadDesc.pVertexLayout = &vertexLayout;
            loadDesc.mFlags = GEOMETRY_LOAD_FLAG_SHADOWED;

            addResource(&loadDesc, &token);
        }

        for (auto &t : baseColorTextures)
        {
            TextureLoadDesc desc = {};
            desc.pFileName = t.first.c_str();
            desc.ppTexture = &t.second;

            addResource(&desc, &token);
        }

        for (auto &t : normalTextures)
        {
            TextureLoadDesc desc = {};
            desc.pFileName = t.first.c_str();
            desc.ppTexture = &t.second;

            addResource(&desc, &token);
        }

        for (auto &t : ormTextures)
        {
            TextureLoadDesc desc = {};
            desc.pFileName = t.first.c_str();
            desc.ppTexture = &t.second;

            addResource(&desc, &token);
        }

        {
            BufferLoadDesc ubDesc = {};
            ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            ubDesc.mDesc.mSize = sizeof(SceneUniform);
            ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
            ubDesc.pData = nullptr;

            for (auto &pSceneBuffer : pSceneBuffers)
            {
                ubDesc.ppBuffer = &pSceneBuffer;
                addResource(&ubDesc, &token);
            }
        }

        {
            BufferLoadDesc ubDesc = {};
            ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            ubDesc.mDesc.mSize = sizeof(ObjectUniform);
            ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
            ubDesc.pData = nullptr;

            for (auto &o : objectResources)
            {
                for (auto &b : o.pUniformBuffers)
                {
                    ubDesc.ppBuffer = &b;
                    addResource(&ubDesc, &token);
                }
            }
        }

        waitForToken(&token);
        waitForAllResourceLoads();

        pPositionBuffer = pGeometry->pVertexBuffers[0];
        pTexCoordBuffer = pGeometry->pVertexBuffers[1];
        pNormalBuffer = pGeometry->pVertexBuffers[2];
        pIndexBuffer = pGeometry->pIndexBuffer;

        objectResources[0].pColorTexture = baseColorTextures["king_black_base_color"];
        objectResources[0].pNormalTexture = normalTextures["King_black_normal"];
        objectResources[0].pOrmTexture = ormTextures["King_black_ORM"];
        objectResources[0].mStartIndex = pGeometry->pDrawArgs[0].mStartIndex;
        objectResources[0].mIndexCount = pGeometry->pDrawArgs[0].mIndexCount;

        return true;
    }

    void Exit()
    {
        removeResource(pGeometry);

        for (auto &t : baseColorTextures)
        {
            removeResource(t.second);
        }

        for (auto &t : normalTextures)
        {
            removeResource(t.second);
        }

        for (auto &t : ormTextures)
        {
            removeResource(t.second);
        }

        for (auto &b : pSceneBuffers)
        {
            removeResource(b);
        }

        for (auto &o : objectResources)
        {
            for (auto &b : o.pUniformBuffers)
            {
                removeResource(b);
            }
        }
    }

    bool Load(ReloadDesc *pReloadDesc, Renderer *pRenderer, RenderTarget *pRenderTarget)
    {
        if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
        {
            {
                SamplerDesc samplerDesc = {
                    FILTER_LINEAR,
                    FILTER_LINEAR,
                    MIPMAP_MODE_LINEAR,
                    ADDRESS_MODE_CLAMP_TO_EDGE,
                    ADDRESS_MODE_CLAMP_TO_EDGE,
                    ADDRESS_MODE_CLAMP_TO_EDGE,
                };

                addSampler(pRenderer, &samplerDesc, &pLinearSampler);

                samplerDesc = {
                    FILTER_NEAREST,
                    FILTER_NEAREST,
                    MIPMAP_MODE_NEAREST,
                    ADDRESS_MODE_CLAMP_TO_EDGE,
                    ADDRESS_MODE_CLAMP_TO_EDGE,
                    ADDRESS_MODE_CLAMP_TO_EDGE,
                };
                addSampler(pRenderer, &samplerDesc, &pPointSampler);
            }

            {
                ShaderLoadDesc shaderLoadDesc = {};
                shaderLoadDesc.mStages[0] = {"chess-albedo.vert", nullptr, 0, nullptr,
                                             SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW};
                shaderLoadDesc.mStages[1] = {"chess-albedo.frag"};

                addShader(pRenderer, &shaderLoadDesc, &pAlbedoShader);
            }

            {
                std::array<const char *, 2> samplerNames{"linearSampler", "pointSampler"};
                std::array<Sampler *, 2> pSamplers{pLinearSampler, pPointSampler};
                std::array<Shader *, 1> pShaders{pAlbedoShader};

                RootSignatureDesc desc{};
                desc.mShaderCount = pShaders.size();
                desc.ppShaders = pShaders.data();
                desc.mStaticSamplerCount = samplerNames.size();
                desc.ppStaticSamplers = pSamplers.data();
                desc.ppStaticSamplerNames = samplerNames.data();

                addRootSignature(pRenderer, &desc, &pRootSignature);
                ASSERT(pRootSignature);
            }

            {
                DescriptorSetDesc desc{
                    pRootSignature,
                    DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
                    IMAGE_COUNT,
                };
                addDescriptorSet(pRenderer, &desc, &pSceneDS);

                for (auto &r : objectResources)
                {
                    desc = {
                        pRootSignature,
                        DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
                        IMAGE_COUNT,
                    };

                    addDescriptorSet(pRenderer, &desc, &r.pDescriptorSet);
                }
            }
        }
        if (pReloadDesc->mType & (RELOAD_TYPE_RENDERTARGET | RELOAD_TYPE_RESIZE))
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
            desc.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
            addRenderTarget(pRenderer, &desc, &pDepthBuffer);

            ASSERT(pDepthBuffer);
        }
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
        {
            RasterizerStateDesc rasterizerStateDesc = {};
            rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
            {
                DepthStateDesc depthStateDesc = {};
                depthStateDesc.mDepthTest = true;
                depthStateDesc.mDepthWrite = true;
                depthStateDesc.mDepthFunc = CMP_GEQUAL;

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
                pipelineSettings.pShaderProgram = pAlbedoShader;
                pipelineSettings.pVertexLayout = nullptr;
                pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                pipelineSettings.mVRFoveatedRendering = true;
                addPipeline(pRenderer, &pipelineDesc, &pAlbedoPipeline);

                ASSERT(pAlbedoPipeline);
            }
        }

        for (int i = 0; i < IMAGE_COUNT; i++)
        {
            DescriptorData params = {};
            params.pName = "uniformSceneBlock";
            params.ppBuffers = &pSceneBuffers[i];

            updateDescriptorSet(pRenderer, i, pSceneDS, 1, &params);

            params = {};
            params.pName = "uniformObjectBlock";

            for (auto &o : objectResources)
            {
                params.ppBuffers = &o.pUniformBuffers[i];
                updateDescriptorSet(pRenderer, i, o.pDescriptorSet, 1, &params);
            }
        }

        return true;
    }

    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
        {
            removeShader(pRenderer, pAlbedoShader);

            removeSampler(pRenderer, pLinearSampler);
            removeSampler(pRenderer, pPointSampler);

            removeRootSignature(pRenderer, pRootSignature);
            removeDescriptorSet(pRenderer, pSceneDS);

            for (auto &r : objectResources)
            {
                removeDescriptorSet(pRenderer, r.pDescriptorSet);
            }
        }

        if (pReloadDesc->mType & (RELOAD_TYPE_RENDERTARGET | RELOAD_TYPE_RESIZE))
        {
            removeRenderTarget(pRenderer, pDepthBuffer);
        }
        if (pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
        {
            removePipeline(pRenderer, pAlbedoPipeline);
        }
    }

    void Update(float deltaTime, uint32_t width, uint32_t height) {}

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageIndex)
    {
        cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
        {
            std::array<RenderTargetBarrier, 1> barriers = {
                {
                    pDepthBuffer,
                    RESOURCE_STATE_SHADER_RESOURCE,
                    RESOURCE_STATE_DEPTH_WRITE,
                },
            };
            cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
        }

        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
        loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
        loadActions.mClearDepth.depth = 0.0f;

        cmdBindRenderTargets(pCmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(pCmd, 0, 0, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
        cmdSetScissor(pCmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

        cmdBindPipeline(pCmd, pAlbedoPipeline);
        cmdBindDescriptorSet(pCmd, 0, pSceneDS);

        for (auto &objectResource : objectResources)
        {
            cmdBindIndexBuffer(pCmd, pIndexBuffer, INDEX_TYPE_UINT16, 0);
            cmdBindDescriptorSet(pCmd, imageIndex, objectResource.pDescriptorSet);
            cmdDrawIndexed(pCmd, objectResource.mIndexCount, objectResource.mStartIndex, 0);
        }

        cmdBindRenderTargets(pCmd, 0, nullptr, nullptr, nullptr, nullptr, nullptr, -1, -1);
        {
            std::array<RenderTargetBarrier, 1> barriers = {
                {
                    pDepthBuffer,
                    RESOURCE_STATE_DEPTH_WRITE,
                    RESOURCE_STATE_SHADER_RESOURCE,
                },
            };
            cmdResourceBarrier(pCmd, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
        }
    }

    void PreDraw(uint32_t imageIndex)
    {
        {
            BufferUpdateDesc updateDesc = {pSceneBuffers[imageIndex]};
            beginUpdateResource(&updateDesc);
            *static_cast<SceneUniform *>(updateDesc.pMappedData) = scene;
            endUpdateResource(&updateDesc, nullptr);
        }

        for (int i = 0; i < objectResources.size(); i++)
        {
            BufferUpdateDesc updateDesc = {objectResources[i].pUniformBuffers[imageIndex]};
            beginUpdateResource(&updateDesc);
            *static_cast<ObjectUniform *>(updateDesc.pMappedData) = objectsUniforms[i];
            endUpdateResource(&updateDesc, nullptr);
        }
    }
} // namespace ChessScene