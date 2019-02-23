//-------------------------------------------------------------------------------------
// testutils.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

//#define D3D_DEBUG

#include <assert.h>

#include "directxtest.h"

#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "directxtex.h"

#include "d3dx12.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define RENDER_TEST_MS_DELAY 5000

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480 

//-------------------------------------------------------------------------------------
namespace
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw std::exception();
        }
    }

    inline size_t AlignUp(size_t v, size_t a)
    {
        return ((v + a - 1) & ~(a-1));
    }

    enum RootParameterIndex
    {
        TextureSRV,
        CBNever,
        CBOnResize,
        CBEveryFrame,
        RootParameterCount
    };

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

    HWND                                g_hWnd = nullptr;

    ID3D12Device*                       g_pd3dDevice = nullptr;

    ID3D12CommandQueue*                 g_pd3dCommandQueue = nullptr;
    ID3D12GraphicsCommandList*          g_pd3dCommandList = nullptr;
    ID3D12CommandAllocator*             g_pd3dAllocator = nullptr;
    ID3D12Fence*                        g_pd3dFence = nullptr;
    HANDLE                              g_fenceEvent = nullptr;

    UINT64                              g_fenceValue = 0;
    UINT                                g_rtvDescriptorSize = 0;
    UINT                                g_backBufferIndex = 0;

    ID3D12Resource*                     g_pRenderTarget[2] = {};
    ID3D12Resource*                     g_pDepthStencil = nullptr;

    ID3D12DescriptorHeap*               g_pd3dHeapRTV = nullptr;
    ID3D12DescriptorHeap*               g_pd3dHeapDSV = nullptr;
    ID3D12DescriptorHeap*               g_pd3dHeapSRV = nullptr;

    IDXGIFactory4*                      g_pdxgiFactory = nullptr;
    IDXGISwapChain3*                    g_pSwapChain = nullptr;

    ID3D12RootSignature*                g_rootSig = nullptr;
    ID3D12PipelineState*                g_p1DPSO = nullptr;
    ID3D12PipelineState*                g_p2DPSO = nullptr;
    ID3D12PipelineState*                g_pCubePSO = nullptr;
    ID3D12PipelineState*                g_p3DPSO = nullptr;

    ID3D12Resource*                     g_VB = nullptr;
    ID3D12Resource*                     g_IB = nullptr;
    ID3D12Resource*                     g_CB = nullptr;

    D3D12_VERTEX_BUFFER_VIEW            g_VBView;
    D3D12_INDEX_BUFFER_VIEW             g_IBView;

    uint8_t*                            g_CBCPU = nullptr;

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
}


//-------------------------------------------------------------------------------------
HRESULT CreateDevice( ID3D12Device** pDev )
{
    HRESULT hr = E_FAIL;

#if defined(D3D_DEBUG)
    // Enable the debug layer (only available if the Graphics Tools feature-on-demand is enabled).
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
        }
    }

    if (!g_pdxgiFactory)
    {
        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&g_pdxgiFactory));
        if (FAILED(hr))
            return hr;
    }
#else
    if (!g_pdxgiFactory)
    {
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&g_pdxgiFactory));
        if (FAILED(hr))
            return hr;
    }
#endif

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != g_pdxgiFactory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf()); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        if (SUCCEEDED(adapter->GetDesc1(&desc)))
        {
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

    if (!adapter)
    {
        // Try WARP12 instead
        if (FAILED(g_pdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
        {
            return E_FAIL;
        }
    }

    hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(pDev));
    if (FAILED(hr))
        return hr;

#ifndef NDEBUG
    // Configure debug device (if active).
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED((*pDev)->QueryInterface(IID_PPV_ARGS(&d3dInfoQueue))))
    {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
        D3D12_MESSAGE_ID hide[] =
        {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
        };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
    }
#endif

    return S_OK;
}

