//-------------------------------------------------------------------------------------
// d3d11.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

enum
{
    FLAGS_NONE = 0x0,
    FLAGS_YUV = 0x1,
    FLAGS_NOT_SUPPORTED = 0x2,
};

struct TestMedia
{
    DWORD options;
    TexMetadata metadata;
    const wchar_t *fname;
};

static const TestMedia g_TestMedia[] = 
{
// test-options | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
{ FLAGS_NONE, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds" },
{ FLAGS_NONE, { 2048, 2048, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex4.png" },
{ FLAGS_NONE, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg" },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_DXT3.dds" },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
{ FLAGS_NONE, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.dds" },
{ FLAGS_NONE, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.dds" },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds" },
{ FLAGS_NONE, { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds" },
{ FLAGS_NONE, { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lobbycube.dds" },
{ FLAGS_NONE, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds" },
{ FLAGS_NONE, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds" },
{ FLAGS_NONE, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5.dds" },
{ FLAGS_NONE, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5mip.dds" },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1.dds" },
{ FLAGS_NONE, { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1mip.dds" },
{ FLAGS_NONE, { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds" },
{ FLAGS_NONE, { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds" },
{ FLAGS_NONE, { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvoldxt1mip.dds" },
{ FLAGS_NONE, { 32, 1, 1, 1, 6, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE1D }, MEDIA_PATH L"io_R8G8B8A8_UNORM_SRV_DIMENSION_TEXTURE1D_MipOn.DDS" },
{ FLAGS_YUV, { 200, 200, 1, 1, 8, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaYUY2.dds" },
{ FLAGS_YUV, { 200, 200, 1, 1, 8, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaUYVY.dds" },
{ FLAGS_YUV, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv12.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P010, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP010.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaCubeYUY2.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 4, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"lenaVolYUY2.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_AYUV, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaAYUV.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y210, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY210.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y216, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY216.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y410, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY410.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y416, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaY416.dds" },
{ FLAGS_YUV
  | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P016, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP016.dds" },
{ FLAGS_YUV
   | FLAGS_NOT_SUPPORTED, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV11, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv11.dds" },
};

//-------------------------------------------------------------------------------------
extern HRESULT CreateDevice( ID3D11Device** pDev, ID3D11DeviceContext** pContext );
extern HRESULT SetupRenderTest( ID3D11Device** pDev, ID3D11DeviceContext** pContext );
extern void RenderTest( const TexMetadata& metadata, ID3D11ShaderResourceView* pSRV );
extern void CleanupRenderTest();

//-------------------------------------------------------------------------------------
// IsSupportedTexture
bool Test01()
{
    ComPtr<ID3D11Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf(), nullptr);
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", hr);
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(g_TestMedia); ++index)
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
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath);
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
                        if (!(g_TestMedia[index].options & FLAGS_NOT_SUPPORTED))
                        {
                            print("WARNING: Format %u is not supported by this hardware\n", metadata.format);
                        }
                    }
                }
                else
                {
                    success = false;
                    pass = false;
                    printe("Failed testing data from (HRESULT %08X):\n%ls\n", hr, szPath);
                }
            }

            if (pass)
            {
                ++npass;
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass);

    return success;
}

//-------------------------------------------------------------------------------------
// CreateTexture
bool Test02()
{
    ComPtr<ID3D11Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf(), nullptr);
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", hr);
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(g_TestMedia); ++index)
    {
        if (g_TestMedia[index].options & FLAGS_NOT_SUPPORTED)
        {
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
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath);
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
                        print("WARNING: Format %u is not supported by this hardware\n", metadata.format);
                        continue;
                    }
                }
            }

            ComPtr<ID3D11Resource> pResource;
            hr = CreateTexture(device.Get(), image.GetImages(), image.GetImageCount(), metadata, pResource.GetAddressOf());
            if (FAILED(hr))
            {
                success = false;
                printe("Failed creating texture from (HRESULT %08X):\n%ls\n", hr, szPath);
            }
            else
            {
                hr = CreateTextureEx(device.Get(), image.GetImages(), image.GetImageCount(), metadata,
                    D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0,
                    true, pResource.ReleaseAndGetAddressOf());
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed creating texture ex from (HRESULT %08X):\n%ls\n", hr, szPath);
                }
                else
                {
                    ++npass;
                }
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass);

    return success;
}

//-------------------------------------------------------------------------------------
// CreateShaderResourceView
bool Test03()
{
    ComPtr<ID3D11Device> device;
    HRESULT hr = CreateDevice(device.GetAddressOf(), nullptr);
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", hr);
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(g_TestMedia); ++index)
    {
        if (g_TestMedia[index].options & (FLAGS_YUV | FLAGS_NOT_SUPPORTED))
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
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath);
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
            ComPtr<ID3D11ShaderResourceView> pSRV;
            hr = CreateShaderResourceView(device.Get(), image.GetImages(), image.GetImageCount(), metadata, pSRV.GetAddressOf());
            if (FAILED(hr))
            {
                success = false;
                printe("Failed creating SRV from (HRESULT %08X):\n%ls\n", hr, szPath);
            }
            else
            {
                hr = CreateShaderResourceViewEx(device.Get(), image.GetImages(), image.GetImageCount(), metadata,
                    D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, pSRV.ReleaseAndGetAddressOf());
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed creating SRV ex from (HRESULT %08X):\n%ls\n", hr, szPath);
                }
                else
                {
                    ++npass;
                }
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass);

    return success;
}

//-------------------------------------------------------------------------------------
// rendertest
bool Test04()
{
    ComPtr<ID3D11Device> device;
    HRESULT hr = SetupRenderTest(device.GetAddressOf(), nullptr);
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", hr);
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(g_TestMedia); ++index)
    {
        if (g_TestMedia[index].options & (FLAGS_YUV | FLAGS_NOT_SUPPORTED))
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
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath);
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
            ComPtr<ID3D11ShaderResourceView> pSRV;
            hr = CreateShaderResourceView(device.Get(), image.GetImages(), image.GetImageCount(), metadata, pSRV.GetAddressOf());
            if (FAILED(hr))
            {
                success = false;
                printe("Failed creating SRV from (HRESULT %08X):\n%ls\n", hr, szPath);
            }
            else
            {
                print("Viewing %ls\n", szPath);

                RenderTest(metadata, pSRV.Get());

                ++npass;
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass);

    CleanupRenderTest();

    return success;
}

//-------------------------------------------------------------------------------------
// CaptureTexture
bool Test05()
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    HRESULT hr = CreateDevice(device.GetAddressOf(), context.GetAddressOf());
    if (FAILED(hr))
    {
        printe("Failed creating device (HRESULT %08X)\n", hr);
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(g_TestMedia); ++index)
    {
        if (g_TestMedia[index].options & FLAGS_NOT_SUPPORTED)
        {
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"d3d11", tempDir, MAX_PATH);
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
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath);
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

            if (g_TestMedia[index].options & FLAGS_YUV)
            {
                if (!IsSupportedTexture(device.Get(), metadata))
                {
                    // Can't create video textures with mips on most hardware
                    metadata.mipLevels = 1;
                    forceNoMips = true;

                    if (!IsSupportedTexture(device.Get(), metadata))
                    {
                        print("WARNING: Format %u is not supported by this hardware\n", metadata.format);
                        continue;
                    }
                }
            }

            {
                ComPtr<ID3D11Resource> pResource;
                hr = CreateTexture(device.Get(), image.GetImages(), image.GetImageCount(), metadata, pResource.GetAddressOf());
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed creating texture from (HRESULT %08X):\n%ls\n", hr, szPath);
                }
                else
                {
                    ScratchImage image2;
                    hr = CaptureTexture(device.Get(), context.Get(), pResource.Get(), image2);

                    if (FAILED(hr))
                    {
                        success = false;
                        printe("Failed capturing texture from (HRESULT %08X):\n%ls\n", hr, szPath);
                    }
                    else
                    {
                        const TexMetadata& mdata2 = image2.GetMetadata();

                        if (memcmp(&mdata2, &metadata, sizeof(TexMetadata)) != 0)
                        {
                            success = false;
                            printe("Metadata error in:\n%ls\n", szDestPath);
                            printmeta(&mdata2);
                            printmetachk(check);
                        }
                        else if (!forceNoMips && image.GetImageCount() != image2.GetImageCount())
                        {
                            success = false;
                            printe("Image count in captured texture (%Iu) doesn't match source (%Iu) in:\n%ls\n", image2.GetImageCount(), image.GetImageCount(), szDestPath);
                        }
                        else
                        {
                            hr = SaveToDDSFile(image2.GetImages(), image2.GetImageCount(), image2.GetMetadata(), DDS_FLAGS_NONE, szDestPath);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed writing DDS to (HRESULT %08X):\n%ls\n", hr, szDestPath);
                            }

                            if (!IsPlanar(metadata.format))
                            {
                                float mse, mseV[4];
                                hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                                if (FAILED(hr))
                                {
                                    success = false;
                                    pass = false;
                                    printe("Failed comparing captured image (HRESULT %08X):\n%ls\n", hr, szPath);
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
                }
            }

            // Staging resource tests
            {
                ComPtr<ID3D11Resource> pStaging;
                hr = CreateTextureEx(device.Get(), image.GetImages(), image.GetImageCount(), metadata,
                    D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0, false, pStaging.GetAddressOf());
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("Failed creating test staging texture (HRESULT %08X):\n%ls\n", hr, szPath);
                }
                else
                {
                    ScratchImage image2;
                    hr = CaptureTexture(device.Get(), context.Get(), pStaging.Get(), image2);

                    if (FAILED(hr))
                    {
                        success = false;
                        printe("Failed capturing texture from staging texture (HRESULT %08X):\n%ls\n", hr, szPath);
                    }
                    else
                    {
                        const TexMetadata& mdata2 = image2.GetMetadata();

                        if (memcmp(&mdata2, &metadata, sizeof(TexMetadata)) != 0)
                        {
                            success = false;
                            printe("Metadata error in:\n%ls\n", szDestPath);
                            printmeta(&mdata2);
                            printmetachk(check);
                        }
                        else if (!forceNoMips && image.GetImageCount() != image2.GetImageCount())
                        {
                            success = false;
                            printe("Image count in captured texture staging texture (%Iu) doesn't match source (%Iu) in:\n%ls\n", image2.GetImageCount(), image.GetImageCount(), szDestPath);
                        }
                        else
                        {
                            wchar_t tname[MAX_PATH] = {};
                            wcscpy_s(tname, fname);
                            wcscat_s(tname, L"_staging");

                            wchar_t szDestPath2[MAX_PATH];
                            _wmakepath_s(szDestPath2, MAX_PATH, nullptr, tempDir, tname, L".dds");

                            hr = SaveToDDSFile(image2.GetImages(), image2.GetImageCount(), image2.GetMetadata(), DDS_FLAGS_NONE, szDestPath2);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed writing DDS to (HRESULT %08X):\n%ls\n", hr, szDestPath2);
                            }

                            if (!IsPlanar(metadata.format))
                            {
                                float mse, mseV[4];
                                hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                                if (FAILED(hr))
                                {
                                    success = false;
                                    pass = false;
                                    printe("Failed comparing captured image (HRESULT %08X):\n%ls\n", hr, szPath);
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
                }
            }

            // CaptureTexture of an MSAA resource is tested elsewhere

            if (pass)
                ++npass;
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass);

    return success;
}