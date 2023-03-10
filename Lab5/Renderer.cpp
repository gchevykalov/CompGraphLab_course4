#include "Renderer.h"
#include <iostream>

#define SAFE_RELEASE(A) if ((A) != NULL) { (A)->Release(); (A) = NULL; }

Renderer& Renderer::GetInstance() {
    static Renderer instance;
    return instance;
}

Renderer::Renderer() :
    pDevice_(NULL),
    pDeviceContext_(NULL),
    pSwapChain_(NULL),
    pRenderTargetView_(NULL),
    pRasterizerState_(NULL),
    pSampler_(NULL),
    pCamera_(NULL),
    pInput_(NULL),
    pDepthBuffer_(NULL),
    pDepthBufferDSV_(NULL),
    pBlendState_(NULL),
    width_(defaultWidth),
    height_(defaultHeight),
    numSphereTriangles_(0),
    radius_(1.0) {}

bool Renderer::Init(HINSTANCE hInstance, HWND hWnd) {
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
    if (pSelectedAdapter == NULL) {
        SAFE_RELEASE(pFactory);
        return false;
    }

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
    if (D3D_FEATURE_LEVEL_11_0 != level || !SUCCEEDED(result)) {
        SAFE_RELEASE(pFactory);
        SAFE_RELEASE(pSelectedAdapter);
        Cleanup();
        return false;
    }

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

    ID3D11Texture2D* pBackBuffer = NULL;
    if (SUCCEEDED(result)) {
        result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    }
    if (SUCCEEDED(result)) {
        result = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView_);
    }
    if (SUCCEEDED(result)) {
        result = InitScene();
    }
    SAFE_RELEASE(pFactory);
    SAFE_RELEASE(pSelectedAdapter);
    SAFE_RELEASE(pBackBuffer);
    if (SUCCEEDED(result)) {
        pCamera_ = new Camera;
        if (!pCamera_) {
            result = S_FALSE;
        }
    }
    if (SUCCEEDED(result)) {
        pInput_ = new Input;
        if (!pInput_) {
            result = S_FALSE;
        }
    }
    if (SUCCEEDED(result)) {
        result = pInput_->Init(hInstance, hWnd);
    }
    if (FAILED(result)) {
        Cleanup();
    }

    return SUCCEEDED(result);
}

