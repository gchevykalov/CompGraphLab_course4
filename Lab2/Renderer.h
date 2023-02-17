#pragma once

#include "framework.h"

struct Vertex {
    float x, y, z;
    COLORREF color;
};

class Renderer {
public:
    static constexpr UINT defaultWidth = 1280;
    static constexpr UINT defaultHeight = 720;

    static Renderer& GetInstance();
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    bool Init(const HWND hWnd);
    bool Render();
    bool Resize(UINT width, UINT height);

    void Cleanup();
    ~Renderer();

private:
    Renderer();

    HRESULT InitScene();

    ID3D11Device* pDevice_;
    ID3D11DeviceContext* pDeviceContext_;
    IDXGISwapChain* pSwapChain_;
    ID3D11RenderTargetView* pRenderTargetView_;

    ID3D11Buffer* pVertexBuffer_;
    ID3D11Buffer* pIndexBuffer_;
    ID3D11InputLayout* pInputLayout_;
    ID3D11VertexShader* pVertexShader_;
    ID3D11PixelShader* pPixelShader_;

    UINT width_;
    UINT height_;
};