#include "TestScene.h"

namespace {
struct UniformBlock {
    mat4 projectView;
    mat4 world;
};
} // namespace

void TestScene::Update(float deltaTime) {}

void TestScene::Draw(Cmd *cmd) {
    UniformBlock uniform;
    uniform.projectView = mat4::identity();
    uniform.projectView = mat4::identity();


}

void TestScene::DoUI() {}

bool TestScene::Load(Renderer *pRenderer) {

    {
        TextureLoadDesc desc{};
        desc.pFileName = "texture";
        desc.ppTexture = &pTexture;

        addResource(&desc, nullptr);
    }

    {
        VertexLayout gVertexLayoutDefault{};
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

        for (auto &buffer : pUniformBuffer) {
            ubDesc.ppBuffer = &buffer;
            addResource(&ubDesc, NULL);
        }

    }

    waitForAllResourceLoads();
    {
        ShaderLoadDesc desc{};
        desc.mStages[0] = {"model.vert", nullptr, 0};
        desc.mStages[1] = {"model.frag", nullptr, 0};

        addShader(pRenderer, &desc, &pShader);
    }

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

    DescriptorSetDesc desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
    addDescriptorSet(pRenderer, &desc, &pDescriptorSetTexture);
    desc = {pRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, MainApp::ImageCount };
    addDescriptorSet(pRenderer, &desc, &pDescriptorSetUniforms);

    return true;
}

void TestScene::Unload(Renderer *pRenderer) {
    removeResource(pTexture);
    removeResource(pGeometry);
    removeShader(pRenderer, pShader);

    for (auto &buffer : pUniformBuffer) {
        removeResource(buffer);
    }

    removeSampler(pRenderer, pSampler);
    removeDescriptorSet(pRenderer, pDescriptorSetTexture);
    removeDescriptorSet(pRenderer, pDescriptorSetUniforms);

}
