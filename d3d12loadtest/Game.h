//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "MSAAHelper.h"


// A basic game implementation that creates a D3D12 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    uint64_t                                m_frame;

    // Test objects.
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_cup;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_dx5logo;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_screenshot;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_screenshotMSAA;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_test1;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_test2;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_srvHeap;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>     m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>     m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                        m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW                         m_indexBufferView;

    UINT                                            m_strideSRV;

    std::unique_ptr<DX::MSAAHelper>                 m_msaaHelper;
};
