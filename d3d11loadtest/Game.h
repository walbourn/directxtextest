//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();
    void Render();

    // Rendering helpers
    void Clear();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    uint64_t                                m_frame;

    // Test objects.
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_cup;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_dx5logo;

    Microsoft::WRL::ComPtr<ID3D11InputLayout>           m_spInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>                m_spVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>                m_spIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>          m_spVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           m_spPixelShader;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_spSampler;
};
