#pragma once

#include "Camera.h"
#include "Input.h"
#include <vector>

struct Vertex {
    float x, y, z;
    float u, v;
};

struct WorldMatrixBuffer {
    XMMATRIX worldMatrix;
};

struct ViewMatrixBuffer {
    XMMATRIX viewProjectionMatrix;
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

    ID3D11Device* pDevice_;
    ID3D11DeviceContext* pDeviceContext_;
    IDXGISwapChain* pSwapChain_;
    ID3D11RenderTargetView* pRenderTargetView_;

    ID3D11Buffer* pVertexBuffer_[2] = {NULL, NULL};
    ID3D11Buffer* pIndexBuffer_[2] = { NULL, NULL };
    ID3D11InputLayout* pInputLayout_[2] = { NULL, NULL };
    ID3D11VertexShader* pVertexShader_[2] = { NULL, NULL };
    ID3D11PixelShader* pPixelShader_[2] = { NULL, NULL };

    ID3D11Buffer* pWorldMatrixBuffer_[2] = { NULL, NULL };
    ID3D11Buffer* pViewMatrixBuffer_[2] = { NULL, NULL };
    ID3D11RasterizerState* pRasterizerState_;
    ID3D11SamplerState* pSampler_;

    ID3D11ShaderResourceView* pTexture_[2] = { NULL, NULL };

    Camera* pCamera_;
    Input* pInput_;

    UINT width_;
    UINT height_;
    UINT numSphereTriangles_;
    float radius_;
};