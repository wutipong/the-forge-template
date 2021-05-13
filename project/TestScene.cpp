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

    waitForAllResourceLoads();
    {
        ShaderLoadDesc desc{};
        desc.mStages[0] = {"model.vert", nullptr, 0};
        desc.mStages[1] = {"model.frag", nullptr, 0};

        addShader(pRenderer, &desc, &pShader);
    }
    return true;
}

void TestScene::Unload(Renderer *pRenderer) {
    removeResource(pTexture);
    removeResource(pGeometry);
    removeShader(pRenderer, pShader);
}