HRESULT Renderer::InitScene() {
    HRESULT result;

    static const Vertex Vertices[] = {
        {-1.0, -1.0,  1.0, 0, 1},
        { 1.0, -1.0,  1.0, 1, 1},
        { 1.0, -1.0, -1.0, 1, 0},
        {-1.0, -1.0, -1.0, 0, 0},

        {-1.0,  1.0, -1.0, 0, 1},
        { 1.0,  1.0, -1.0, 1, 1},
        { 1.0,  1.0,  1.0, 1, 0},
        {-1.0,  1.0,  1.0, 0, 0},

        { 1.0, -1.0, -1.0, 0, 1},
        { 1.0, -1.0,  1.0, 1, 1},
        { 1.0,  1.0,  1.0, 1, 0},
        { 1.0,  1.0, -1.0, 0, 0},

        {-1.0, -1.0,  1.0, 0, 1},
        {-1.0, -1.0, -1.0, 1, 1},
        {-1.0,  1.0, -1.0, 1, 0},
        {-1.0,  1.0,  1.0, 0, 0},

        { 1.0, -1.0,  1.0, 0, 1},
        {-1.0, -1.0,  1.0, 1, 1},
        {-1.0,  1.0,  1.0, 1, 0},
        { 1.0,  1.0,  1.0, 0, 0},

        {-1.0, -1.0, -1.0, 0, 1},
        { 1.0, -1.0, -1.0, 1, 1},
        { 1.0,  1.0, -1.0, 1, 0},
        {-1.0,  1.0, -1.0, 0, 0}
    };
    static const USHORT Indices[] = {
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
    };
    static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    static const TransparentVertex VerticesT[] = {
        {0, -1, -1, RGB(0, 255, 255)},
        {0,  1, -1, RGB(255, 255, 0)},
        {0,  1,  1, RGB(0, 255, 255)},
        {0, -1,  1, RGB(255, 255, 0)}
    };
    static const USHORT IndicesT[] = {
        0, 2, 1, 0, 3, 2
    };
    static const D3D11_INPUT_ELEMENT_DESC InputDescT[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0,  DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    UINT LatLines = 20, LongLines = 20;
    UINT numSphereVertices = ((LatLines - 2) * LongLines) + 2;
    numSphereTriangles_ = ((LatLines - 3) * (LongLines) * 2) + (LongLines * 2);

    float phi = 0.0f;
    float theta = 0.0f;

    std::vector<SkyboxVertex> vertices(numSphereVertices);

    XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    vertices[0].x = 0.0f;
    vertices[0].y = 0.0f;
    vertices[0].z = 1.0f;

    for (UINT i = 0; i < LatLines - 2; i++) {
        theta = (i + 1) * (XM_PI / (LatLines - 1));
        XMMATRIX Rotationx = XMMatrixRotationX(theta);
        for (UINT j = 0; j < LongLines; j++) {
            phi = j * (XM_2PI / LongLines);
            XMMATRIX Rotationy = XMMatrixRotationZ(phi);
            currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
            currVertPos = XMVector3Normalize(currVertPos);
            vertices[i * (__int64)LongLines + j + 1].x = XMVectorGetX(currVertPos);
            vertices[i * (__int64)LongLines + j + 1].y = XMVectorGetY(currVertPos);
            vertices[i * (__int64)LongLines + j + 1].z = XMVectorGetZ(currVertPos);
        }
    }

    vertices[(__int64)numSphereVertices - 1].x = 0.0f;
    vertices[(__int64)numSphereVertices - 1].y = 0.0f;
    vertices[(__int64)numSphereVertices - 1].z = -1.0f;

    std::vector<UINT> indices((__int64)numSphereTriangles_ * 3);

    UINT k = 0;
    for (UINT i = 0; i < LongLines - 1; i++) {
        indices[k] = 0;
        indices[(__int64)k + 2] = i + 1;
        indices[(__int64)k + 1] = i + 2;
        k += 3;
    }
    indices[k] = 0;
    indices[(__int64)k + 2] = LongLines;
    indices[(__int64)k + 1] = 1;
    k += 3;

    for (UINT i = 0; i < LatLines - 3; i++) {
        for (UINT j = 0; j < LongLines - 1; j++) {
            indices[k] = i * LongLines + j + 1;
            indices[(__int64)k + 1] = i * LongLines + j + 2;
            indices[(__int64)k + 2] = (i + 1) * LongLines + j + 1;

            indices[(__int64)k + 3] = (i + 1) * LongLines + j + 1;
            indices[(__int64)k + 4] = i * LongLines + j + 2;
            indices[(__int64)k + 5] = (i + 1) * LongLines + j + 2;

            k += 6;
        }

        indices[k] = (i * LongLines) + LongLines;
        indices[(__int64)k + 1] = (i * LongLines) + 1;
        indices[(__int64)k + 2] = ((i + 1) * LongLines) + LongLines;

        indices[(__int64)k + 3] = ((i + 1) * LongLines) + LongLines;
        indices[(__int64)k + 4] = (i * LongLines) + 1;
        indices[(__int64)k + 5] = ((i + 1) * LongLines) + 1;

        k += 6;
    }

    for (UINT i = 0; i < LongLines - 1; i++) {
        indices[k] = numSphereVertices - 1;
        indices[(__int64)k + 2] = (numSphereVertices - 1) - (i + 1);
        indices[(__int64)k + 1] = (numSphereVertices - 1) - (i + 2);
        k += 3;
    }

    indices[k] = numSphereVertices - 1;
    indices[(__int64)k + 2] = (numSphereVertices - 1) - LongLines;
    indices[(__int64)k + 1] = numSphereVertices - 2;

    static const D3D11_INPUT_ELEMENT_DESC SkyboxInputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(Vertices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &Vertices;
    data.SysMemPitch = sizeof(Vertices);
    data.SysMemSlicePitch = 0;

    result = pDevice_->CreateBuffer(&desc, &data, &pVertexBuffer_[0]);

    if (SUCCEEDED(result)) {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(Indices);
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = &Indices;
        data.SysMemPitch = sizeof(Indices);
        data.SysMemSlicePitch = 0;

        result = pDevice_->CreateBuffer(&desc, &data, &pIndexBuffer_[0]);
    }

    ID3D10Blob* vertexShaderBuffer = nullptr;
    ID3D10Blob* pixelShaderBuffer = nullptr;
    int flags = 0;
#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if (SUCCEEDED(result)) {
        result = D3DCompileFromFile(L"VS.hlsl", NULL, NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
        if (SUCCEEDED(result)) {
            result = pDevice_->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader_[0]);
        }
    }
    if (SUCCEEDED(result)) {
        result = D3DCompileFromFile(L"PS.hlsl", NULL, NULL, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
        if (SUCCEEDED(result)) {
            result = pDevice_->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader_[0]);
        }
    }
    if (SUCCEEDED(result)) {
        int numElements = sizeof(InputDesc) / sizeof(InputDesc[0]);
        result = pDevice_->CreateInputLayout(InputDesc, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout_[0]);
    }

    SAFE_RELEASE(vertexShaderBuffer);
    SAFE_RELEASE(pixelShaderBuffer);

    if (SUCCEEDED(result)) {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(WorldMatrixBuffer);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        WorldMatrixBuffer worldMatrixBuffer;
        worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = &worldMatrixBuffer;
        data.SysMemPitch = sizeof(worldMatrixBuffer);
        data.SysMemSlicePitch = 0;

        result = pDevice_->CreateBuffer(&desc, &data, &pWorldMatrixBuffer_[0]);
        if (SUCCEEDED(result)) {
            worldMatrixBuffer.worldMatrix = DirectX::XMMatrixTranslation(4.0f, 0.0f, 0.0f);
            result = pDevice_->CreateBuffer(&desc, &data, &pWorldMatrixBuffer_[1]);
        }
        if (SUCCEEDED(result)) {
            worldMatrixBuffer.worldMatrix = DirectX::XMMatrixTranslation(1.8f, 0.0f, 0.0f);
            result = pDevice_->CreateBuffer(&desc, &data, &pWorldMatrixBuffer_[3]);
        }
        if (SUCCEEDED(result)) {
            worldMatrixBuffer.worldMatrix = DirectX::XMMatrixTranslation(2.2f, 0.0f, 0.0f);
            result = pDevice_->CreateBuffer(&desc, &data, &pWorldMatrixBuffer_[4]);
        }
    }
    if (SUCCEEDED(result)) {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(ViewMatrixBuffer);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        result = pDevice_->CreateBuffer(&desc, nullptr, &pViewMatrixBuffer_[0]);
    }
    {
        if (SUCCEEDED(result)) {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(SkyboxVertex) * numSphereVertices;
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA data;
            ZeroMemory(&data, sizeof(data));
            data.pSysMem = &vertices[0];
            result = pDevice_->CreateBuffer(&desc, &data, &pVertexBuffer_[1]);
        }
        if (SUCCEEDED(result)) {
            D3D11_BUFFER_DESC desc = {};
            ZeroMemory(&desc, sizeof(desc));
            desc.ByteWidth = sizeof(UINT) * numSphereTriangles_ * 3;
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA data;
            data.pSysMem = &indices[0];

            result = pDevice_->CreateBuffer(&desc, &data, &pIndexBuffer_[1]);
        }

        ID3D10Blob* vertexShaderBuffer = nullptr;
        ID3D10Blob* pixelShaderBuffer = nullptr;
        int flags = 0;
#ifdef _DEBUG
        flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        if (SUCCEEDED(result)) {
            result = D3DCompileFromFile(L"CubeMapVS.hlsl", NULL, NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
            if (SUCCEEDED(result)) {
                result = pDevice_->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader_[1]);
            }
        }
        if (SUCCEEDED(result)) {
            result = D3DCompileFromFile(L"CubeMapPS.hlsl", NULL, NULL, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
            if (SUCCEEDED(result)) {
                result = pDevice_->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader_[1]);
            }
        }
        if (SUCCEEDED(result)) {
            int numElements = sizeof(SkyboxInputDesc) / sizeof(SkyboxInputDesc[0]);
            result = pDevice_->CreateInputLayout(SkyboxInputDesc, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout_[1]);
        }

        SAFE_RELEASE(vertexShaderBuffer);
        SAFE_RELEASE(pixelShaderBuffer);

        if (SUCCEEDED(result)) {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(SkyboxWorldMatrixBuffer);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            SkyboxWorldMatrixBuffer skyboxWorldMatrixBuffer;

            skyboxWorldMatrixBuffer.worldMatrix = XMMatrixIdentity();
            skyboxWorldMatrixBuffer.size = XMFLOAT4(radius_, 0.0f, 0.0f, 0.0f);

            D3D11_SUBRESOURCE_DATA data;
            data.pSysMem = &skyboxWorldMatrixBuffer;
            data.SysMemPitch = sizeof(skyboxWorldMatrixBuffer);
            data.SysMemSlicePitch = 0;

            result = pDevice_->CreateBuffer(&desc, &data, &pWorldMatrixBuffer_[2]);
        }
        if (SUCCEEDED(result)) {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(SkyboxViewMatrixBuffer);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            result = pDevice_->CreateBuffer(&desc, nullptr, &pViewMatrixBuffer_[1]);
        }
    }
    {
        if (SUCCEEDED(result)) {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(VerticesT);
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA data;
            data.pSysMem = &VerticesT;
            data.SysMemPitch = sizeof(VerticesT);
            data.SysMemSlicePitch = 0;

            result = pDevice_->CreateBuffer(&desc, &data, &pVertexBuffer_[2]);
        }
        if (SUCCEEDED(result)) {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(IndicesT);
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA data;
            data.pSysMem = &IndicesT;
            data.SysMemPitch = sizeof(IndicesT);
            data.SysMemSlicePitch = 0;

            result = pDevice_->CreateBuffer(&desc, &data, &pIndexBuffer_[2]);
        }

        ID3D10Blob* vertexShaderBuffer = nullptr;
        ID3D10Blob* pixelShaderBuffer = nullptr;
        int flags = 0;
#ifdef _DEBUG
        flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        if (SUCCEEDED(result)) {
            result = D3DCompileFromFile(L"TVS.hlsl", NULL, NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
            if (SUCCEEDED(result)) {
                result = pDevice_->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader_[2]);
            }
        }
        if (SUCCEEDED(result)) {
            result = D3DCompileFromFile(L"TPS.hlsl", NULL, NULL, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
            if (SUCCEEDED(result)) {
                result = pDevice_->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader_[2]);
            }
        }
        if (SUCCEEDED(result)) {
            int numElements = sizeof(InputDescT) / sizeof(InputDescT[0]);
            result = pDevice_->CreateInputLayout(InputDescT, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout_[2]);
        }

        SAFE_RELEASE(vertexShaderBuffer);
        SAFE_RELEASE(pixelShaderBuffer);
    }
    if (SUCCEEDED(result)) {
        D3D11_RASTERIZER_DESC desc = {};
        desc.AntialiasedLineEnable = false;
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;
        desc.ScissorEnable = false;
        desc.MultisampleEnable = false;
        desc.SlopeScaledDepthBias = 0.0f;

        result = pDevice_->CreateRasterizerState(&desc, &pRasterizerState_);
    }
    if (SUCCEEDED(result)) {
        result = CreateDDSTextureFromFile(pDevice_, pDeviceContext_, L"textures/tea.dds", nullptr, &pTexture_[0]);
    }
    if (SUCCEEDED(result)) {
        result = CreateDDSTextureFromFileEx(pDevice_, pDeviceContext_, L"textures/cube.dds",
            0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE,
            DDS_LOADER_DEFAULT, nullptr, &pTexture_[1]);
    }
    if (SUCCEEDED(result)) {
        D3D11_SAMPLER_DESC desc = {};

        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MinLOD = -D3D11_FLOAT32_MAX;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        desc.MipLODBias = 0.0f;
        desc.MaxAnisotropy = 16;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 1.0f;

        result = pDevice_->CreateSamplerState(&desc, &pSampler_);
    }
    if (SUCCEEDED(result)) {
        D3D11_DEPTH_STENCIL_DESC dsDesc = { };
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
        dsDesc.StencilEnable = FALSE;

        result = pDevice_->CreateDepthStencilState(&dsDesc, &pDepthState_[0]);
    }
    if (SUCCEEDED(result)) {
        D3D11_DEPTH_STENCIL_DESC dsDesc = { };
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        dsDesc.StencilEnable = FALSE;

        result = pDevice_->CreateDepthStencilState(&dsDesc, &pDepthState_[1]);
    }
    if (SUCCEEDED(result)) {
        D3D11_BLEND_DESC desc = { 0 };
        desc.AlphaToCoverageEnable = false;
        desc.IndependentBlendEnable = false;
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED |
            D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;

        result = pDevice_->CreateBlendState(&desc, &pBlendState_);
    }

    return result;
}

void Renderer::InputHandler() {
    XMFLOAT3 mouse = pInput_->ReadMouse();
    pCamera_->Rotate(mouse.x / 200.0f, mouse.y / 200.0f);
    pCamera_->Zoom(-mouse.z / 100.0f);

    XMFLOAT4 keyboard = pInput_->ReadKeyboard();
    float di = 0.0, dj = 0.0;
    if (keyboard.x > 0.0)
        dj += 0.005f;
    if (keyboard.y > 0.0)
        dj -= 0.005f;
    if (keyboard.z > 0.0)
        di += 0.005f;
    if (keyboard.w > 0.0)
        di -= 0.005f;
    pCamera_->Move(di, dj);
}

bool Renderer::UpdateScene() {
    HRESULT result;

    InputHandler();

    static float t = 0.0f;
    static ULONGLONG timeStart = 0;
    ULONGLONG timeCur = GetTickCount64();
    if (timeStart == 0) {
        timeStart = timeCur;
    }
    t = (timeCur - timeStart) / 1000.0f;

    WorldMatrixBuffer worldMatrixBuffer;
    worldMatrixBuffer.worldMatrix = XMMatrixRotationY(t);

    pDeviceContext_->UpdateSubresource(pWorldMatrixBuffer_[0], 0, nullptr, &worldMatrixBuffer, 0, 0);

    XMMATRIX mView = pCamera_->GetViewMatrix();

    XMMATRIX mProjection = XMMatrixPerspectiveFovLH(XM_PI / 3, width_ / (FLOAT)height_, 100.0f, 0.01f);

    D3D11_MAPPED_SUBRESOURCE subresource, skyboxSubresource;
    result = pDeviceContext_->Map(pViewMatrixBuffer_[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    if (SUCCEEDED(result)) {
        ViewMatrixBuffer& sceneBuffer = *reinterpret_cast<ViewMatrixBuffer*>(subresource.pData);
        sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(mView, mProjection);
        pDeviceContext_->Unmap(pViewMatrixBuffer_[0], 0);
    }
    if (SUCCEEDED(result)) {
        SkyboxWorldMatrixBuffer skyboxWorldMatrixBuffer;

        skyboxWorldMatrixBuffer.worldMatrix = XMMatrixIdentity();
        skyboxWorldMatrixBuffer.size = XMFLOAT4(radius_, 0.0f, 0.0f, 0.0f);

        pDeviceContext_->UpdateSubresource(pWorldMatrixBuffer_[2], 0, nullptr, &skyboxWorldMatrixBuffer, 0, 0);

        result = pDeviceContext_->Map(pViewMatrixBuffer_[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &skyboxSubresource);
    }
    if (SUCCEEDED(result)) {
        SkyboxViewMatrixBuffer& skyboxSceneBuffer = *reinterpret_cast<SkyboxViewMatrixBuffer*>(skyboxSubresource.pData);
        skyboxSceneBuffer.viewProjectionMatrix = XMMatrixMultiply(mView, mProjection);
        XMFLOAT3 cameraPos = pCamera_->GetPosition();
        skyboxSceneBuffer.cameraPos = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
        pDeviceContext_->Unmap(pViewMatrixBuffer_[1], 0);
    }

    return SUCCEEDED(result);
}

bool Renderer::Render() {
    if (!UpdateScene())
        return false;

    pDeviceContext_->ClearState();

    ID3D11RenderTargetView* views[] = { pRenderTargetView_ };
    pDeviceContext_->OMSetRenderTargets(1, views, pDepthBufferDSV_);

    static const FLOAT backColor[4] = { 0.4f, 0.2f, 0.4f, 1.0f };
    pDeviceContext_->ClearRenderTargetView(pRenderTargetView_, backColor);
    pDeviceContext_->ClearDepthStencilView(pDepthBufferDSV_, D3D11_CLEAR_DEPTH, 0.0f, 0);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)width_;
    viewport.Height = (FLOAT)height_;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pDeviceContext_->RSSetViewports(1, &viewport);

    D3D11_RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = width_;
    rect.bottom = height_;
    pDeviceContext_->RSSetScissorRects(1, &rect);

    pDeviceContext_->RSSetState(pRasterizerState_);
    pDeviceContext_->OMSetDepthStencilState(pDepthState_[0], 0);

    ID3D11ShaderResourceView* resources[] = { pTexture_[0] };
    pDeviceContext_->PSSetShaderResources(0, 1, resources);

    ID3D11SamplerState* samplers[] = { pSampler_ };
    pDeviceContext_->PSSetSamplers(0, 1, samplers);

    pDeviceContext_->IASetIndexBuffer(pIndexBuffer_[0], DXGI_FORMAT_R16_UINT, 0);
    ID3D11Buffer* vertexBuffers[] = { pVertexBuffer_[0] };
    UINT strides[] = { 20 };
    UINT offsets[] = { 0 };
    pDeviceContext_->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    pDeviceContext_->IASetInputLayout(pInputLayout_[0]);
    pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer_[0]);
    pDeviceContext_->VSSetConstantBuffers(1, 1, &pViewMatrixBuffer_[0]);
    pDeviceContext_->VSSetShader(pVertexShader_[0], nullptr, 0);
    pDeviceContext_->PSSetShader(pPixelShader_[0], nullptr, 0);
    pDeviceContext_->DrawIndexed(36, 0, 0);
    pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer_[1]);
    pDeviceContext_->DrawIndexed(36, 0, 0);

    pDeviceContext_->OMSetDepthStencilState(pDepthState_[1], 0);
    {
        ID3D11ShaderResourceView* resources[] = { pTexture_[1] };
        pDeviceContext_->PSSetShaderResources(0, 1, resources);

        pDeviceContext_->IASetIndexBuffer(pIndexBuffer_[1], DXGI_FORMAT_R32_UINT, 0);
        ID3D11Buffer* vertexBuffers[] = { pVertexBuffer_[1] };
        UINT strides[] = { 12 };
        UINT offsets[] = { 0 };
        pDeviceContext_->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
        pDeviceContext_->IASetInputLayout(pInputLayout_[1]);
        pDeviceContext_->VSSetShader(pVertexShader_[1], nullptr, 0);
        pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer_[2]);
        pDeviceContext_->VSSetConstantBuffers(1, 1, &pViewMatrixBuffer_[1]);
        pDeviceContext_->PSSetShader(pPixelShader_[1], nullptr, 0);

        pDeviceContext_->DrawIndexed(numSphereTriangles_ * 3, 0, 0);
    }

    {
        pDeviceContext_->IASetIndexBuffer(pIndexBuffer_[2], DXGI_FORMAT_R16_UINT, 0);
        ID3D11Buffer* vertexBuffers[] = { pVertexBuffer_[2] };
        UINT strides[] = { 16 };
        UINT offsets[] = { 0 };
        pDeviceContext_->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
        pDeviceContext_->IASetInputLayout(pInputLayout_[2]);

        pDeviceContext_->VSSetShader(pVertexShader_[2], nullptr, 0);
        pDeviceContext_->PSSetShader(pPixelShader_[2], nullptr, 0);
        pDeviceContext_->VSSetConstantBuffers(1, 1, &pViewMatrixBuffer_[0]);

        pDeviceContext_->OMSetBlendState(pBlendState_, nullptr, 0xFFFFFFFF);

        pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer_[3]);
        pDeviceContext_->DrawIndexed(6, 0, 0);

        pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer_[4]);
        pDeviceContext_->DrawIndexed(6, 0, 0);
    }
    HRESULT result = pSwapChain_->Present(0, 0);

    return SUCCEEDED(result);
}

