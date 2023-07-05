#include "QuadDemoScene.h"

#include <ICameraController.h>
#include <IGraphics.h>
#include <IOperatingSystem.h>
#include <IResourceLoader.h>
#include <IUI.h>
#include <Math/MathTypes.h>
#include <array>
#include "Settings.h"

namespace ChessScene
{
    const char *pFileName = "Chess.gltf";
    Geometry *pGeometry{nullptr};

    struct ObjectUniform
    {
        mat4 mWorld = mat4::identity();
    };

    struct ObjectResources
    {
        DescriptorSet *pDescriptorSets{nullptr};
        Buffer *pVertexBuffer;
        Buffer *pIndexBuffer;
    };

    struct SceneUniform
    {
        mat4 mViewProjection{mat4::identity()};
    };

    constexpr int OBJECT_COUNT = 1;

    ObjectUniform objects[OBJECT_COUNT]{};
    ObjectResources objectResources[OBJECT_COUNT]{};

    SceneUniform scene{};
    DescriptorSet *pSceneDS{nullptr};

    RootSignature *pRootSignature{nullptr};
    Sampler *pLinearSampler{nullptr};
    Sampler *pPointSampler{nullptr};

    bool Init()
    {
        SyncToken token = {};

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
        loadDesc.pFileName = pFileName;
        loadDesc.ppGeometry = &pGeometry;
        loadDesc.pVertexLayout = &vertexLayout;
        loadDesc.mFlags = GEOMETRY_LOAD_FLAG_SHADOWED;

        addResource(&loadDesc, &token);

        waitForToken(&token);
        waitForAllResourceLoads();    

        return true;
    }

    void Exit() { removeResource(pGeometry); }

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
                const char *samplerNames[]{"linearSampler", "pointSampler"};
                Sampler *pSamplers[]{pLinearSampler, pPointSampler};

                RootSignatureDesc desc{};
                desc.mShaderCount = 0;
                // desc.ppShaders = pShaders;
                desc.mStaticSamplerCount = 2;
                desc.ppStaticSamplers = pSamplers;
                desc.ppStaticSamplerNames = samplerNames;

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

                    addDescriptorSet(pRenderer, &desc, &r.pDescriptorSets);
                }
            }
        }
        return true;
    }

    void Unload(ReloadDesc *pReloadDesc, Renderer *pRenderer)
    {
        if (pReloadDesc->mType & RELOAD_TYPE_SHADER)
        {
            removeSampler(pRenderer, pLinearSampler);
            removeSampler(pRenderer, pPointSampler);

            removeRootSignature(pRenderer, pRootSignature);
            removeDescriptorSet(pRenderer, pSceneDS);

            for (auto &r : objectResources)
            {
                removeDescriptorSet(pRenderer, r.pDescriptorSets);
            }
        }
    }

    void Update(float deltaTime, uint32_t width, uint32_t height) {}

    void Draw(Cmd *pCmd, Renderer *pRenderer, RenderTarget *pRenderTarget, uint32_t imageIndex) {}

    void PreDraw(uint32_t imageIndex) {}
} // namespace ChessScene