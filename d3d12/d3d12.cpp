//-------------------------------------------------------=------------------------------
// d3d12.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#define D3DX12_NO_STATE_OBJECT_HELPERS
#define D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS

#ifdef USING_DIRECTX_HEADERS
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>
#else
#include <d3d12.h>
#include "d3dx12.h"
#endif

#include "DirectXTexP.h"

#include <wrl/client.h>

#include "getname.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace
{
    const UINT DXGI_START = 1;
    const UINT DXGI_END = 190; // as of DXGI 1.3

    enum
    {
        FLAGS_NONE = 0x0,
        FLAGS_YUV = 0x1,
        FLAGS_OPTIONAL = 0x2,
        FLAGS_SRGB = 0x4,
    };

    struct TestMedia
    {
        DWORD options;
        TexMetadata metadata;
        const wchar_t *fname;
    };

    const TestMedia g_TestMedia[] =
    {
        // test-options | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { FLAGS_SRGB, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds" },
        { FLAGS_SRGB, { 2048, 2048, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex4.png" },
        { FLAGS_SRGB, { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg" },
        { FLAGS_SRGB, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_DXT3.dds" },
        { FLAGS_SRGB, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
        { FLAGS_SRGB, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.dds" },
        { FLAGS_SRGB, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds" },
        { FLAGS_SRGB, { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lobbycube.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5mip.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1.dds" },
        { FLAGS_SRGB, { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1mip.dds" },
        { FLAGS_SRGB, { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds" },
        { FLAGS_SRGB, { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds" },
        { FLAGS_SRGB, { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvoldxt1mip.dds" },
        { FLAGS_SRGB, { 32, 1, 1, 1, 6, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE1D }, MEDIA_PATH L"io_R8G8B8A8_UNORM_SRV_DIMENSION_TEXTURE1D_MipOn.DDS" },
        { FLAGS_YUV, { 200, 200, 1, 1, 8, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaYUY2.dds" },
        { FLAGS_YUV, { 200, 200, 1, 1, 8, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaUYVY.dds" },
        { FLAGS_YUV, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv12.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P010, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP010.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaCubeYUY2.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 4, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"lenaVolYUY2.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_AYUV, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaAYUV.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y210, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY210.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y216, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY216.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y410, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY410.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y416, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY416.dds" },
        { FLAGS_YUV
          | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P016, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP016.dds" },
        { FLAGS_YUV
           | FLAGS_OPTIONAL, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV11, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv11.dds" },
    };
}

//-------------------------------------------------------------------------------------
extern HRESULT CreateDevice( ID3D12Device** pDev );
extern HRESULT SetupRenderTest(ID3D12Device** pDev, ID3D12CommandQueue** pCommandQ, ID3D12GraphicsCommandList** pCommandList);
extern HRESULT CreateCommmandList(ID3D12Device* pDev, ID3D12CommandQueue** pCommandQ, ID3D12GraphicsCommandList** pCommandList);
extern void UploadTest();
extern void RenderTest(const TexMetadata& metadata, ID3D12Resource* pResource);
extern void CleanupRenderTest();

//-------------------------------------------------------------------------------------
// planartest
bool Test01()
{
    ComPtr<ID3D12Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        return false;
    }

    bool success = true;

    for (UINT f = DXGI_START; f <= DXGI_END; ++f)
    {
        UINT count = D3D12GetFormatPlaneCount(device.Get(), static_cast<DXGI_FORMAT>(f));

        if (!IsValid(static_cast<DXGI_FORMAT>(f)))
            continue;

        if (!count)
        {
            // Not suppoorted for this device
        }
        else if (IsPlanar(static_cast<DXGI_FORMAT>(f)))
        {
            if (count == 1 && f != DXGI_FORMAT_420_OPAQUE)
            {
                printe("ERROR: D3D12 and IsPlanar don't agree on on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
        }
        else if (count > 1)
        {
            switch (f)
            {
            // Stencil formats
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            case 118 /*DXGI_FORMAT_D16_UNORM_S8_UINT*/:
            case 119 /*XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS*/:
            case 120 /*XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT*/:
                break;

            default:
                printe("ERROR: D3D12 and DirectXTex don't agree on on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
                break;
            }
        }
    }

    return success;
}

//-------------------------------------------------------------------------------------
// IsSupportedTexture
bool Test02()
{
    ComPtr<ID3D12Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);

        TexMetadata metadata;

        if (_wcsicmp(ext, L".dds") == 0)
        {
            hr = GetMetadataFromDDSFile(szPath, DDS_FLAGS_NONE, metadata);
        }
        else
        {
            hr = GetMetadataFromWICFile(szPath, WIC_FLAGS_NONE, metadata);
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if (FAILED(hr))
        {
            success = false;
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if (memcmp(&metadata, check, sizeof(TexMetadata)) != 0)
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta(&metadata);
            printmetachk(check);
        }
        else
        {
            bool pass = true;

            if (!IsSupportedTexture(device.Get(), metadata))
            {
                if (g_TestMedia[index].options & FLAGS_YUV)
                {
                    // Can't create video textures with mips on most hardware
                    metadata.mipLevels = 1;
                    if (!IsSupportedTexture(device.Get(), metadata))
                    {
                        if (!(g_TestMedia[index].options & FLAGS_OPTIONAL))
                        {
                            print("WARNING: Format %ls is not supported by this hardware\n", GetName(metadata.format));
                        }
                    }
                }
                else
                {
                    success = false;
                    pass = false;
                    printe("Failed testing data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
            }

            if (pass)
            {
                ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    return success;
}


//-------------------------------------------------------------------------------------
// CreateTexture
bool Test03()
{
    ComPtr<ID3D12Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);

        TexMetadata metadata;
        ScratchImage image;

        if (_wcsicmp(ext, L".dds") == 0)
        {
            hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, image);
        }
        else
        {
            hr = LoadFromWICFile(szPath, WIC_FLAGS_NONE, &metadata, image);
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if (FAILED(hr))
        {
            success = false;
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if (memcmp(&metadata, check, sizeof(TexMetadata)) != 0)
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta(&metadata);
            printmetachk(check);
        }
        else
        {
            if (g_TestMedia[index].options & FLAGS_YUV)
            {
                if (!IsSupportedTexture(device.Get(), metadata))
                {
                    // Can't create video textures with mips on most hardware
                    metadata.mipLevels = 1;

                    if (!IsSupportedTexture(device.Get(), metadata))
                    {
                        if (!(g_TestMedia[index].options & FLAGS_OPTIONAL))
                        {
                            print("WARNING: Format %ls is not supported by this hardware\n", GetName(metadata.format));
                        }
                        continue;
                    }
                }
            }

            ComPtr<ID3D12Resource> pResource;
            hr = CreateTexture(device.Get(), metadata, pResource.GetAddressOf());
            if (FAILED(hr))
            {
                success = false;
                printe("Failed creating texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else
            {
                bool pass = true;

                hr = CreateTextureEx(device.Get(), metadata, D3D12_RESOURCE_FLAG_NONE,
                    CREATETEX_DEFAULT, pResource.ReleaseAndGetAddressOf());
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("Failed creating texture ex from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }

                if (g_TestMedia[index].options & FLAGS_SRGB)
                {
                    // CREATETEX_FORCE_SRGB


                    hr = CreateTextureEx(device.Get(), metadata, D3D12_RESOURCE_FLAG_NONE,
                        CREATETEX_FORCE_SRGB, pResource.ReleaseAndGetAddressOf());
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed creating texture ex forceSRGB from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                    #if defined(_MSC_VER) || !defined(WIN32)
                        auto desc = pResource->GetDesc();
                    #else
                        D3D12_RESOURCE_DESC tmpDesc;
                        auto desc = *pResource->GetDesc(&tmpDesc);
                    #endif
                        DXGI_FORMAT format = desc.Format;
                        if (!IsSRGB(format))
                        {
                            success = false;
                            pass = false;
                            printe("Unexpected format %d for forceSRGB:\n%ls\n", format, szPath);
                        }
                    }

                    // CREATETEX_IGNORE_SRGB
                    hr = CreateTextureEx(device.Get(), metadata, D3D12_RESOURCE_FLAG_NONE,
                        CREATETEX_IGNORE_SRGB, pResource.ReleaseAndGetAddressOf());
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed creating texture ex ignoreSRGB from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                    #if defined(_MSC_VER) || !defined(WIN32)
                        auto desc = pResource->GetDesc();
                    #else
                        D3D12_RESOURCE_DESC tmpDesc;
                        auto desc = *pResource->GetDesc(&tmpDesc);
                    #endif
                        DXGI_FORMAT format = desc.Format;
                        if (IsSRGB(format))
                        {
                            success = false;
                            pass = false;
                            printe("Unexpected format %d for ignoreSRGB:\n%ls\n", format, szPath);
                        }
                    }
                }

                if (pass)
                    ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    return success;
}


//-------------------------------------------------------------------------------------
// PrepareUpload
bool Test04()
{
    ComPtr<ID3D12Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);

        TexMetadata metadata;
        ScratchImage image;

        if (_wcsicmp(ext, L".dds") == 0)
        {
            hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, image);
        }
        else
        {
            hr = LoadFromWICFile(szPath, WIC_FLAGS_NONE, &metadata, image);
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if (FAILED(hr))
        {
            success = false;
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if (memcmp(&metadata, check, sizeof(TexMetadata)) != 0)
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta(&metadata);
            printmetachk(check);
        }
        else
        {
            std::vector<D3D12_SUBRESOURCE_DATA> subresources;
            hr = PrepareUpload(device.Get(), image.GetImages(), image.GetImageCount(), metadata, subresources);
            if (FAILED(hr))
            {
                success = false;
                printe("Failed preparing texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else if (subresources.empty())
            {
                success = false;
                printe("Failed preparing texture with empty subresources result:\n%ls\n", szPath);
            }
            else
            {
                size_t expected = metadata.mipLevels * metadata.arraySize * D3D12GetFormatPlaneCount(device.Get(), metadata.format);
                if (subresources.size() != expected)
                {
                    success = false;
                    printe("Failed preparing texture with unexpected subresources result (%zu..%zu):\n%ls\n",
                        subresources.size(), expected, szPath);
                }
                else
                {
                    ++npass;
                }
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    return success;
}


//-------------------------------------------------------------------------------------
// RenderTest
bool Test05()
{
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    HRESULT hr = SetupRenderTest(device.GetAddressOf(), queue.GetAddressOf(), commandList.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        if (g_TestMedia[index].options & FLAGS_YUV)
        {
            // Skip video textures which need special options to render
            continue;
        }

        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);

        TexMetadata metadata;
        ScratchImage image;

        if (_wcsicmp(ext, L".dds") == 0)
        {
            hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, image);
        }
        else
        {
            hr = LoadFromWICFile(szPath, WIC_FLAGS_NONE, &metadata, image);
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if (FAILED(hr))
        {
            success = false;
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if (memcmp(&metadata, check, sizeof(TexMetadata)) != 0)
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta(&metadata);
            printmetachk(check);
        }
        else
        {
            ComPtr<ID3D12Resource> pResource;
            hr = CreateTexture(device.Get(), metadata, pResource.GetAddressOf());
            if (FAILED(hr))
            {
                success = false;
                printe("Failed preparing texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else
            {
                std::vector<D3D12_SUBRESOURCE_DATA> subresources;
                hr = PrepareUpload(device.Get(), image.GetImages(), image.GetImageCount(), metadata, subresources);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed preparing texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else if (subresources.empty())
                {
                    success = false;
                    printe("Failed preparing texture with empty subresources result:\n%ls\n", szPath);
                }
                else
                {
                    // Manual upload
                    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pResource.Get(), 0, static_cast<unsigned int>(subresources.size()));

                    CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

                    auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

                    ComPtr<ID3D12Resource> textureUploadHeap;
                    hr = device->CreateCommittedResource(
                        &uploadHeap,
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        D3D12_RESOURCE_STATE_GENERIC_READ,
                        nullptr,
                        IID_PPV_ARGS(textureUploadHeap.GetAddressOf()));
                    if (FAILED(hr))
                    {
                        success = false;
                        printe("Failed creating upload texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                        UpdateSubresources(commandList.Get(), pResource.Get(), textureUploadHeap.Get(),
                                           0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

                        D3D12_RESOURCE_BARRIER barrier = {};
                        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                        barrier.Transition.pResource = pResource.Get();
                        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

                        commandList->ResourceBarrier(1, &barrier);

                        print("Viewing %ls\n", szPath);

                        RenderTest(metadata, pResource.Get());

                        ++npass;
                    }
                }
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    queue.Reset();
    commandList.Reset();
    device.Reset();
    CleanupRenderTest();

    return success;
}


//-------------------------------------------------------------------------------------
// CaptureTexture
bool Test06()
{
    ComPtr<ID3D12Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        return false;
    }

    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    hr = CreateCommmandList(device.Get(), queue.GetAddressOf(), commandList.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating command list (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        if (g_TestMedia[index].options & FLAGS_YUV)
        {
            // Skip video textures which need special options to render
            continue;
        }

        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

        wchar_t tempDir[MAX_PATH];
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"d3d12", tempDir, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

        CreateDirectoryW(tempDir, nullptr);

        wchar_t szDestPath[MAX_PATH];
        _wmakepath_s(szDestPath, MAX_PATH, nullptr, tempDir, fname, L".dds");

        TexMetadata metadata;
        ScratchImage image;

        if (_wcsicmp(ext, L".dds") == 0)
        {
            hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, image);
        }
        else
        {
            hr = LoadFromWICFile(szPath, WIC_FLAGS_NONE, &metadata, image);
        }

        bool forceNoMips = false;
        const TexMetadata* check = &g_TestMedia[index].metadata;
        if (FAILED(hr))
        {
            success = false;
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if (memcmp(&metadata, check, sizeof(TexMetadata)) != 0)
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta(&metadata);
            printmetachk(check);
        }
        else
        {
            if (g_TestMedia[index].options & FLAGS_YUV)
            {
                if (!IsSupportedTexture(device.Get(), metadata))
                {
                    // Can't create video textures with mips on most hardware
                    metadata.mipLevels = 1;
                    forceNoMips = true;

                    if (!IsSupportedTexture(device.Get(), metadata))
                    {
                        if (!(g_TestMedia[index].options & FLAGS_OPTIONAL))
                        {
                            print("WARNING: Format %ls is not supported by this hardware\n", GetName(metadata.format));
                        }
                        continue;
                    }
                }
            }

            ComPtr<ID3D12Resource> pResource;
            hr = CreateTexture(device.Get(), metadata, pResource.GetAddressOf());
            if (FAILED(hr))
            {
                success = false;
                printe("Failed preparing texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else
            {
                std::vector<D3D12_SUBRESOURCE_DATA> subresources;
                hr = PrepareUpload(device.Get(), image.GetImages(), image.GetImageCount(), metadata, subresources);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed preparing texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else
                {
                    // Manual upload
                    CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

                    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pResource.Get(), 0, static_cast<unsigned int>(subresources.size()));

                    auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

                    ComPtr<ID3D12Resource> textureUploadHeap;
                    hr = device->CreateCommittedResource(
                        &uploadHeap,
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        D3D12_RESOURCE_STATE_GENERIC_READ,
                        nullptr,
                        IID_PPV_ARGS(textureUploadHeap.GetAddressOf()));
                    if (FAILED(hr))
                    {
                        success = false;
                        printe("Failed creating upload texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                        bool pass = false;

                        UpdateSubresources(commandList.Get(), pResource.Get(), textureUploadHeap.Get(),
                            0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

                        UploadTest();

                        ScratchImage image2;
                        hr = CaptureTexture(queue.Get(), pResource.Get(),
                            image.GetMetadata().IsCubemap(),
                            image2,
                            D3D12_RESOURCE_STATE_COPY_DEST,
                            D3D12_RESOURCE_STATE_COPY_DEST);

                        if (FAILED(hr))
                        {
                            success = false;
                            printe("Failed capturing texture from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                        }
                        else
                        {
                            const TexMetadata& mdata2 = image2.GetMetadata();

                            if (mdata2.width != metadata.width
                                || mdata2.height != metadata.height
                                || mdata2.depth != metadata.depth
                                || mdata2.arraySize != metadata.arraySize
                                || mdata2.mipLevels != metadata.mipLevels
                                || mdata2.miscFlags != metadata.miscFlags /* ignore miscFlags2 */
                                || mdata2.format != metadata.format
                                || mdata2.dimension != metadata.dimension)
                            {
                                success = false;
                                printe("Metadata error in:\n%ls\n", szDestPath);
                                printmeta(&mdata2);
                                printmetachk(check);
                            }
                            else if (!forceNoMips && image.GetImageCount() != image2.GetImageCount())
                            {
                                success = false;
                                printe("Image count in captured texture (%zu) doesn't match source (%zu) in:\n%ls\n", image2.GetImageCount(), image.GetImageCount(), szDestPath);
                            }
                            else
                            {
                                hr = SaveToDDSFile(image2.GetImages(), image2.GetImageCount(), image2.GetMetadata(), DDS_FLAGS_NONE, szDestPath);
                                if (FAILED(hr))
                                {
                                    success = false;
                                    pass = false;
                                    printe("Failed writing DDS to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath);
                                }

                                if (!IsPlanar(metadata.format))
                                {
                                    float mse, mseV[4];
                                    hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                                    if (FAILED(hr))
                                    {
                                        success = false;
                                        pass = false;
                                        printe("Failed comparing captured image (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                                    }
                                    else if (fabs(mse) > 0.000001f)
                                    {
                                        success = false;
                                        pass = false;
                                        printe("Failed comparing captured image MSE = %f (%f %f %f %f)... 0.f:\n%ls\n",
                                            mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath);
                                    }
                                }
                            }
                        }

                        if (pass)
                            ++npass;
                    }
                }
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    queue.Reset();
    commandList.Reset();
    device.Reset();
    CleanupRenderTest();

    return success;
}


//-------------------------------------------------------------------------------------
// CalcuateSubresource
bool Test07()
{
    bool success = true;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        const auto& metadata = g_TestMedia[index].metadata;

        if (metadata.IsVolumemap())
        {
            for (uint32_t level = 0; level < metadata.mipLevels; ++level)
            {
                uint32_t expected = D3D12CalcSubresource(level, 0, 0, static_cast<UINT>(metadata.mipLevels), 1);
                uint32_t result = metadata.CalculateSubresource(level, 0);

                if (expected != result)
                {
                    success = false;
                    printe("Failed CalcuateSubresource [3D %zu] %u = %u...%u\n", index, level, result, expected);
                }
            }

            // No planar formats really used, so this test is synthetic
            for (uint32_t plane = 0; plane < 3; ++plane)
            {
                for (uint32_t level = 0; level < metadata.mipLevels; ++level)
                {
                    uint32_t expected = D3D12CalcSubresource(level, 0, plane, static_cast<UINT>(metadata.mipLevels), 1);
                    uint32_t result = metadata.CalculateSubresource(level, 0, plane);

                    if (expected != result)
                    {
                        success = false;
                        printe("Failed CalcuateSubresource [3D planar %zu] %u, %u = %u...%u\n", index, level, plane, result, expected);
                    }
                }
            }
        }
        else
        {
            for (uint32_t item = 0; item < metadata.arraySize; ++item)
            {
                for (uint32_t level = 0; level < metadata.mipLevels; ++level)
                {
                    uint32_t expected = D3D12CalcSubresource(level, item, 0, static_cast<UINT>(metadata.mipLevels), static_cast<UINT>(metadata.arraySize));
                    uint32_t result = metadata.CalculateSubresource(level, item);

                    if (expected != result)
                    {
                        success = false;
                        printe("Failed CalcuateSubresource [2D %zu] %u, %u = %u...%u\n", index, item, level, result, expected);
                    }
                }
            }

            // No planar formats really used, so this test is synthetic
            for (uint32_t plane = 0; plane < 3; ++plane)
            {
                for (uint32_t item = 0; item < metadata.arraySize; ++item)
                {
                    for (uint32_t level = 0; level < metadata.mipLevels; ++level)
                    {
                        uint32_t expected = D3D12CalcSubresource(level, item, plane, static_cast<UINT>(metadata.mipLevels), static_cast<UINT>(metadata.arraySize));
                        uint32_t result = metadata.CalculateSubresource(level, item, plane);

                        if (expected != result)
                        {
                            success = false;
                            printe("Failed CalcuateSubresource [2D planar %zu] %u, %u, %u = %u...%u\n", index, item, level, plane, result, expected);
                        }
                    }
                }
            }
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// formattest
#ifdef USING_DIRECTX_HEADERS
bool Test08()
{
    bool success = true;

    for (UINT f = DXGI_START; f <= DXGI_END; ++f)
    {
        if (!IsValid(static_cast<DXGI_FORMAT>(f)))
            continue;

        if (!D3D12_PROPERTY_LAYOUT_FORMAT_TABLE::FormatExists(static_cast<DXGI_FORMAT>(f)))
            continue;

        if ((f == XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT)
            || (f == XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS)
            || (f == XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT)
            || (f == XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM)
            || (f == XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT)
            || (f == XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT))
            continue;

        size_t bbp = BitsPerPixel(static_cast<DXGI_FORMAT>(f));
        size_t d3dxbpu = D3D12_PROPERTY_LAYOUT_FORMAT_TABLE::GetBitsPerUnit(static_cast<DXGI_FORMAT>(f));
        switch(f)
        {
            case DXGI_FORMAT_R8G8_B8G8_UNORM:
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
                d3dxbpu = 32;
                break;

            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM:
                d3dxbpu = 4;
                break;

            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16:
            case DXGI_FORMAT_BC7_TYPELESS:
            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                d3dxbpu = 8;
                break;

            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_420_OPAQUE:
            case DXGI_FORMAT_NV11:
                d3dxbpu = 12;
                break;

            case WIN10_DXGI_FORMAT_P208:
            case WIN10_DXGI_FORMAT_V208:
                d3dxbpu = 16;
                break;

            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:
            case WIN10_DXGI_FORMAT_V408:
                d3dxbpu = 24;
                break;

            case DXGI_FORMAT_YUY2:
                d3dxbpu = 32;
                break;

            case DXGI_FORMAT_Y210:
            case DXGI_FORMAT_Y216:
                d3dxbpu = 64;
                break;

            default:
                break;
        }

        if (bbp != d3dxbpu)
        {
            printe("ERROR: BitsPerPixel mismatch with D3DX12 on DXGI Format %u (%zu .. %zu)\n", f, bbp, d3dxbpu);
            success = false;
        }

        if (IsCompressed(static_cast<DXGI_FORMAT>(f)) != D3D12_PROPERTY_LAYOUT_FORMAT_TABLE::IsBlockCompressFormat(static_cast<DXGI_FORMAT>(f)))
        {
            printe("ERROR: IsCompressed mismatch with D3DX12 on DXGI Format %u\n", f);
            success = false;
        }

        if (IsSRGB(static_cast<DXGI_FORMAT>(f)) != D3D12_PROPERTY_LAYOUT_FORMAT_TABLE::IsSRGBFormat(static_cast<DXGI_FORMAT>(f)))
        {
            printe("ERROR: IsSRGB mismatch with D3DX12 on DXGI Format %u\n", f);
            success = false;
        }

        if (IsPlanar(static_cast<DXGI_FORMAT>(f), true) != static_cast<bool>(D3D12_PROPERTY_LAYOUT_FORMAT_TABLE::Planar(static_cast<DXGI_FORMAT>(f)) != 0))
        {
            printe("ERROR: IsPlanar mismatch with D3DX12 on DXGI Format %u\n", f);
            success = false;
        }

        if (IsVideo(static_cast<DXGI_FORMAT>(f)) != static_cast<bool>(D3D12_PROPERTY_LAYOUT_FORMAT_TABLE::YUV(static_cast<DXGI_FORMAT>(f)) != 0))
        {
            printe("ERROR: IsVideo mismatch with D3DX12 on DXGI Format %u\n", f);
            success = false;
        }
    }

    return success;
}
#endif // USING_DIRECTX_HEADERS
