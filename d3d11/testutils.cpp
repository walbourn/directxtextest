//-------------------------------------------------------------------------------------
// testutils.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#define D3D_DEBUG

#include <assert.h>

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

#define RENDER_TEST_MS_DELAY 5000

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480 

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

//-------------------------------------------------------------------------------------
HWND                                g_hWnd = NULL;

ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;

ID3D11PixelShader*                  g_pPixelShader1D = NULL;
ID3D11PixelShader*                  g_pPixelShader2D = NULL;
ID3D11PixelShader*                  g_pPixelShader3D = NULL;
ID3D11PixelShader*                  g_pPixelShaderCube = NULL;

ID3D11VertexShader*                 g_pVertexShader2D = NULL;
ID3D11VertexShader*                 g_pVertexShader3D = NULL;
ID3D11VertexShader*                 g_pVertexShaderCube = NULL;

ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer = NULL;
ID3D11Buffer*                       g_pIndexBuffer = NULL;
ID3D11Buffer*                       g_pCBNeverChanges = NULL;
ID3D11Buffer*                       g_pCBChangeOnResize = NULL;
ID3D11Buffer*                       g_pCBChangesEveryFrame = NULL;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;

XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT4                            g_vEyePosition( 0.0f, 3.0f, -6.0f, 0.0f );
XMFLOAT4                            g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );

//-------------------------------------------------------------------------------------

// fxc d3d11.fx /EVS2D /Tvs_4_0 /Fhvs2D.h
#include "shaders\vs2D.h"

// fxc d3d11.fx /EPS1D /Tps_4_0 /Fhps1D.h
#include "shaders\ps1d.h"

// fxc d3d11.fx /EPS2D /Tps_4_0 /Fhps2D.h
#include "shaders\ps2d.h"

// fxc d3d11.fx /EVSCube /Tvs_4_0 /FhvsCube.h
#include "shaders\vsCube.h"

// fxc d3d11.fx /EPSCube /Tps_4_0 /FhpsCube.h
#include "shaders\psCube.h"

// fxc d3d11.fx /nologo /EVS3D /Tvs_4_0 /Fhshaders\vs3D.h
#include "shaders\vs3D.h"

//fxc d3d11.fx /nologo /EPS3D /Tps_4_0 /Fhshaders\ps3D.h
#include "shaders\ps3D.h"

//-------------------------------------------------------------------------------------
static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

         default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//-------------------------------------------------------------------------------------
HRESULT CreateDevice( ID3D11Device** pDev, ID3D11DeviceContext** pContext )
{
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };
    const UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    const UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    HRESULT hr = E_FAIL;

    UINT createDeviceFlags = 0;
#ifdef D3D_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        hr = D3D11CreateDevice( NULL, driverTypes[driverTypeIndex], NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, pDev, NULL, pContext );
        if( SUCCEEDED( hr ) )
            break;
    }

    return hr;
}

//-------------------------------------------------------------------------------------
HRESULT SetupRenderTest( ID3D11Device** pDev, ID3D11DeviceContext** pContext )
{
    if ( g_hWnd )
        return E_FAIL;

    //--- Create window ---------------------------------------------------------------
    static bool s_classExists;
    if ( !s_classExists )
    {
        WNDCLASSEX wcex;
        memset( &wcex, 0, sizeof(wcex) );
        wcex.cbSize = sizeof( WNDCLASSEX );
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
        wcex.lpfnWndProc = WndProc;
        wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
        wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = L"DirectXTexClass";
        if( !RegisterClassEx( &wcex ) )
            return E_FAIL;

        s_classExists = true;
    }

    RECT rc = { 0, 0, RENDER_WIDTH, RENDER_HEIGHT };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"DirectXTexClass", L"DirectXTex", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, NULL, NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, SW_SHOWDEFAULT );

    //--- Setup device ----------------------------------------------------------------
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };
    const UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    const UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = RENDER_WIDTH;
    sd.BufferDesc.Height = RENDER_HEIGHT;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    HRESULT hr = E_FAIL;

    UINT createDeviceFlags = 0;
#ifdef D3D_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        hr = D3D11CreateDeviceAndSwapChain( NULL, driverTypes[driverTypeIndex], NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, NULL, &g_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = RENDER_WIDTH;
    descDepth.Height = RENDER_HEIGHT;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)RENDER_WIDTH;
    vp.Height = (FLOAT)RENDER_HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    //--- Setup render cube -----------------------------------------------------------

    // Create the vertex shaders
    hr = g_pd3dDevice->CreateVertexShader( g_VS2D, sizeof(g_VS2D), NULL, &g_pVertexShader2D );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateVertexShader( g_VS3D, sizeof(g_VS3D), NULL, &g_pVertexShader3D );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateVertexShader( g_VSCube, sizeof(g_VSCube), NULL, &g_pVertexShaderCube );
    if( FAILED( hr ) )
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout (assumes all shaders share the same layout)
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, g_VS2D, sizeof(g_VS2D), &g_pVertexLayout );
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

    // Create the pixel shaders
    hr = g_pd3dDevice->CreatePixelShader( g_PS1D, sizeof(g_PS1D), NULL, &g_pPixelShader1D );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreatePixelShader( g_PS2D, sizeof(g_PS2D), NULL, &g_pPixelShader2D );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreatePixelShader( g_PS3D, sizeof(g_PS3D), NULL, &g_pPixelShader3D );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreatePixelShader( g_PSCube, sizeof(g_PSCube), NULL, &g_pPixelShaderCube );
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2( 1.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2( 1.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f), XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2( 1.0f, 0.0f ) },
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

    // Create index buffer
    // Create vertex buffer
    WORD indices[] =
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

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangeOnResize);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangeOnResize );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangesEveryFrame );
    if( FAILED( hr ) )
        return hr;

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( XMLoadFloat4( &g_vEyePosition ), At, Up );

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose( g_View );
    g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, RENDER_WIDTH / (FLOAT)RENDER_HEIGHT, 0.01f, 100.0f );
    
    CBChangeOnResize cbChangesOnResize;
    cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pCBChangeOnResize, 0, NULL, &cbChangesOnResize, 0, 0 );

    if ( pDev )
    {
        *pDev = g_pd3dDevice;
        g_pd3dDevice->AddRef();
    }

    if ( pContext )
        *pContext = g_pImmediateContext;

    return S_OK;
}


