//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

#include "DDSTextureLoader9.h"
#include "WICTextureLoader9.h"

namespace
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 texcoord;
    };
}

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();
    CreateWindowSizeDependentResources();
}

// Executes the basic game loop.
void Game::Tick()
{
    HRESULT hr = m_d3dDevice->TestCooperativeLevel();
    if (FAILED(hr))
    {
        if (hr == D3DERR_DEVICENOTRESET)
        {
            OnDeviceLost();
        }
        return;
    }

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    elapsedTime;
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_d3dDevice->BeginScene();

    m_d3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

    m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

    static const Vertex s_vertexData[] =
    {
        { { -1.0f, -0.5f, 0.5f },{ 0.f, 1.f } },
        { { 0.0f, -0.5f, 0.5f },{ 1.f, 1.f } },
        { { 0.0f,  0.5f, 0.5f },{ 1.f, 0.f } },
        { { -1.0f,  0.5f, 0.5f },{ 0.f, 0.f } },

        { { 0.f, -0.5f, 0.5f },{ 0.f, 1.f } },
        { { 1.0f, -0.5f, 0.5f },{ 1.f, 1.f } },
        { { 1.0f,  0.5f, 0.5f },{ 1.f, 0.f } },
        { { 0.f,  0.5f, 0.5f },{ 0.f, 0.f } },
    };

    static const uint16_t s_indexData[6] =
    {
        3,1,0,
        2,1,3,
    };

    m_d3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,
        0, _countof(s_vertexData), 2,
        s_indexData, D3DFMT_INDEX16, s_vertexData, sizeof(Vertex));

    // Draw quad 1.
    m_d3dDevice->SetTexture(0, m_dx5logo.Get());
    m_d3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,
        0, _countof(s_vertexData), 2,
        s_indexData, D3DFMT_INDEX16, s_vertexData, sizeof(Vertex));

    // Draw quad 2.
    m_d3dDevice->SetTexture(0, m_cup.Get());

    m_d3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,
        4, _countof(s_vertexData), 2,
        s_indexData, D3DFMT_INDEX16, s_vertexData, sizeof(Vertex));

    m_d3dDevice->EndScene();

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    DirectX::PackedVector::XMCOLOR bgcolor(Colors::CornflowerBlue.f);
    m_d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, bgcolor, 1.f, 0);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    m_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
}

// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    D3DPRESENT_PARAMETERS params = {};
    params.BackBufferCount = 1;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.hDeviceWindow = m_window;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = TRUE;
    params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (SUCCEEDED(m_d3dDevice->Reset(&params)))
    {
        CreateWindowSizeDependentResources();
    }
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 800;
    height = 600;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    DX::ThrowIfFailed(
        Direct3DCreate9Ex(D3D_SDK_VERSION, m_d3d.ReleaseAndGetAddressOf()
        ));

    D3DPRESENT_PARAMETERS params = {};
    params.BackBufferCount = 1;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.hDeviceWindow = m_window;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = TRUE;
    params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    HRESULT hr = m_d3d->CreateDeviceEx(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
        m_window,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &params, nullptr,
        m_d3dDevice.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(hr);

    // Test DDSTextureLoader
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(m_d3dDevice.Get(), L"dx5_logo.dds", m_dx5logo.ReleaseAndGetAddressOf())
    );

    // Test WICTextureLoader
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(m_d3dDevice.Get(), L"cup_small.jpg", m_cup.ReleaseAndGetAddressOf())
    );
}

void Game::CreateWindowSizeDependentResources()
{
    D3DVIEWPORT9 vp = { 0u, 0u, DWORD(m_outputWidth), DWORD(m_outputHeight) };
    m_d3dDevice->SetViewport(&vp);
}

void Game::OnDeviceLost()
{
    D3DPRESENT_PARAMETERS params = {};
    params.BackBufferCount = 1;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.hDeviceWindow = m_window;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = TRUE;
    params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (SUCCEEDED(m_d3dDevice->Reset(&params)))
    {
        CreateWindowSizeDependentResources();
    }
}
