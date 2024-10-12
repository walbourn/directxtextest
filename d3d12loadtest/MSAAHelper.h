//--------------------------------------------------------------------------------------
// File: MSAAHelper.h
//
// Helper for managing MSAA render targets (DirectX 12 version)
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//-------------------------------------------------------------------------------------

#pragma once

#include <DirectXMath.h>

#include <wrl/client.h>

#include <tuple>


namespace DX
{
    class MSAAHelper
    {
    public:
        explicit MSAAHelper(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
            unsigned int sampleCount = 4) noexcept(false);

        MSAAHelper(MSAAHelper&&) = default;
        MSAAHelper& operator= (MSAAHelper&&) = default;

        MSAAHelper(MSAAHelper const&) = delete;
        MSAAHelper& operator= (MSAAHelper const&) = delete;

        void SetDevice(_In_ ID3D12Device* device);

        void SizeResources(size_t width, size_t height);

        void ReleaseDevice();

        void Prepare(_In_ ID3D12GraphicsCommandList* commandList,
            D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

        void Resolve(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* backBuffer,
            D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_PRESENT);

        void Transition(_In_ ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

        void SetWindow(const RECT& rect);

        ID3D12Resource* GetMSAARenderTarget() const noexcept { return m_msaaRenderTarget.Get(); }
        ID3D12Resource* GetMSAADepthStencil() const noexcept { return m_msaaDepthStencil.Get(); }

        D3D12_CPU_DESCRIPTOR_HANDLE GetMSAARenderTargetView() const noexcept
        {
#if defined(_MSC_VER) || !defined(_WIN32)
            return m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
#else
            D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
            std::ignore = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(&hCPU);
            return hCPU;
#endif
        }
        D3D12_CPU_DESCRIPTOR_HANDLE GetMSAADepthStencilView() const noexcept
        {
#if defined(_MSC_VER) || !defined(_WIN32)
            return m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
#else
            D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
            std::ignore = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(&hCPU);
            return hCPU;
#endif
        }

        void SetClearColor(DirectX::FXMVECTOR color)
        {
            DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(m_clearColor), color);
        }

        DXGI_FORMAT GetBackBufferFormat() const noexcept { return m_backBufferFormat; }
        DXGI_FORMAT GetDepthBufferFormat() const noexcept { return m_depthBufferFormat; }
        unsigned int GetSampleCount() const noexcept { return m_sampleCount; }

    private:
        Microsoft::WRL::ComPtr<ID3D12Device>                m_device;
        Microsoft::WRL::ComPtr<ID3D12Resource>              m_msaaRenderTarget;
        Microsoft::WRL::ComPtr<ID3D12Resource>              m_msaaDepthStencil;
        float                                               m_clearColor[4];

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_rtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_dsvDescriptorHeap;

        DXGI_FORMAT                                         m_backBufferFormat;
        DXGI_FORMAT                                         m_depthBufferFormat;
        unsigned int                                        m_sampleCount;
        unsigned int                                        m_targetSampleCount;

        size_t                                              m_width;
        size_t                                              m_height;
    };
}