//-------------------------------------------------------------------------------------
HRESULT CreateCommmandList(ID3D12Device* pDev, ID3D12CommandQueue** pCommandQ, ID3D12GraphicsCommandList** pCommandList)
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HRESULT hr = pDev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_pd3dCommandQueue));
    if (FAILED(hr))
        return hr;

    hr = pDev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_pd3dAllocator));
    if (FAILED(hr))
        return hr;

    hr = pDev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_pd3dAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList));
    if (FAILED(hr))
        return hr;

    hr = pDev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_pd3dFence));
    if (FAILED(hr))
        return hr;

    // Note the test code uses a simplified fence model, but this is not a recommended code pattern
    g_fenceValue = 1;

    g_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (!g_fenceEvent)
        return E_FAIL;

    if (pCommandQ)
    {
        *pCommandQ = g_pd3dCommandQueue;
        g_pd3dCommandQueue->AddRef();
    }

    if (pCommandList)
    {
        *pCommandList = g_pd3dCommandList;
        g_pd3dCommandList->AddRef();
    }

    return S_OK;
}

//-------------------------------------------------------------------------------------
HRESULT SetupRenderTest(ID3D12Device** pDev, ID3D12CommandQueue** pCommandQ, ID3D12GraphicsCommandList** pCommandList)
{
    if (g_hWnd)
        return E_FAIL;

    //--- Create window ---------------------------------------------------------------
    static bool s_classExists;
    if (!s_classExists)
    {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEX);
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
    g_hWnd = CreateWindowW(L"DirectXTexClass", L"DirectXTex (D3D12)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, nullptr, nullptr);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);

    //--- Setup device ----------------------------------------------------------------
    HRESULT hr = CreateDevice(&g_pd3dDevice);
    if (FAILED(hr))
        return hr;

    hr = CreateCommmandList(g_pd3dDevice, nullptr, nullptr);

    //--- Setup descriptor heaps ------------------------------------------------------
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 2;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dHeapRTV));
        if (FAILED(hr))
            return hr;
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dHeapDSV));
        if (FAILED(hr))
            return hr;
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dHeapSRV));
        if (FAILED(hr))
            return hr;
    }

    //--- Setup swap chain ------------------------------------------------------------
    {
        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.BufferCount = 2;
        sd.Width = RENDER_WIDTH;
        sd.Height = RENDER_HEIGHT;
        sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fs = {};
        fs.Windowed = TRUE;

        ComPtr<IDXGISwapChain1> swapChain;
        hr = g_pdxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, g_hWnd, &sd, &fs, nullptr, swapChain.GetAddressOf());
        if (FAILED(hr))
            return hr;

        hr = g_pdxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr))
            return hr;

        hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&g_pRenderTarget[0]));
        if (FAILED(hr))
            return hr;

        hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&g_pRenderTarget[1]));
        if (FAILED(hr))
            return hr;

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = sd.Format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        g_rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor0(g_pd3dHeapRTV->GetCPUDescriptorHandleForHeapStart());
        g_pd3dDevice->CreateRenderTargetView(g_pRenderTarget[0], &rtvDesc, rtvDescriptor0);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor1(g_pd3dHeapRTV->GetCPUDescriptorHandleForHeapStart(), 1, g_rtvDescriptorSize);
        g_pd3dDevice->CreateRenderTargetView(g_pRenderTarget[1], &rtvDesc, rtvDescriptor1);

        hr = swapChain->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
        if (FAILED(hr))
            return hr;

        g_backBufferIndex = g_pSwapChain->GetCurrentBackBufferIndex();
    }

    //--- Setup depth buffer ----------------------------------------------------------
    {
        // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
        // on this surface.
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT,
            RENDER_WIDTH,
            RENDER_HEIGHT,
            1, // This depth stencil view has only one texture.
            1  // Use a single mipmap level.
        );
        desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = desc.Format;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        hr = g_pd3dDevice->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&g_pDepthStencil)
        );
        if (FAILED(hr))
            return hr;

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = desc.Format;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

        g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &dsvDesc, g_pd3dHeapDSV->GetCPUDescriptorHandleForHeapStart());
    }

    //--- Setup PSOs ------------------------------------------------------------------
    {
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE textureSRV(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        {
            CD3DX12_STATIC_SAMPLER_DESC sampler(
                0, // register
                D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP);

            CD3DX12_ROOT_PARAMETER rootParameters[RootParameterIndex::RootParameterCount] = {};
            rootParameters[RootParameterIndex::TextureSRV].InitAsDescriptorTable(1, &textureSRV);
            rootParameters[RootParameterIndex::CBNever].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
            rootParameters[RootParameterIndex::CBOnResize].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
            rootParameters[RootParameterIndex::CBEveryFrame].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);

            CD3DX12_ROOT_SIGNATURE_DESC rsigDesc;
            rsigDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

            ComPtr<ID3DBlob> pSignature;
            ComPtr<ID3DBlob> pError;
            hr = D3D12SerializeRootSignature(&rsigDesc, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf());
            if (FAILED(hr))
                return hr;

            hr = g_pd3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&g_rootSig));
            if (FAILED(hr))
                return hr;
        }
    }

    {
        static const D3D12_INPUT_ELEMENT_DESC s_layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        static const D3D12_BLEND_DESC s_blend = { FALSE, FALSE,
                                                  { FALSE, FALSE, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                                                                  D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                                                                  D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL } };

        static const D3D12_RASTERIZER_DESC s_raster = { D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE,
                                                         FALSE,
                                                         D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
                                                         TRUE, FALSE, FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF };

        static const D3D12_DEPTH_STENCIL_DESC s_depth = { TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS_EQUAL, FALSE, D3D12_DEFAULT_STENCIL_READ_MASK,D3D12_DEFAULT_STENCIL_WRITE_MASK,
                                                          { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS },
                                                          { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS } };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = g_rootSig;
        desc.InputLayout.NumElements = _countof(s_layout);
        desc.InputLayout.pInputElementDescs = s_layout;
        desc.BlendState = s_blend;
        desc.DepthStencilState = s_depth;
        desc.RasterizerState = s_raster;
        desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        desc.NodeMask = 0;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.SampleMask = UINT_MAX;
        desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        desc.VS.pShaderBytecode = g_VS2D;
        desc.VS.BytecodeLength = _countof(g_VS2D);
        desc.PS.pShaderBytecode = g_PS1D;
        desc.PS.BytecodeLength = _countof(g_PS1D);
        hr = g_pd3dDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&g_p1DPSO));
        if (FAILED(hr))
            return hr;

        desc.PS.pShaderBytecode = g_PS2D;
        desc.PS.BytecodeLength = _countof(g_PS2D);
        hr = g_pd3dDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&g_p2DPSO));
        if (FAILED(hr))
            return hr;

        desc.VS.pShaderBytecode = g_VSCube;
        desc.VS.BytecodeLength = _countof(g_VSCube);
        desc.PS.pShaderBytecode = g_PSCube;
        desc.PS.BytecodeLength = _countof(g_PSCube);
        hr = g_pd3dDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&g_pCubePSO));
        if (FAILED(hr))
            return hr;

        desc.VS.pShaderBytecode = g_VS3D;
        desc.VS.BytecodeLength = _countof(g_VS3D);
        desc.PS.pShaderBytecode = g_PS3D;
        desc.PS.BytecodeLength = _countof(g_PS3D);
        hr = g_pd3dDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&g_p3DPSO));
        if (FAILED(hr))
            return hr;
    }

    //--- Setup geometry --------------------------------------------------------------
    // Note the test code uses an upload heap for geometry, but this is not a recommended code pattern

    {
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

        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(s_vertices));
        hr = g_pd3dDevice->CreateCommittedResource(&uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&g_VB));
        if (FAILED(hr))
            return hr;

        // Copy the data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
        hr = g_VB->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
        if (FAILED(hr))
            return hr;
        memcpy(pVertexDataBegin, s_vertices, sizeof(s_vertices));
        g_VB->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        g_VBView.BufferLocation = g_VB->GetGPUVirtualAddress();
        g_VBView.StrideInBytes = sizeof(SimpleVertex);
        g_VBView.SizeInBytes = sizeof(s_vertices);
    }

    {
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

        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(s_indices));
        hr = g_pd3dDevice->CreateCommittedResource(&uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&g_IB));
        if (FAILED(hr))
            return hr;

        // Copy the data to the index buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
        hr = g_IB->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
        if (FAILED(hr))
            return hr;
        memcpy(pVertexDataBegin, s_indices, sizeof(s_indices));
        g_IB->Unmap(0, nullptr);

        // Initialize the index buffer view.
        g_IBView.BufferLocation = g_IB->GetGPUVirtualAddress();
        g_IBView.Format = DXGI_FORMAT_R16_UINT;
        g_IBView.SizeInBytes = sizeof(s_indices);
    }

    //--- Setup Constant Buffer -------------------------------------------------------

    {
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        size_t cbSize = AlignUp(sizeof(CBNeverChanges), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
            + AlignUp(sizeof(CBChangeOnResize), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
            + AlignUp(sizeof(CBChangesEveryFrame), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
        hr = g_pd3dDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&g_CB));
        if (FAILED(hr))
            return hr;

        hr = g_CB->Map(0, nullptr, reinterpret_cast< void** >(&g_CBCPU));
        if (FAILED(hr))
            return hr;

        // Initialize the world matrices
        g_World = XMMatrixIdentity();

        // Initialize the view matrix
        XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        g_View = XMMatrixLookAtLH(XMLoadFloat4(&g_vEyePosition), At, Up);

        uint8_t* ptr = g_CBCPU;

        CBNeverChanges cbNeverChanges = {};
        cbNeverChanges.mView = XMMatrixTranspose(g_View);
        memcpy(ptr, &cbNeverChanges, sizeof(cbNeverChanges));

        ptr += AlignUp(sizeof(CBNeverChanges), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        // Initialize the projection matrix
        g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, RENDER_WIDTH / (FLOAT)RENDER_HEIGHT, 0.01f, 100.0f);

        CBChangeOnResize cbChangesOnResize = {};
        cbChangesOnResize.mProjection = XMMatrixTranspose(g_Projection);
        memcpy(ptr, &cbChangesOnResize, sizeof(cbChangesOnResize));
    }

    if (pDev)
    {
        *pDev = g_pd3dDevice;
        g_pd3dDevice->AddRef();
    }
    if (pCommandQ)
    {
        *pCommandQ = g_pd3dCommandQueue;
        g_pd3dCommandQueue->AddRef();
    }
    if (pCommandList)
    {
        *pCommandList = g_pd3dCommandList;
        g_pd3dCommandList->AddRef();
    }

    return S_OK;
}


