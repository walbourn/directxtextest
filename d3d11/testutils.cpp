//-------------------------------------------------------------------------------------
// testutils.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

//#define D3D_DEBUG

#include <assert.h>

#include "directxtest.h"

#include <wrl/client.h>

#include "DirectXTex.h"

#include <exception>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wswitch"
#endif

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define RENDER_TEST_MS_DELAY 5000

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480 

namespace
{
    struct SimpleVertex
    {
        XMFLOAT3 Pos;
        XMFLOAT3 Normal;
        XMFLOAT2 Tex;
    };

    struct CBNeverChanges
    {
        XMMATRIX mView;
    };

    struct CBChangeOnResize
    {
        XMMATRIX mProjection;
    };

    struct CBChangesEveryFrame
    {
        XMMATRIX mWorld;
        XMFLOAT4 vEyePosition;
        XMFLOAT4 vMeshColor;
    };

    //---------------------------------------------------------------------------------

    HWND                                g_hWnd = nullptr;

    ID3D11Device1*                      g_pd3dDevice = nullptr;
    ID3D11DeviceContext1*               g_pImmediateContext = nullptr;
    IDXGISwapChain1*                    g_pSwapChain = nullptr;
    ID3D11RenderTargetView*             g_pRenderTargetView = nullptr;
    ID3D11Texture2D*                    g_pDepthStencil = nullptr;
    ID3D11DepthStencilView*             g_pDepthStencilView = nullptr;

    ID3D11PixelShader*                  g_pPixelShader1D = nullptr;
    ID3D11PixelShader*                  g_pPixelShader2D = nullptr;
    ID3D11PixelShader*                  g_pPixelShader3D = nullptr;
    ID3D11PixelShader*                  g_pPixelShaderCube = nullptr;

    ID3D11VertexShader*                 g_pVertexShader2D = nullptr;
    ID3D11VertexShader*                 g_pVertexShader3D = nullptr;
    ID3D11VertexShader*                 g_pVertexShaderCube = nullptr;

    ID3D11InputLayout*                  g_pVertexLayout = nullptr;
    ID3D11Buffer*                       g_pVertexBuffer = nullptr;
    ID3D11Buffer*                       g_pIndexBuffer = nullptr;
    ID3D11Buffer*                       g_pCBNeverChanges = nullptr;
    ID3D11Buffer*                       g_pCBChangeOnResize = nullptr;
    ID3D11Buffer*                       g_pCBChangesEveryFrame = nullptr;
    ID3D11SamplerState*                 g_pSamplerLinear = nullptr;

    XMMATRIX                            g_World;
    XMMATRIX                            g_View;
    XMMATRIX                            g_Projection;
    XMFLOAT4                            g_vEyePosition(0.0f, 3.0f, -6.0f, 0.0f);
    XMFLOAT4                            g_vMeshColor(0.7f, 0.7f, 0.7f, 1.0f);

    //---------------------------------------------------------------------------------

    #include "shaders\vs2D.h"
    #include "shaders\ps1D.h"
    #include "shaders\ps2D.h"
    #include "shaders\vsCube.h"
    #include "shaders\psCube.h"
    #include "shaders\vs3D.h"
    #include "shaders\ps3D.h"

    //---------------------------------------------------------------------------------

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (message)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    //---------------------------------------------------------------------------------
    const D3D_DRIVER_TYPE g_driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };

    const D3D_FEATURE_LEVEL g_featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
}


//-------------------------------------------------------------------------------------
HRESULT CreateDevice(ID3D11Device** pDev, ID3D11DeviceContext** pContext)
{
    HRESULT hr = E_FAIL;

    UINT createDeviceFlags = 0;
#ifdef D3D_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    for (UINT driverTypeIndex = 0; driverTypeIndex < _countof(g_driverTypes); driverTypeIndex++)
    {
        hr = D3D11CreateDevice(nullptr, g_driverTypes[driverTypeIndex], nullptr, createDeviceFlags,
            g_featureLevels, _countof(g_featureLevels),
            D3D11_SDK_VERSION, pDev, nullptr, pContext);
        if (SUCCEEDED(hr))
            break;
    }

    return hr;
}

