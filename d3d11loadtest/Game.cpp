//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"
#include "ScreenGrab11.h"
#include "DirectXTex.h"
#include "ReadData.h"

#include <wincodec.h>

namespace
{
    const XMVECTORF32 c_clearColor = { { { 0.127437726f, 0.300543845f, 0.846873462f, 1.f } } };
    const XMVECTORF32 c_clearColor4 = { { { 0.015996292f, 0.258182913f, 0.015996292f, 1.f } } };

    struct Vertex
    {
        XMFLOAT4 position;
        XMFLOAT2 texcoord;
    };
}

Game::Game() noexcept(false) :
    m_frame(0)
{
    // Use gamma-correct rendering.
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    m_deviceResources->RegisterDeviceNotify(this);

    m_msaaHelper = std::make_unique<DX::MSAAHelper>(m_deviceResources->GetBackBufferFormat(),
        DXGI_FORMAT_UNKNOWN,
        4);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
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

    ++m_frame;
}

// Updates the world.
void Game::Update(DX::StepTimer const&)
{
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

    auto context = m_deviceResources->GetD3DDeviceContext();

    {
        auto renderTarget = m_msaaHelper->GetMSAARenderTargetView();
        context->ClearRenderTargetView(renderTarget, c_clearColor4);

        // TODO: Render a triangle in MSAA

        // Note we don't resolve it to the swapchain, just use it for screensave tests.
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");

    // Set input assembler state.
    context->IASetInputLayout(m_spInputLayout.Get());

    UINT strides = sizeof(Vertex);
    UINT offsets = 0;
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetVertexBuffers(0, 1, m_spVertexBuffer.GetAddressOf(), &strides, &offsets);

    context->IASetIndexBuffer(m_spIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // Set shaders.
    context->VSSetShader(m_spVertexShader.Get(), nullptr, 0);
    context->GSSetShader(nullptr, nullptr, 0);
    context->PSSetShader(m_spPixelShader.Get(), nullptr, 0);

    // Set texture and sampler.
    auto sampler = m_spSampler.Get();
    context->PSSetSamplers(0, 1, &sampler);

    // Draw quad 1.
    auto texture = m_dx5logo.Get();
    context->PSSetShaderResources(0, 1, &texture);

    context->DrawIndexed(6, 0, 0);

    // Draw quad 2.
    texture = m_cup.Get();
    context->PSSetShaderResources(0, 1, &texture);

    context->DrawIndexed(6, 0, 4);

    m_deviceResources->PIXEndEvent();

    if (!(m_frame % 100))
    {
        OutputDebugStringA("Saving screenshot...\n");

        ComPtr<ID3D11Resource> backbuffer;
        m_deviceResources->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));

        DX::ThrowIfFailed(SaveDDSTextureToFile(context, backbuffer.Get(), L"screenshot.dds"));

        DX::ThrowIfFailed(SaveWICTextureToFile(context, backbuffer.Get(), GUID_ContainerFormatPng, L"screenshot.png", nullptr, nullptr, true));

        DX::ThrowIfFailed(SaveWICTextureToFile(context, backbuffer.Get(), GUID_ContainerFormatJpeg, L"screenshot.jpg"));

        ScratchImage image;
        DX::ThrowIfFailed(CaptureTexture(m_deviceResources->GetD3DDevice(), context, backbuffer.Get(), image));

        DX::ThrowIfFailed(SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DDS_FLAGS_NONE, L"screenshot2.dds"));
        DX::ThrowIfFailed(SaveToWICFile(*image.GetImage(0,0,0), WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, L"screenshot2.jpg"));

        // MSAA test
        auto backBufferMSAA = m_msaaHelper->GetMSAARenderTarget();
        DX::ThrowIfFailed(SaveDDSTextureToFile(context, backBufferMSAA, L"screenshotMSAA.dds"));

        DX::ThrowIfFailed(SaveWICTextureToFile(context, backBufferMSAA, GUID_ContainerFormatPng, L"screenshotMSAA.png", nullptr, nullptr, true));

        DX::ThrowIfFailed(CaptureTexture(m_deviceResources->GetD3DDevice(), context, backBufferMSAA, image));

        DX::ThrowIfFailed(SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DDS_FLAGS_NONE, L"screenshotMSAA2.dds"));
        DX::ThrowIfFailed(SaveToWICFile(*image.GetImage(0, 0, 0), WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, L"screenshotMSAA2.jpg"));
    }

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    // Use linear clear color for gamma-correct rendering.
    context->ClearRenderTargetView(renderTarget, c_clearColor);

    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
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
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
}