//-------------------------------------------------------------------------------------
namespace
{
    void Render(const TexMetadata& metadata)
    {
        // Update our time
        static float t = 0.0f;
        static ULONGLONG dwTimeStart = 0;
        ULONGLONG dwTimeCur = GetTickCount64();
        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;
        t = (dwTimeCur - dwTimeStart) / 1000.0f;

        // Rotate cube around the origin
        g_World = XMMatrixRotationY( t );

        {
            CBChangesEveryFrame cb = {};
            cb.mWorld = XMMatrixTranspose(g_World);
            cb.vEyePosition = g_vEyePosition;
            cb.vMeshColor = g_vMeshColor;

            uint8_t* ptr = g_CBCPU
                + AlignUp(sizeof(CBNeverChanges), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
                + AlignUp(sizeof(CBChangeOnResize), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

            memcpy(ptr, &cb, sizeof(cb));
        }

        // Prepare
        ThrowIfFailed(g_pd3dAllocator->Reset());
        ThrowIfFailed(g_pd3dCommandList->Reset(g_pd3dAllocator, nullptr /* PSO */));

        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(g_pRenderTarget[g_backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
     
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(g_pd3dHeapRTV->GetCPUDescriptorHandleForHeapStart(), g_backBufferIndex, g_rtvDescriptorSize);
        auto dsvDescriptor = g_pd3dHeapDSV->GetCPUDescriptorHandleForHeapStart();

        g_pd3dCommandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);

        // Clear
        float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
        g_pd3dCommandList->ClearRenderTargetView(rtvDescriptor, ClearColor, 0, nullptr);

        g_pd3dCommandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        CD3DX12_VIEWPORT vp(0.f, 0.f, RENDER_WIDTH, RENDER_HEIGHT, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
        g_pd3dCommandList->RSSetViewports(1, &vp);

        CD3DX12_RECT scissorRect(0, 0, RENDER_WIDTH, RENDER_HEIGHT);
        g_pd3dCommandList->RSSetScissorRects(1, &scissorRect);

        // Render texture
        g_pd3dCommandList->SetGraphicsRootSignature(g_rootSig);

        switch (metadata.dimension)
        {
        case TEX_DIMENSION_TEXTURE1D:
            g_pd3dCommandList->SetPipelineState(g_p1DPSO);
            break;

        case TEX_DIMENSION_TEXTURE3D:
            g_pd3dCommandList->SetPipelineState(g_p3DPSO);
            break;

        case TEX_DIMENSION_TEXTURE2D:
            if (metadata.miscFlags & TEX_MISC_TEXTURECUBE)
            {
                g_pd3dCommandList->SetPipelineState(g_pCubePSO);
            }
            else
            {
                g_pd3dCommandList->SetPipelineState(g_p2DPSO);
            }
            break;
        }

        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dHeapSRV);

        g_pd3dCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSRV, g_pd3dHeapSRV->GetGPUDescriptorHandleForHeapStart());

        D3D12_GPU_VIRTUAL_ADDRESS ptr = g_CB->GetGPUVirtualAddress();
        g_pd3dCommandList->SetGraphicsRootConstantBufferView(RootParameterIndex::CBNever, ptr);

        ptr += AlignUp(sizeof(CBNeverChanges), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        g_pd3dCommandList->SetGraphicsRootConstantBufferView(RootParameterIndex::CBOnResize, ptr);

        ptr += AlignUp(sizeof(CBChangeOnResize), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        g_pd3dCommandList->SetGraphicsRootConstantBufferView(RootParameterIndex::CBEveryFrame, ptr);

        g_pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_pd3dCommandList->IASetVertexBuffers(0, 1, &g_VBView);
        g_pd3dCommandList->IASetIndexBuffer(&g_IBView);

        g_pd3dCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

        //
        // Present our back buffer to our front buffer
        //
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(g_pRenderTarget[g_backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);

        ThrowIfFailed(g_pd3dCommandList->Close());
        ID3D12CommandList* ppCommandLists[] = { g_pd3dCommandList };
        g_pd3dCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        HRESULT hr = g_pSwapChain->Present(1, 0);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            throw std::exception("Present");
        }

        const UINT64 fence = g_fenceValue;
        ThrowIfFailed(g_pd3dCommandQueue->Signal(g_pd3dFence, fence));

        // Update the back buffer index.
        g_backBufferIndex = g_pSwapChain->GetCurrentBackBufferIndex();

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (g_pd3dFence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(g_pd3dFence->SetEventOnCompletion(g_fenceValue, g_fenceEvent));
            WaitForSingleObjectEx(g_fenceEvent, INFINITE, FALSE);
        }

        // Set the fence value for the next frame.
        g_fenceValue = fence + 1;
    }

    void WaitForGpu()
    {
        const UINT64 fence = g_fenceValue;
        ThrowIfFailed(g_pd3dCommandQueue->Signal(g_pd3dFence, fence));

        if (g_pd3dFence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(g_pd3dFence->SetEventOnCompletion(fence, g_fenceEvent));
            WaitForSingleObject(g_fenceEvent, INFINITE);
        }

        g_fenceValue = fence + 1;
    }
}

//-------------------------------------------------------------------------------------
void UploadTest()
{
    // Wait for resource upload
    ThrowIfFailed(g_pd3dCommandList->Close());
    ID3D12CommandList* ppCommandLists[] = { g_pd3dCommandList };
    g_pd3dCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    WaitForGpu();

    ThrowIfFailed(g_pd3dCommandList->Reset(g_pd3dAllocator, nullptr));
}

//-------------------------------------------------------------------------------------
void RenderTest(const TexMetadata& metadata, ID3D12Resource* pResource)
{
    // Wait for resource upload
    ThrowIfFailed(g_pd3dCommandList->Close());
    ID3D12CommandList* ppCommandLists[] = { g_pd3dCommandList };
    g_pd3dCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    {
        D3D12_RESOURCE_DESC desc = pResource->GetDesc();
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = desc.Format;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        switch (desc.Dimension)
        {
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            if (desc.DepthOrArraySize > 1)
            {
                SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                SRVDesc.Texture1DArray.MipLevels = (!desc.MipLevels) ? -1 : desc.MipLevels;
                SRVDesc.Texture1DArray.ArraySize = static_cast<UINT>(desc.DepthOrArraySize);
            }
            else
            {
                SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                SRVDesc.Texture1D.MipLevels = (!desc.MipLevels) ? -1 : desc.MipLevels;
            }
            break;

        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            if (metadata.miscFlags & TEX_MISC_TEXTURECUBE)
            {
                if (desc.DepthOrArraySize > 6)
                {
                    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    SRVDesc.TextureCubeArray.MipLevels = (!desc.MipLevels) ? -1 : desc.MipLevels;
                    SRVDesc.TextureCubeArray.NumCubes = static_cast<UINT>(desc.DepthOrArraySize / 6);
                }
                else
                {
                    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    SRVDesc.TextureCube.MipLevels = (!desc.MipLevels) ? -1 : desc.MipLevels;
                }
            }
            else if (desc.DepthOrArraySize > 1)
            {
                SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                SRVDesc.Texture2DArray.MipLevels = (!desc.MipLevels) ? -1 : desc.MipLevels;
                SRVDesc.Texture2DArray.ArraySize = static_cast<UINT>(desc.DepthOrArraySize);
            }
            else
            {
                SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                SRVDesc.Texture2D.MipLevels = (!desc.MipLevels) ? -1 : desc.MipLevels;
            }
            break;

        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            SRVDesc.Texture3D.MipLevels = (!desc.MipLevels) ? -1 : desc.MipLevels;
            break;
        }

        g_pd3dDevice->CreateShaderResourceView(pResource, &SRVDesc, g_pd3dHeapSRV->GetCPUDescriptorHandleForHeapStart());
    }

    WaitForGpu();

    // Main message loop
    MSG msg = {};

    ULONGLONG end = GetTickCount64() + RENDER_TEST_MS_DELAY;

    for(;;)
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render(metadata);
        }

        if ( GetTickCount64() > end )
            break;
    }

    ThrowIfFailed(g_pd3dCommandList->Reset(g_pd3dAllocator, nullptr));
}


//-------------------------------------------------------------------------------------

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p)=nullptr; } }

