//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"
#include "ScreenGrab12.h"
#include "DirectXTex.h"
#include "ReadData.h"

#include <wincodec.h>

#pragma warning(disable : 4238)

namespace
{
    struct Vertex
    {
        XMFLOAT4 position;
        XMFLOAT2 texcoord;
    };
}
Game::Game() noexcept(false) :
    m_frame(0),
    m_vertexBufferView{},
    m_indexBufferView{},
    m_strideSRV(0)
{
    // Use gamma-correct rendering.
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
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

    if (m_screenshot)
    {
        OutputDebugStringA("Saving screenshot...\n");

        auto commandQ = m_deviceResources->GetCommandQueue();

        DX::ThrowIfFailed(
            SaveDDSTextureToFile(commandQ, m_screenshot.Get(),
                                 L"screenshot.dds",
                                 D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT)
            );

        DX::ThrowIfFailed(
            SaveWICTextureToFile(commandQ, m_screenshot.Get(),
                GUID_ContainerFormatPng, L"screenshot.png",
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT,
                nullptr, nullptr, true)
        );

        DX::ThrowIfFailed(
            SaveWICTextureToFile(commandQ, m_screenshot.Get(),
                GUID_ContainerFormatJpeg, L"screenshot.jpg",
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT)
            );

        ScratchImage image;
        DX::ThrowIfFailed(CaptureTexture(commandQ, m_screenshot.Get(), false, image,
                                         D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT));

        DX::ThrowIfFailed(SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DDS_FLAGS_NONE, L"screenshot2.dds"));
        DX::ThrowIfFailed(SaveToWICFile(*image.GetImage(0, 0, 0), WIC_FLAGS_NONE, GUID_ContainerFormatJpeg, L"screenshot2.jpg"));

        m_screenshot.Reset();
    }

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

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetPipelineState(m_pipelineState.Get());

    auto heap = m_srvHeap.Get();
    commandList->SetDescriptorHeaps(1, &heap);

    commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    // Set necessary state.
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);

    // Draw quad 1. 
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    // Draw quad 2. 
    commandList->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart()).Offset(1, m_strideSRV));
    commandList->DrawIndexedInstanced(6, 1, 0, 4, 0);

    PIXEndEvent(commandList);

    // Show the new frame.
    if (!(m_frame % 100))
    {
        m_screenshot = m_deviceResources->GetRenderTarget();
    }

    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    PIXEndEvent(m_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);

    // Use linear clear color for gamma-correct rendering.
    XMVECTORF32 color;
    color.v = XMColorSRGBToRGB(Colors::CornflowerBlue);
    commandList->ClearRenderTargetView(rtvDescriptor, color, 0, nullptr);

    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
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
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
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

    m_strideSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Create descriptor heaps.
    {
        // Describe and create a shader resource view (SRV) heap for the texture.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 2;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        DX::ThrowIfFailed(
            device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(m_srvHeap.ReleaseAndGetAddressOf())));
    }

    // Create a root signature with one sampler and one texture
    {
        CD3DX12_DESCRIPTOR_RANGE descRange = {};
        descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER rp = {};
        rp.InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);

        // Use a static sampler that matches the defaults
        // https://msdn.microsoft.com/en-us/library/windows/desktop/dn913202(v=vs.85).aspx#static_sampler
        D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.MaxAnisotropy = 16;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.Init(1, &rp, 1, &samplerDesc,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (FAILED(hr))
        {
            if (error)
            {
                OutputDebugStringA(reinterpret_cast<const char*>(error->GetBufferPointer()));
            }
            throw DX::com_exception(hr);
        }

        DX::ThrowIfFailed(
            device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())));
    }

    // Create the pipeline state, which includes loading shaders.
    auto vertexShaderBlob = DX::ReadData(L"VertexShader.cso");

    auto pixelShaderBlob = DX::ReadData(L"PixelShader.cso");

    static const D3D12_INPUT_ELEMENT_DESC s_inputElementDesc[2] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  0 },
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { s_inputElementDesc, _countof(s_inputElementDesc) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.data(), vertexShaderBlob.size());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.data(), pixelShaderBlob.size());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
    psoDesc.SampleDesc.Count = 1;
    DX::ThrowIfFailed(
        device->CreateGraphicsPipelineState(&psoDesc,
            IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf())));

    CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

    // Create vertex buffer.
    {
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

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(s_vertexData));

        DX::ThrowIfFailed(
            device->CreateCommittedResource(&uploadHeap,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_vertexBuffer.ReleaseAndGetAddressOf())));

        // Copy the quad data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(
            m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, s_vertexData, sizeof(s_vertexData));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = sizeof(s_vertexData);
    }

    // Create index buffer.
    {
        static const uint16_t s_indexData[6] =
        {
            3,1,0,
            2,1,3,
        };

        // See note above
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(s_indexData));

        DX::ThrowIfFailed(
            device->CreateCommittedResource(&uploadHeap,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_indexBuffer.ReleaseAndGetAddressOf())));

        // Copy the data to the index buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(
            m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, s_indexData, sizeof(s_indexData));
        m_indexBuffer->Unmap(0, nullptr);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        m_indexBufferView.SizeInBytes = sizeof(s_indexData);
    }

    {
        auto commandList = m_deviceResources->GetCommandList();
        commandList->Reset(m_deviceResources->GetCommandAllocator(), nullptr);

        // Test DDSTextureLoader
        ComPtr<ID3D12Resource> ddsUploadHeap;
        ComPtr<ID3D12Resource> ddsUploadHeap2;

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
        {
            std::unique_ptr<uint8_t[]> ddsData;
            std::vector<D3D12_SUBRESOURCE_DATA> subresources;
            DX::ThrowIfFailed(LoadDDSTextureFromFile(device, L"dx5_logo.dds", m_dx5logo.ReleaseAndGetAddressOf(),
                ddsData, subresources));

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_dx5logo.Get(), 0, static_cast<UINT>(subresources.size()));

            auto bdesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            // Create the GPU upload buffer.
            DX::ThrowIfFailed(
                device->CreateCommittedResource(
                    &uploadHeap,
                    D3D12_HEAP_FLAG_NONE,
                    &bdesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(ddsUploadHeap.GetAddressOf())));

            UpdateSubresources(commandList, m_dx5logo.Get(), ddsUploadHeap.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_dx5logo.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandList->ResourceBarrier(1, &barrier);

            auto desc = m_dx5logo->GetDesc();

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = desc.MipLevels;

            device->CreateShaderResourceView(m_dx5logo.Get(), &srvDesc, cpuHandle.Offset(0, m_strideSRV));
        }

        {
            auto ddsData = DX::ReadData(L"dx5_logo.dds");

            std::vector<D3D12_SUBRESOURCE_DATA> subresources;
            DX::ThrowIfFailed(LoadDDSTextureFromMemory(device, ddsData.data(), ddsData.size(), m_test1.ReleaseAndGetAddressOf(),
                subresources));

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_test1.Get(), 0, static_cast<UINT>(subresources.size()));

            auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            // Create the GPU upload buffer.
            DX::ThrowIfFailed(
                device->CreateCommittedResource(
                    &uploadHeap,
                    D3D12_HEAP_FLAG_NONE,
                    &desc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(ddsUploadHeap2.GetAddressOf())));

            UpdateSubresources(commandList, m_test1.Get(), ddsUploadHeap2.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_test1.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandList->ResourceBarrier(1, &barrier);
        }

        // Test WICTextureLoader
        ComPtr<ID3D12Resource> wicUploadHeap;
        ComPtr<ID3D12Resource> wicUploadHeap2;

        {
            std::unique_ptr<uint8_t[]> decodedData;
            D3D12_SUBRESOURCE_DATA subresource;
            DX::ThrowIfFailed(LoadWICTextureFromFile(device, L"cup_small.jpg", m_cup.ReleaseAndGetAddressOf(),
                decodedData, subresource));

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_cup.Get(), 0, 1);

            auto bdesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            // Create the GPU upload buffer.
            DX::ThrowIfFailed(
                device->CreateCommittedResource(
                    &uploadHeap,
                    D3D12_HEAP_FLAG_NONE,
                    &bdesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(wicUploadHeap.GetAddressOf())));

            UpdateSubresources(commandList, m_cup.Get(), wicUploadHeap.Get(), 0, 0, 1, &subresource);
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_cup.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandList->ResourceBarrier(1, &barrier);

            auto desc = m_cup->GetDesc();

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = desc.MipLevels;

            device->CreateShaderResourceView(m_cup.Get(), &srvDesc, cpuHandle.Offset(1, m_strideSRV));
        }

        {
            auto blob = DX::ReadData(L"cup_small.jpg");

            std::unique_ptr<uint8_t[]> decodedData;
            D3D12_SUBRESOURCE_DATA subresource;
            DX::ThrowIfFailed(LoadWICTextureFromMemory(device, blob.data(), blob.size(), m_test2.ReleaseAndGetAddressOf(),
                decodedData, subresource));

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_test2.Get(), 0, 1);

            auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            // Create the GPU upload buffer.
            DX::ThrowIfFailed(
                device->CreateCommittedResource(
                    &uploadHeap,
                    D3D12_HEAP_FLAG_NONE,
                    &desc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(wicUploadHeap2.GetAddressOf())));

            UpdateSubresources(commandList, m_test2.Get(), wicUploadHeap2.Get(), 0, 0, 1, &subresource);
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_test2.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandList->ResourceBarrier(1, &barrier);
        }

        DX::ThrowIfFailed(commandList->Close());
        m_deviceResources->GetCommandQueue()->ExecuteCommandLists(1, CommandListCast(&commandList));

        // Wait until assets have been uploaded to the GPU.
        m_deviceResources->WaitForGpu();
    }
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
}

void Game::OnDeviceLost()
{
     m_cup.Reset();
     m_dx5logo.Reset();
     m_screenshot.Reset();

     m_test1.Reset();
     m_test2.Reset();

     m_srvHeap.Reset();
     m_rootSignature.Reset();
     m_pipelineState.Reset();
     m_vertexBuffer.Reset();
     m_indexBuffer.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
