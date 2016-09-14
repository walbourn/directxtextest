//-------------------------------------------------------------------------------------
// d3d11.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

#include <wrl.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct TestMedia
{
    TexMetadata metadata;
    const wchar_t *fname;
    // TODO - Checksum to verify loaded image?
};

static const TestMedia g_TestMedia[] = 
{
// width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
{ { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds" },
{ { 2048, 2048, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex4.png" },
{ { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg" },
{ { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_DXT3.dds" },
{ { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
{ { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.dds" },
{ { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.dds" },
{ { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds" },
{ { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds" },
{ { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lobbycube.dds" },
{ { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds" },
{ { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds" },
{ { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5.dds" },
{ { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5mip.dds" },
{ { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1.dds" },
{ { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1mip.dds" },
{ { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds" },
{ { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds" },
{ { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvoldxt1mip.dds" },
{ { 32, 1, 1, 1, 6, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE1D }, MEDIA_PATH L"io_R8G8B8A8_UNORM_SRV_DIMENSION_TEXTURE1D_MipOn.DDS" },
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
    HRESULT hr = CreateDevice( device.GetAddressOf(), NULL );
    if ( FAILED(hr) )
    {
        printe( "Failed creating device (HRESULT %08X)\n", hr );
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT );

        TexMetadata metadata;

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = GetMetadataFromDDSFile( szPath, DDS_FLAGS_NONE, metadata );
        }
        else
        {
            hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_NONE, metadata );
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            if ( !IsSupportedTexture( device.Get(), metadata ) )
            {
                success = false;
                printe( "Failed testing data from (HRESULT %08X):\n%ls\n", hr, szPath );
            }
            else
                ++npass;
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}

//-------------------------------------------------------------------------------------
// CreateTexture
bool Test02()
{
    ComPtr<ID3D11Device> device;
    HRESULT hr = CreateDevice( device.GetAddressOf(), NULL );
    if ( FAILED(hr) )
    {
        printe( "Failed creating device (HRESULT %08X)\n", hr );
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT );

        TexMetadata metadata;
        ScratchImage image;

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );
        }
        else
        {
            hr = LoadFromWICFile( szPath, WIC_FLAGS_NONE, &metadata, image );
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            ID3D11Resource* pResource = NULL;

            hr = CreateTexture( device.Get(), image.GetImages(), image.GetImageCount(), metadata, &pResource );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed creating texture from (HRESULT %08X):\n%ls\n", hr, szPath );
            }
            else
            {
                pResource->Release();

                hr = CreateTextureEx( device.Get(), image.GetImages(), image.GetImageCount(), metadata,
                                      D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &pResource );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed creating texture ex from (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else
                {
                    pResource->Release();

                    ++npass;
                }
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}

//-------------------------------------------------------------------------------------
// CreateShaderResourceView
bool Test03()
{
    ComPtr<ID3D11Device> device;
    HRESULT hr = SetupRenderTest( device.GetAddressOf(), NULL );
    if ( FAILED(hr) )
    {
        printe( "Failed creating device (HRESULT %08X)\n", hr );
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT );

        TexMetadata metadata;
        ScratchImage image;

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );
        }
        else
        {
            hr = LoadFromWICFile( szPath, WIC_FLAGS_NONE, &metadata, image );
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            ID3D11ShaderResourceView* pSRV = NULL;

            hr = CreateShaderResourceView( device.Get(), image.GetImages(), image.GetImageCount(), metadata, &pSRV );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed creating SRV from (HRESULT %08X):\n%ls\n", hr, szPath );
            }
            else
            {
                print( "Viewing %ls\n", szPath );

                RenderTest( metadata, pSRV );

                pSRV->Release();

                hr = CreateShaderResourceViewEx( device.Get(), image.GetImages(), image.GetImageCount(), metadata,
                                                 D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &pSRV );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed creating SRV ex from (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else
                {
                    pSRV->Release();

                    ++npass;
                }
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    CleanupRenderTest();

    return success;
}

//-------------------------------------------------------------------------------------
// CaptureTexture
bool Test04()
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    HRESULT hr = CreateDevice( device.GetAddressOf(), context.GetAddressOf() );
    if ( FAILED(hr) )
    {
        printe( "Failed creating device (HRESULT %08X)\n", hr );
        return false;
    }

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif
        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        wchar_t tempDir[MAX_PATH];
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"d3d11", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, NULL );

        wchar_t szDestPath[MAX_PATH];
        _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, fname, L".dds" );

        TexMetadata metadata;
        ScratchImage image;

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );
        }
        else
        {
            hr = LoadFromWICFile( szPath, WIC_FLAGS_NONE, &metadata, image );
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting data from (HRESULT %08X):\n%ls\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;

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

                        if (memcmp(&mdata2, check, sizeof(TexMetadata)) != 0)
                        {
                            success = false;
                            printe("Metadata error in:\n%ls\n", szDestPath);
                            printmeta(&mdata2);
                            printmetachk(check);
                        }
                        else if (image.GetImageCount() != image2.GetImageCount())
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

                        if (memcmp(&mdata2, check, sizeof(TexMetadata)) != 0)
                        {
                            success = false;
                            printe("Metadata error in:\n%ls\n", szDestPath);
                            printmeta(&mdata2);
                            printmetachk(check);
                        }
                        else if (image.GetImageCount() != image2.GetImageCount())
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
                            _wmakepath_s(szDestPath2, MAX_PATH, NULL, tempDir, tname, L".dds");

                            hr = SaveToDDSFile(image2.GetImages(), image2.GetImageCount(), image2.GetMetadata(), DDS_FLAGS_NONE, szDestPath2);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed writing DDS to (HRESULT %08X):\n%ls\n", hr, szDestPath2);
                            }

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

            // CaptureTexture of an MSAA resource is tested elsewhere

            if (pass)
                ++npass;
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}