#pragma once

#include "Camera.h"
#include "Input.h"
#include "D3DInclude.h"
#include "Macros.h"
#include "Frustum.h"
#include <vector>
#include <string>

struct Light {
    XMFLOAT4 pos;
    XMFLOAT4 color;
};

struct Cube {
    XMFLOAT4 pos;
    XMFLOAT4 shineSpeedIdNM;
};

struct PostEffectConstantBuffer {
    XMINT4 params;
};

struct Vertex {
    XMFLOAT3 pos;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
    XMFLOAT3 tangent;
};

struct GeomBuffer {
    XMMATRIX worldMatrix;
    XMMATRIX norm;
    XMFLOAT4 shineSpeedTexIdNM;
};

struct SceneBuffer {
    XMMATRIX viewProjectionMatrix;
    XMFLOAT4 planes[6];
};

struct CullingParams {
    XMINT4 numShapes;
    XMFLOAT4 bbMin[MAX_CUBE];
    XMFLOAT4 bbMax[MAX_CUBE];
};

struct LightBuffer {
    XMFLOAT4 cameraPos;
    XMINT4 lightParams;
    Light lights[MAX_LIGHT];
    XMFLOAT4 ambientColor;
};

struct SkyboxVertex {
    float x, y, z;
};

struct SkyboxWorldMatrixBuffer {
    XMMATRIX worldMatrix;
    XMFLOAT4 size;
};

struct SkyboxViewMatrixBuffer {
    XMMATRIX viewProjectionMatrix;
    XMFLOAT4 cameraPos;
};

struct TransparentVertex {
    float x, y, z;
};

struct TransparentWorldMatrixBuffer {
    XMMATRIX worldMatrix;
    XMFLOAT4 color;
};

class Renderer {
public:
    static constexpr UINT defaultWidth = 1280;
    static constexpr UINT defaultHeight = 720;

    static Renderer& GetInstance();
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    bool Init(HINSTANCE hInstance, HWND hWnd);
    bool Render();
    bool Resize(UINT width, UINT height);

    void Cleanup();
    ~Renderer();

private:
    Renderer();

    HRESULT InitScene();
    void InputHandler();
    bool UpdateScene();
    void ProcessPostEffect(D3D11_VIEWPORT viewport);
    HRESULT InitRenderTexture(int textureWidth, int textureHeight);
    void ReleaseRenderTexture();
    void ReadQueries();

    ID3D11Device* pDevice_;
    ID3D11DeviceContext* pDeviceContext_;
    IDXGISwapChain* pSwapChain_;
    ID3D11RenderTargetView* pRenderTargetView_;

    ID3D11Buffer* pVertexBuffer_[3] = {NULL, NULL, NULL };
    ID3D11Buffer* pIndexBuffer_[3] = { NULL, NULL, NULL };
    ID3D11InputLayout* pInputLayout_[3] = { NULL, NULL, NULL };
    ID3D11VertexShader* pVertexShader_[3] = { NULL, NULL, NULL };
    ID3D11PixelShader* pPixelShader_[3] = { NULL, NULL, NULL };

    ID3D11Buffer* pGeomBufferInst_ = NULL;
    ID3D11Buffer* pPlanesWorldMatrixBuffer_[2] = { NULL, NULL };
    ID3D11Buffer* pSkyboxWorldMatrixBuffer_ = NULL;
    ID3D11Buffer* pViewMatrixBuffer_[2] = { NULL, NULL };
    ID3D11Buffer* pLightBuffer_ = NULL;
    ID3D11RasterizerState* pRasterizerState_;
    ID3D11SamplerState* pSampler_;

    ID3D11ShaderResourceView* pTexture_[3] = { NULL, NULL, NULL };
    ID3D11Texture2D* pDepthBuffer_;
    ID3D11DepthStencilView* pDepthBufferDSV_;
    ID3D11DepthStencilState* pDepthState_[2] = { NULL, NULL };
    ID3D11BlendState* pBlendState_;

    ID3D11VertexShader* pPostEffectVertexShader_ = NULL;
    ID3D11PixelShader* pPostEffectPixelShader_ = NULL;
    ID3D11SamplerState* pPostEffectSamplerState_ = NULL;
    ID3D11Buffer* pPostEffectConstantBuffer_ = NULL;
    ID3D11Texture2D* pRenderTargetTexture_ = NULL;
    ID3D11RenderTargetView* pPostEffectRenderTargetView_ = NULL;
    ID3D11ShaderResourceView* pShaderResourceView_ = NULL;

    ID3D11Buffer* pCullingParams_ = NULL;
    ID3D11ComputeShader* pCullingShader_ = NULL;

    ID3D11Buffer* pInderectArgsSrc_ = NULL;
    ID3D11Buffer* pInderectArgs_ = NULL;
    ID3D11UnorderedAccessView* pInderectArgsUAV_ = NULL;
    ID3D11Buffer* pGeomBufferInstVis_ = NULL;
    ID3D11Buffer* pGeomBufferInstVisGpu_ = NULL;
    ID3D11UnorderedAccessView* pGeomBufferInstVisGpuUAV_ = NULL;

    Camera* pCamera_;
    Input* pInput_;
    Frustum* pFrustum_;

    bool useNormalMap_ = true;
    bool showNormals_ = false;
    bool withPostEffect_ = true;
    bool withCulling_ = true;
    bool withGPUCulling_ = false;
    std::vector<Light> lights_;
    std::vector<Cube> cubes_;
    std::vector<int> cubeIndexies_;
    int cubesCount_ = 2;
    int cubesCountGPU_ = 2;

    ID3D11Query* queries_[MAX_QUERY];
    unsigned int curFrame_ = 0;
    unsigned int lastCompletedFrame_ = 0;

    UINT width_;
    UINT height_;
    UINT numSphereTriangles_;
    float radius_;

    const TransparentVertex VerticesT[4] = {
        {0, -1, -1},
        {0,  1, -1},
        {0,  1,  1},
        {0, -1,  1}
    };
    XMMATRIX TransparentMatrixs[2] = {
        DirectX::XMMatrixTranslation(1.8f, 0.0f, 0.0f),
        DirectX::XMMatrixTranslation(2.2f, 0.0f, 0.0f)
    };
    bool isFirst_ = true;

    const XMFLOAT4 AABB[8] = {
        {-1.0f, -1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f,  1.0f, 1.0f, 1.0f}
    };
};