//-------------------------------------------------------------------------------------
HRESULT SetupRenderTest(ID3D11Device** pDev, ID3D11DeviceContext** pContext)
{
    if (g_hWnd)
        return E_FAIL;

    //--- Create window ---------------------------------------------------------------
    static bool s_classExists;
    if (!s_classExists)
    {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
        wcex.lpfnWndProc = WndProc;
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"DirectXTexClass";
        if (!RegisterClassExW(&wcex))
            return E_FAIL;

        s_classExists = true;
    }

    RECT rc = { 0, 0, RENDER_WIDTH, RENDER_HEIGHT };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindowW(L"DirectXTexClass", L"DirectXTex (D3D11)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, nullptr, nullptr);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);

    //--- Setup device ----------------------------------------------------------------
    HRESULT hr = E_FAIL;

    UINT createDeviceFlags = 0;
#ifdef D3D_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    for (UINT driverTypeIndex = 0; driverTypeIndex < _countof(g_driverTypes); driverTypeIndex++)
    {
        hr = D3D11CreateDevice(nullptr, g_driverTypes[driverTypeIndex], nullptr, createDeviceFlags,
            g_featureLevels, _countof(g_featureLevels),
            D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, context.GetAddressOf());
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    hr = device.Get()->QueryInterface(IID_PPV_ARGS(&g_pd3dDevice));
    if (FAILED(hr))
        return hr;

    hr = context.Get()->QueryInterface(IID_PPV_ARGS(&g_pImmediateContext));
    if (FAILED(hr))
        return hr;

    // Create swapchain
    {
        ComPtr<IDXGIDevice1> dxgiDevice;
        hr = device.As(&dxgiDevice);
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIAdapter> dxgiAdapter;
        hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIFactory1> dxgiFactory;
        hr = dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (FAILED(hr))
            return hr;

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = RENDER_WIDTH;
        sd.Height = RENDER_HEIGHT;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 2;
        sd.SampleDesc.Count = 1;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fs = {};
        fs.Windowed = TRUE;

        hr = dxgiFactory2->CreateSwapChainForHwnd(device.Get(), g_hWnd, &sd, &fs, nullptr, &g_pSwapChain);
        if (FAILED(hr))
            return hr;

        hr = dxgiFactory2->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr))
            return hr;
    }

    // Create a render target view
    ComPtr<ID3D11Texture2D> pBackBuffer;
    hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &g_pRenderTargetView);
    if (FAILED(hr))
        return hr;

    // Create depth stencil texture
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = RENDER_WIDTH;
        desc.Height = RENDER_HEIGHT;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        hr = g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_pDepthStencil);
        if (FAILED(hr))
            return hr;

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = desc.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
        if (FAILED(hr))
            return hr;
    }

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)RENDER_WIDTH;
    vp.Height = (FLOAT)RENDER_HEIGHT;
    vp.MaxDepth = 1.0f;
    g_pImmediateContext->RSSetViewports(1, &vp);

    //--- Setup render cube -----------------------------------------------------------

    // Create the vertex shaders
    hr = g_pd3dDevice->CreateVertexShader(g_VS2D, sizeof(g_VS2D), nullptr, &g_pVertexShader2D);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateVertexShader(g_VS3D, sizeof(g_VS3D), nullptr, &g_pVertexShader3D);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateVertexShader(g_VSCube, sizeof(g_VSCube), nullptr, &g_pVertexShaderCube);
    if (FAILED(hr))
        return hr;

    // Define the input layout
    static const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Create the input layout (assumes all shaders share the same layout)
    hr = g_pd3dDevice->CreateInputLayout(layout, _countof(layout), g_VS2D, sizeof(g_VS2D), &g_pVertexLayout);
    if (FAILED(hr))
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    // Create the pixel shaders
    hr = g_pd3dDevice->CreatePixelShader(g_PS1D, sizeof(g_PS1D), nullptr, &g_pPixelShader1D);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreatePixelShader(g_PS2D, sizeof(g_PS2D), nullptr, &g_pPixelShader2D);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreatePixelShader(g_PS3D, sizeof(g_PS3D), nullptr, &g_pPixelShader3D);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreatePixelShader(g_PSCube, sizeof(g_PSCube), nullptr, &g_pPixelShaderCube);
    if (FAILED(hr))
        return hr;

    // Create vertex buffer
    static const SimpleVertex s_vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    };

    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(SimpleVertex) * 24;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = s_vertices;
        hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
        if (FAILED(hr))
            return hr;
    }

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    // Create index buffer
    static const WORD s_indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(WORD) * 36;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = s_indices;
        hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
        if (FAILED(hr))
            return hr;
    }

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create the constant buffers
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CBNeverChanges);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBNeverChanges);
        if (FAILED(hr))
            return hr;

        bd.ByteWidth = sizeof(CBChangeOnResize);
        hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangeOnResize);
        if (FAILED(hr))
            return hr;

        bd.ByteWidth = sizeof(CBChangesEveryFrame);
        hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame);
        if (FAILED(hr))
            return hr;
    }

    // Create the sample state
    {
        D3D11_SAMPLER_DESC desc = {};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        hr = g_pd3dDevice->CreateSamplerState(&desc, &g_pSamplerLinear);
        if (FAILED(hr))
            return hr;
    }

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    g_View = XMMatrixLookAtLH(XMLoadFloat4(&g_vEyePosition), At, Up);

    CBNeverChanges cbNeverChanges = {};
    cbNeverChanges.mView = XMMatrixTranspose(g_View);
    g_pImmediateContext->UpdateSubresource(g_pCBNeverChanges, 0, nullptr, &cbNeverChanges, 0, 0);

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, RENDER_WIDTH / (FLOAT)RENDER_HEIGHT, 0.01f, 100.0f);

    CBChangeOnResize cbChangesOnResize = {};
    cbChangesOnResize.mProjection = XMMatrixTranspose(g_Projection);
    g_pImmediateContext->UpdateSubresource(g_pCBChangeOnResize, 0, nullptr, &cbChangesOnResize, 0, 0);

    if (pDev)
    {
        *pDev = g_pd3dDevice;
        g_pd3dDevice->AddRef();
    }

    if (pContext)
    {
        *pContext = g_pImmediateContext;
        g_pImmediateContext->AddRef();
    }

    return S_OK;
}