void Game::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    m_msaaHelper->SetDevice(device);

    // Load and create shaders.
    auto vertexShaderBlob = DX::ReadData(L"VertexShader11.cso");

    DX::ThrowIfFailed(
        device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
            nullptr, m_spVertexShader.ReleaseAndGetAddressOf()));

    auto pixelShaderBlob = DX::ReadData(L"PixelShader11.cso");

    DX::ThrowIfFailed(
        device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
            nullptr, m_spPixelShader.ReleaseAndGetAddressOf()));

    // Create input layout.
    static const D3D11_INPUT_ELEMENT_DESC s_inputElementDesc[2] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,  0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA , 0 },
    };

    DX::ThrowIfFailed(
        device->CreateInputLayout(s_inputElementDesc, static_cast<UINT>(std::size(s_inputElementDesc)),
            vertexShaderBlob.data(), vertexShaderBlob.size(),
            m_spInputLayout.ReleaseAndGetAddressOf()));

    // Create vertex buffer.
    static const Vertex s_vertexData[] =
    {
        { { -1.0f, -0.5f, 0.5f, 1.0f },{ 0.f, 1.f } },
        { { 0.0f, -0.5f, 0.5f, 1.0f },{ 1.f, 1.f } },
        { { 0.0f,  0.5f, 0.5f, 1.0f },{ 1.f, 0.f } },
        { { -1.0f,  0.5f, 0.5f, 1.0f },{ 0.f, 0.f } },

        { { 0.f, -0.5f, 0.5f, 1.0f },{ 0.f, 1.f } },
        { { 1.0f, -0.5f, 0.5f, 1.0f },{ 1.f, 1.f } },
        { { 1.0f,  0.5f, 0.5f, 1.0f },{ 1.f, 0.f } },
        { { 0.f,  0.5f, 0.5f, 1.0f },{ 0.f, 0.f } },
    };

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = s_vertexData;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(s_vertexData);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.StructureByteStride = sizeof(Vertex);

    DX::ThrowIfFailed(
        device->CreateBuffer(&bufferDesc, &initialData,
            m_spVertexBuffer.ReleaseAndGetAddressOf()));

    // Create index buffer.
    static const uint16_t s_indexData[6] =
    {
        3,1,0,
        2,1,3,
    };

    initialData.pSysMem = s_indexData;

    bufferDesc.ByteWidth = sizeof(s_indexData);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.StructureByteStride = sizeof(uint16_t);

    DX::ThrowIfFailed(
        device->CreateBuffer(&bufferDesc, &initialData,
            m_spIndexBuffer.ReleaseAndGetAddressOf()));

    // Create sampler.
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    DX::ThrowIfFailed(
        device->CreateSamplerState(&samplerDesc,
            m_spSampler.ReleaseAndGetAddressOf()));

    // Test DDSTextureLoader
    DX::ThrowIfFailed(
        CreateDDSTextureFromFileEx(device, L"dx5_logo.dds",
            0, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, DDS_LOADER_FORCE_SRGB,
            nullptr, m_dx5logo.ReleaseAndGetAddressOf()));

    {
        auto blob = DX::ReadData(L"dx5_logo.dds");

        ComPtr<ID3D11ShaderResourceView> srv;
        DX::ThrowIfFailed(
            CreateDDSTextureFromMemoryEx(device, blob.data(), blob.size(),
                0, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, DDS_LOADER_FORCE_SRGB,
                nullptr, srv.ReleaseAndGetAddressOf()));
    }

    // Test WICTextureLoader
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"cup_small.jpg",
            nullptr, m_cup.ReleaseAndGetAddressOf()));

    {
        auto blob = DX::ReadData(L"cup_small.jpg");

        ComPtr<ID3D11ShaderResourceView> srv;
        DX::ThrowIfFailed(
            CreateWICTextureFromMemory(device, blob.data(), blob.size(),
                nullptr, srv.ReleaseAndGetAddressOf()));
    }
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    m_msaaHelper->SetWindow(size);

}

void Game::OnDeviceLost()
{
    m_cup.Reset();
    m_dx5logo.Reset();

    m_spInputLayout.Reset();
    m_spVertexBuffer.Reset();
    m_spIndexBuffer.Reset();
    m_spVertexShader.Reset();
    m_spPixelShader.Reset();
    m_spSampler.Reset();

    m_msaaHelper->ReleaseDevice();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
