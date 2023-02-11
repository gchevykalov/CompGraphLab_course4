#include "Renderer.h"

#define SAFE_RELEASE(A) if ((A) != NULL) { (A)->Release(); }

Renderer& Renderer::GetInstance() {
    static Renderer instance;
    return instance;
}

Renderer::Renderer() :
    pDevice_(NULL),
    pDeviceContext_(NULL),
    pSwapChain_(NULL),
    pRenderTargetView_(NULL) {}

bool Renderer::Init(const HWND hWnd) {
    // Create a DirectX graphics interface factory.​
    IDXGIFactory* pFactory = nullptr;
    HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
    // Select hardware adapter
    IDXGIAdapter* pSelectedAdapter = NULL;
    if (SUCCEEDED(result)) {
        IDXGIAdapter* pAdapter = NULL;
        UINT adapterIdx = 0;
        while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter))) {
            DXGI_ADAPTER_DESC desc;
            pAdapter->GetDesc(&desc);
            if (wcscmp(desc.Description, L"Microsoft Basic Render Driver")) {
                pSelectedAdapter = pAdapter;
                break;
            }
            pAdapter->Release();
            adapterIdx++;
        }
    }
    if (pSelectedAdapter == NULL)
        return false;

    // Create DirectX11 device
    D3D_FEATURE_LEVEL level;
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG
    result = D3D11CreateDevice(
        pSelectedAdapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        NULL,
        flags,
        levels,
        1,
        D3D11_SDK_VERSION,
        &pDevice_,
        &level,
        &pDeviceContext_
    );
    if (D3D_FEATURE_LEVEL_11_0 != level || !SUCCEEDED(result))
        return false;

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = defaultWidth;
    swapChainDesc.BufferDesc.Height = defaultHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = 0;
    result = pFactory->CreateSwapChain(pDevice_, &swapChainDesc, &pSwapChain_);
    if (!SUCCEEDED(result))
        return false;

    ID3D11Texture2D* pBackBuffer = NULL;
    result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (!SUCCEEDED(result))
        return false;
    result = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView_);
    if (!SUCCEEDED(result))
        return false;
    SAFE_RELEASE(pBackBuffer);

    return true;
}

bool Renderer::Render() {
    pDeviceContext_->ClearState();
    ID3D11RenderTargetView* views[] = { pRenderTargetView_ };
    pDeviceContext_->OMSetRenderTargets(1, views, NULL);

    static const FLOAT backColor[4] = { 0.4f, 0.2f, 0.4f, 1.0f };
    pDeviceContext_->ClearRenderTargetView(pRenderTargetView_, backColor);

    HRESULT result = pSwapChain_->Present(0, 0);
    if (!SUCCEEDED(result))
        return false;

    return true;
}

bool Renderer::Resize(UINT width, UINT height) {
    if (pSwapChain_ == NULL)
        return false;

    SAFE_RELEASE(pRenderTargetView_);

    width = max(width, 8);
    height = max(height, 8);

    auto result = pSwapChain_->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (!SUCCEEDED(result))
        return false;

    ID3D11Texture2D* pBuffer;
    result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &pBuffer);
    if (!SUCCEEDED(result))
        return false;

    result = pDevice_->CreateRenderTargetView(pBuffer, NULL, &pRenderTargetView_);
    SAFE_RELEASE(pBuffer);
    if (!SUCCEEDED(result))
        return false;

    return true;
}

void Renderer::Cleanup() {
    if (pDeviceContext_ != NULL)
        pDeviceContext_->ClearState();

    SAFE_RELEASE(pRenderTargetView_);
    SAFE_RELEASE(pDeviceContext_);
    SAFE_RELEASE(pSwapChain_);

    if (pDevice_ != NULL) {
        ID3D11Debug* d3dDebug = NULL;
        pDevice_->QueryInterface(IID_PPV_ARGS(&d3dDebug));

        UINT references = pDevice_->Release();
        pDevice_ = NULL;
        if (references > 1) {
            d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        }
        SAFE_RELEASE(d3dDebug);
    }
    SAFE_RELEASE(pDevice_);
}

Renderer::~Renderer() {
    Cleanup();
}