//-------------------------------------------------------------------------------------
namespace
{
    void Render()
    {
        // Update our time
        static float t = 0.0f;
        static ULONGLONG dwTimeStart = 0;
        ULONGLONG dwTimeCur = GetTickCount64();
        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;
        t = (dwTimeCur - dwTimeStart) / 1000.0f;

        // Rotate cube around the origin
        g_World = XMMatrixRotationY(t);

        //
        // Clear the back buffer
        //
        float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

        //
        // Clear the depth buffer to 1.0 (max depth)
        //
        g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        //
        // Update variables that change once per frame
        //
        CBChangesEveryFrame cb = {};
        cb.mWorld = XMMatrixTranspose(g_World);
        cb.vEyePosition = g_vEyePosition;
        cb.vMeshColor = g_vMeshColor;
        g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);

        //
        // Render the cube
        //
        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
        g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
        g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
        g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
        g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
        g_pImmediateContext->DrawIndexed(36, 0, 0);

        //
        // Present our back buffer to our front buffer
        //
        HRESULT hr = g_pSwapChain->Present(0, 0);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            throw std::exception("Present");
        }
    }
}

//-------------------------------------------------------------------------------------
void RenderTest(const TexMetadata& metadata, ID3D11ShaderResourceView* pSRV)
{
    switch (metadata.dimension)
    {
    case TEX_DIMENSION_TEXTURE1D:
        g_pImmediateContext->VSSetShader(g_pVertexShader2D, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pPixelShader1D, nullptr, 0);
        break;

    case TEX_DIMENSION_TEXTURE3D:
        g_pImmediateContext->VSSetShader(g_pVertexShader3D, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pPixelShader3D, nullptr, 0);
        break;

    case TEX_DIMENSION_TEXTURE2D:
        if (metadata.miscFlags & TEX_MISC_TEXTURECUBE)
        {
            g_pImmediateContext->VSSetShader(g_pVertexShaderCube, nullptr, 0);
            g_pImmediateContext->PSSetShader(g_pPixelShaderCube, nullptr, 0);
        }
        else
        {
            g_pImmediateContext->VSSetShader(g_pVertexShader2D, nullptr, 0);
            g_pImmediateContext->PSSetShader(g_pPixelShader2D, nullptr, 0);
        }
        break;

    default:
        return;
    }

    if (pSRV)
        g_pImmediateContext->PSSetShaderResources(0, 1, &pSRV);

    // Main message loop
    MSG msg = {};

    ULONGLONG end = GetTickCount64() + RENDER_TEST_MS_DELAY;

    for (;;)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }

        if (GetTickCount64() > end)
            break;
    }

#pragma warning(push)
#pragma warning(disable : 6309)
#pragma warning(disable : 6387)
    if (pSRV)
        g_pImmediateContext->PSSetShaderResources(0, 0, nullptr);
#pragma warning(pop)
}


//-------------------------------------------------------------------------------------

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p)=nullptr; } }

void CleanupRenderTest()
{
    if (g_pImmediateContext)
    {
        g_pImmediateContext->ClearState();
        g_pImmediateContext->Flush();
    }

    SAFE_RELEASE(g_pSamplerLinear);
    SAFE_RELEASE(g_pCBNeverChanges);
    SAFE_RELEASE(g_pCBChangeOnResize);
    SAFE_RELEASE(g_pCBChangesEveryFrame);
    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pIndexBuffer);
    SAFE_RELEASE(g_pVertexLayout);

    SAFE_RELEASE(g_pVertexShader2D);
    SAFE_RELEASE(g_pVertexShader3D);
    SAFE_RELEASE(g_pVertexShaderCube);

    SAFE_RELEASE(g_pPixelShader1D);
    SAFE_RELEASE(g_pPixelShader2D);
    SAFE_RELEASE(g_pPixelShader3D);
    SAFE_RELEASE(g_pPixelShaderCube);

    SAFE_RELEASE(g_pDepthStencil);
    SAFE_RELEASE(g_pDepthStencilView);
    SAFE_RELEASE(g_pRenderTargetView);
    SAFE_RELEASE(g_pSwapChain);
    SAFE_RELEASE(g_pImmediateContext);
    SAFE_RELEASE(g_pd3dDevice);

    if (g_hWnd)
    {
        CloseWindow(g_hWnd);
        g_hWnd = nullptr;
    }
}