bool Renderer::Resize(UINT width, UINT height) {
    if (pSwapChain_ == NULL)
        return false;

    SAFE_RELEASE(pRenderTargetView_);

    width_ = max(width, 8);
    height_ = max(height, 8);

    auto result = pSwapChain_->ResizeBuffers(2, width_, height_, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
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

    SAFE_RELEASE(pDepthBuffer_);
    SAFE_RELEASE(pDepthBufferDSV_);
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.ArraySize = 1;
    desc.MipLevels = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Height = height_;
    desc.Width = width_;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    result = pDevice_->CreateTexture2D(&desc, NULL, &pDepthBuffer_);
    if (!SUCCEEDED(result))
        return false;

    result = pDevice_->CreateDepthStencilView(pDepthBuffer_, NULL, &pDepthBufferDSV_);
    if (!SUCCEEDED(result))
        return false;

    float n = 0.01f;
    float fov = XM_PI / 3;
    float halfW = tanf(fov / 2) * n;
    float halfH = height_ / float(width_) * halfW;
    radius_ = sqrtf(n * n + halfH * halfH + halfW * halfW) * 1.1f;

    return true;
}

void Renderer::Cleanup() {
    if (pDeviceContext_ != NULL)
        pDeviceContext_->ClearState();

    SAFE_RELEASE(pRenderTargetView_);
    SAFE_RELEASE(pDeviceContext_);
    SAFE_RELEASE(pSwapChain_);
    SAFE_RELEASE(pRasterizerState_);
    SAFE_RELEASE(pSampler_);
    SAFE_RELEASE(pDepthBuffer_);
    SAFE_RELEASE(pDepthBufferDSV_);
    SAFE_RELEASE(pBlendState_);

    SAFE_RELEASE(pVertexBuffer_[0]);
    SAFE_RELEASE(pVertexBuffer_[1]);
    SAFE_RELEASE(pVertexBuffer_[2]);

    SAFE_RELEASE(pIndexBuffer_[0]);
    SAFE_RELEASE(pIndexBuffer_[1]);
    SAFE_RELEASE(pIndexBuffer_[2]);

    SAFE_RELEASE(pInputLayout_[0]);
    SAFE_RELEASE(pInputLayout_[1]);
    SAFE_RELEASE(pInputLayout_[2]);

    SAFE_RELEASE(pVertexShader_[0]);
    SAFE_RELEASE(pVertexShader_[1]);
    SAFE_RELEASE(pVertexShader_[2]);

    SAFE_RELEASE(pPixelShader_[0]);
    SAFE_RELEASE(pPixelShader_[1]);
    SAFE_RELEASE(pPixelShader_[2]);

    SAFE_RELEASE(pViewMatrixBuffer_[0]);
    SAFE_RELEASE(pViewMatrixBuffer_[1]);

    SAFE_RELEASE(pWorldMatrixBuffer_[0]);
    SAFE_RELEASE(pWorldMatrixBuffer_[1]);
    SAFE_RELEASE(pWorldMatrixBuffer_[2]);
    SAFE_RELEASE(pWorldMatrixBuffer_[3]);
    SAFE_RELEASE(pWorldMatrixBuffer_[4]);

    SAFE_RELEASE(pTexture_[0]);
    SAFE_RELEASE(pTexture_[1]);

    SAFE_RELEASE(pDepthState_[0]);
    SAFE_RELEASE(pDepthState_[1]);

    if (pCamera_) {
        delete pCamera_;
        pCamera_ = NULL;
    }
    if (pInput_) {
        delete pInput_;
        pCamera_ = NULL;
    }

#ifdef _DEBUG
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
#endif
    SAFE_RELEASE(pDevice_);
}

Renderer::~Renderer() {
    Cleanup();
}