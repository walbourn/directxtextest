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

    // TODO: Add your rendering code here.

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

    m_d3dDevice->Reset(&params);
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
    m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_d3d)
    {
        throw std::exception("Direct3DCreate9");
    }

    D3DPRESENT_PARAMETERS params = {};
    params.BackBufferCount = 1;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.hDeviceWindow = m_window;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = TRUE;
    params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    HRESULT hr = m_d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
        m_window,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &params,
        m_d3dDevice.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(hr);
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

    m_d3dDevice->Reset(&params);
}