void CleanupRenderTest()
{
    SAFE_RELEASE(g_pd3dCommandList);
    SAFE_RELEASE(g_pd3dCommandQueue);
    SAFE_RELEASE(g_pd3dFence);
    SAFE_RELEASE(g_pd3dAllocator);

    SAFE_RELEASE(g_pd3dHeapRTV);
    SAFE_RELEASE(g_pd3dHeapDSV);
    SAFE_RELEASE(g_pd3dHeapSRV);

    SAFE_RELEASE(g_rootSig);
    SAFE_RELEASE(g_p1DPSO);
    SAFE_RELEASE(g_p2DPSO);
    SAFE_RELEASE(g_pCubePSO);
    SAFE_RELEASE(g_p3DPSO);

    SAFE_RELEASE(g_VB);
    SAFE_RELEASE(g_IB);
    SAFE_RELEASE(g_CB);
    g_VBView.BufferLocation = 0;
    g_IBView.BufferLocation = 0;
    g_CBCPU = nullptr;

    SAFE_RELEASE(g_pRenderTarget[0]);
    SAFE_RELEASE(g_pRenderTarget[1]);
    SAFE_RELEASE(g_pDepthStencil);

    SAFE_RELEASE(g_pSwapChain);
    SAFE_RELEASE(g_pd3dDevice);
    SAFE_RELEASE(g_pdxgiFactory);

    if (g_fenceEvent)
    {
        CloseHandle(g_fenceEvent);
        g_fenceEvent = nullptr;
    }

    if ( g_hWnd )
    {
        CloseWindow( g_hWnd );
        g_hWnd = nullptr;
    }
}
