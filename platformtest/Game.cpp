//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "DirectXTex.h"
#include "directxtest.h"

#include "..\wic\wic.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

namespace
{
    float c_ColorDelay = 0.25f;

    typedef bool(*TestFN)();

    struct TestInfo
    {
        const char *name;
        TestFN func;
    };

    TestInfo g_Tests[] =
    {
        // WIC
        { "Get/SetWICFactory", WICTest::Test00 },
        { "GetMetadataFromWICMemory", WICTest::Test01 },
        { "GetMetadataFromWICFile", WICTest::Test02 },
        { "LoadFromWICMemory", WICTest::Test03 },
        { "LoadFromWICFile", WICTest::Test04 },
        { "SaveWICToMemory", WICTest::Test05 },
        { "SaveWICToFile", WICTest::Test06 },
        { "*Transcode*", WICTest::Test07 },
        { "Fuzz", WICTest::Test08 },
    };
}

Game::Game() noexcept(false) :
    m_delayTime(0.f),
    m_cycle(0),
    m_terminateThread(false),
    m_suspendThread(false),
    m_testThread(nullptr)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (m_testThread)
    {
        m_terminateThread = true;
        m_testThread->join();
        m_testThread = nullptr;
    }
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation)
{
    m_deviceResources->SetWindow(window, width, height, rotation);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_resumeSignal.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
    if (!m_resumeSignal.IsValid())
        throw std::exception("CreateEvent");

    m_testThread = new std::thread(&Game::TestThreadProc, this);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
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

    if (m_delayTime >= 0.f)
    {
        m_delayTime -= elapsedTime;
    }
    else
    {
        m_delayTime = c_ColorDelay;

        switch (m_cycle % 3)
        {
        case 0: memcpy(m_clearColor, Colors::ForestGreen.f, sizeof(XMVECTOR)); break;
        case 1: memcpy(m_clearColor, Colors::Green.f, sizeof(XMVECTOR)); break;
        case 2: memcpy(m_clearColor, Colors::LightGreen.f, sizeof(XMVECTOR)); break;
        }

        ++m_cycle;
    }

    if (m_testThread)
    {
        if (WaitForSingleObject(m_testThread->native_handle(), 0) == WAIT_OBJECT_0)
        {
            ExitGame();
        }
    }

    Sleep(1000);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Render");

    PIXEndEvent(context);

    // Show the new frame.
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    PIXEndEvent();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto renderTarget = m_deviceResources->GetRenderTargetView();

    context->ClearRenderTargetView(renderTarget, m_clearColor);
    context->OMSetRenderTargets(1, &renderTarget, nullptr);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    PIXEndEvent(context);
 }
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
    ResetEvent(m_resumeSignal.Get());
    m_suspendThread = true;

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->ClearState();

    m_deviceResources->Trim();
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    m_suspendThread = false;
    SetEvent(m_resumeSignal.Get());
}

void Game::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, rotation))
        return;

    CreateWindowSizeDependentResources();
}

void Game::ValidateDevice()
{
    m_deviceResources->ValidateDevice();
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 320;
    height = 200;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
}

void Game::OnDeviceLost()
{
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

void Game::TestThreadProc()
{
    DX::ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

    UINT nPass = 0;
    UINT nFail = 0;

    for (UINT i = 0; i < (sizeof(g_Tests) / sizeof(TestInfo)); ++i)
    {
        if (m_suspendThread)
        {
            (void)WaitForSingleObject(m_resumeSignal.Get(), INFINITE);
        }

        print("\n--- %s ---\n", g_Tests[i].name);

        if (g_Tests[i].func())
        {
            ++nPass;
            print("*** %s: PASS ***\n", g_Tests[i].name);
        }
        else
        {
            ++nFail;
            print("*** %s: FAIL ***\n", g_Tests[i].name);
        }

        if (m_terminateThread)
            break;
    }

    print("Ran %d tests, %d pass, %d fail\n", nPass + nFail, nPass, nFail);

    SetWICFactory(nullptr);
}