//-------------------------------------------------------------------------------------
static void Render()
{
    // Update our time
    static float t = 0.0f;
    static ULONGLONG dwTimeStart = 0;
    ULONGLONG dwTimeCur = GetTickCount64();
    if( dwTimeStart == 0 )
        dwTimeStart = dwTimeCur;
    t = ( dwTimeCur - dwTimeStart ) / 1000.0f;

    // Rotate cube around the origin
    g_World = XMMatrixRotationY( t );

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    //
    // Update variables that change once per frame
    //
    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose( g_World );
    cb.vEyePosition = g_vEyePosition;
    cb.vMeshColor = g_vMeshColor;
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );

    //
    // Render the cube
    //
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->VSSetConstantBuffers( 1, 1, &g_pCBChangeOnResize );
    g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pCBChangesEveryFrame );
    g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pCBChangesEveryFrame );
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
    g_pImmediateContext->DrawIndexed( 36, 0, 0 );

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}

//-------------------------------------------------------------------------------------
void RenderTest( const TexMetadata& metadata, ID3D11ShaderResourceView* pSRV )
{
    switch ( metadata.dimension )
    {
    case TEX_DIMENSION_TEXTURE1D:
        g_pImmediateContext->VSSetShader( g_pVertexShader2D, NULL, 0 );
        g_pImmediateContext->PSSetShader( g_pPixelShader1D, NULL, 0 );
        break;

    case TEX_DIMENSION_TEXTURE3D:
        g_pImmediateContext->VSSetShader( g_pVertexShader3D, NULL, 0 );
        g_pImmediateContext->PSSetShader( g_pPixelShader3D, NULL, 0 );
        break;

    case TEX_DIMENSION_TEXTURE2D:
        if ( metadata.miscFlags & TEX_MISC_TEXTURECUBE ) 
        {
            g_pImmediateContext->VSSetShader( g_pVertexShaderCube, NULL, 0 );
            g_pImmediateContext->PSSetShader( g_pPixelShaderCube, NULL, 0 );
        }
        else
        {
            g_pImmediateContext->VSSetShader( g_pVertexShader2D, NULL, 0 );
            g_pImmediateContext->PSSetShader( g_pPixelShader2D, NULL, 0 );
        }
        break;

    default:
        return;
    }

    if ( pSRV  )
        g_pImmediateContext->PSSetShaderResources( 0, 1, &pSRV );

    // Main message loop
    MSG msg = {0};

    ULONGLONG end = GetTickCount64() + RENDER_TEST_MS_DELAY;

    for(;;)
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }

        if ( GetTickCount64() > end )
            break;
    }

#pragma warning(push)
#pragma warning(disable : 6309)
#pragma warning(disable : 6387)
    if ( pSRV )
        g_pImmediateContext->PSSetShaderResources( 0, 0, NULL );
#pragma warning(pop)
}


//-------------------------------------------------------------------------------------

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p)=NULL; } }

void CleanupRenderTest()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    SAFE_RELEASE( g_pSamplerLinear );
    SAFE_RELEASE( g_pCBNeverChanges );
    SAFE_RELEASE( g_pCBChangeOnResize );
    SAFE_RELEASE( g_pCBChangesEveryFrame );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );
    SAFE_RELEASE( g_pVertexLayout );

    SAFE_RELEASE( g_pVertexShader2D );
    SAFE_RELEASE( g_pVertexShader3D );
    SAFE_RELEASE( g_pVertexShaderCube );

    SAFE_RELEASE( g_pPixelShader1D );
    SAFE_RELEASE( g_pPixelShader2D );
    SAFE_RELEASE( g_pPixelShader3D );
    SAFE_RELEASE( g_pPixelShaderCube );

    SAFE_RELEASE( g_pDepthStencil );
    SAFE_RELEASE( g_pDepthStencilView );
    SAFE_RELEASE( g_pRenderTargetView );
    SAFE_RELEASE( g_pSwapChain );
    SAFE_RELEASE( g_pImmediateContext );
    SAFE_RELEASE( g_pd3dDevice );

    if ( g_hWnd )
    {
        CloseWindow( g_hWnd );
        g_hWnd = NULL;
    }
}
