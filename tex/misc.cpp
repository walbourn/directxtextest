//-------------------------------------------------------------------------------------
// misc.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"
#include "tex.h"

#include "directxtex.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------

extern HRESULT MD5Checksum(_In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages = 0);
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------

namespace
{
    inline bool IsEqual(float f1, float f2)
    {
        return (fabs(f1 - f2) < 0.000001) != 0;
    }
}

//-------------------------------------------------------------------------------------
// CopyRectangle
bool TEXTest::Test09()
{
    bool success = true;

    // Test image
    wchar_t szPath[MAX_PATH] = {};
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"test8888.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#if defined(_DEBUG) && defined(VERBOSE)
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    // Form dest path
    wchar_t ext[_MAX_EXT];
    wchar_t fname[_MAX_FNAME];
    _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

    wchar_t tempDir[MAX_PATH] = {};
    ret = ExpandEnvironmentStringsW(TEMP_PATH L"cpy", tempDir, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

    CreateDirectoryW( tempDir, nullptr );

    const TexMetadata checkTest = { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    TexMetadata metadata;
    ScratchImage imageTest;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_FORCE_RGB, &metadata, imageTest );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkTest, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%ls\n", szPath );
        printmeta( &metadata );
        printmetachk( &checkTest );
        return false;
    }

    // Windows Logo
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"windowslogo_X8R8G8B8.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#if defined(_DEBUG) && defined(VERBOSE)
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    const TexMetadata checkLogo = { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    ScratchImage imageLogo;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_FORCE_RGB, &metadata, imageLogo );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkLogo, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%ls\n", szPath );
        printmeta( &metadata );
        printmetachk( &checkLogo );
        return false;
    }

    // Earth
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"earth_A2B10G10R10.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#if defined(_DEBUG) && defined(VERBOSE)
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    const TexMetadata checkEarth = { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D };

    ScratchImage imageEarth;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, imageEarth );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkEarth, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%ls\n", szPath );
        printmeta( &metadata );
        printmetachk( &checkEarth );
        return false;
    }

    // Copy (same format)
    assert( imageLogo.GetMetadata().format == imageTest.GetMetadata().format );
    for( UINT x=0; x < 16; ++x )
    {
        for( UINT y=0; y < 16; ++y )
        {
            ScratchImage image;
            hr = image.InitializeFromImage( *imageLogo.GetImage(0,0,0) );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed making copy of logo image (HRESULT %08X)\n", hr );
            }
            else
            {
                Rect rct( 0, 0, 32, 32 );
                hr = CopyRectangle( *imageTest.GetImage(0,0,0), rct, *image.GetImage(0,0,0), TEX_FILTER_DEFAULT, x, y );
                if (FAILED(hr))
                {
                    success = false;
                    printe( "Failed doing test->logo offset %d by %d (HRESULT %08X)\n", hr, x, y );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH] = {};
                    swprintf_s( tname, L"%s_%d_%d", fname, x, y );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                }
            }
        }
    }

    // Copy (same format with source offset)
    for( size_t x=0; x < 16; ++x )
    {
        for( size_t y=0; y < 16; ++y )
        {
            ScratchImage image;
            hr = image.InitializeFromImage( *imageLogo.GetImage(0,0,0) );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed making copy of logo image (HRESULT %08X)\n", hr );
            }
            else
            {
                Rect rct( 5, 10, 25, 20 );
                hr = CopyRectangle( *imageTest.GetImage(0,0,0), rct, *image.GetImage(0,0,0), TEX_FILTER_DEFAULT, x, y );
                if (FAILED(hr))
                {
                    success = false;
                    printe( "Failed doing test->logo (%zu,%zu,%zu,%zu) offset %zu by %zu (HRESULT %08X)\n",
                            rct.x, rct.y, rct.w, rct.h, x, y, hr );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH] = {};
                    swprintf_s( tname, L"offset_%s_%zu_%zu", fname, x, y );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                }
            }
        }
    }

    // Copy (with conversion)
    assert( imageLogo.GetMetadata().format != imageEarth.GetMetadata().format );
    for( size_t x=0; x < 16; ++x )
    {
        for( size_t y=0; y < 16; ++y )
        {
            ScratchImage image;
            hr = image.InitializeFromImage( *imageEarth.GetImage(0,0,0) );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed making copy of earth image (HRESULT %08X)\n", hr );
            }
            else
            {
                Rect rct( 0, 0, 200, 200 );
                hr = CopyRectangle( *imageLogo.GetImage(0,0,0), rct, *image.GetImage(0,0,0), TEX_FILTER_DEFAULT, x, y );
                if (FAILED(hr))
                {
                    success = false;
                    printe( "Failed doing logo->earth offset %zu by %zu (HRESULT %08X)\n", x, y, hr );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH] = {};
                    swprintf_s( tname, L"cvt_%s_%zu_%zu", fname, x, y );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                }
            }
        }
    }

    // Copy (with conversion with source offset)
    for( size_t x=0; x < 16; ++x )
    {
        for( size_t y=0; y < 16; ++y )
        {
            ScratchImage image;
            hr = image.InitializeFromImage( *imageEarth.GetImage(0,0,0) );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed making copy of earth image (HRESULT %08X)\n", hr );
            }
            else
            {
                Rect rct( 100, 50, 128, 64 );
                hr = CopyRectangle( *imageLogo.GetImage(0,0,0), rct, *image.GetImage(0,0,0), TEX_FILTER_DEFAULT, x, y );
                if (FAILED(hr))
                {
                    success = false;
                    printe( "Failed doing logo->earth (%zu,%zu,%zu,%zu) offset %zu by %zu (HRESULT %08X)\n",
                            rct.x, rct.y, rct.w, rct.h, x, y, hr );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH] = {};
                    swprintf_s( tname, L"cvt_offset_%s_%zu_%zu", fname, x, y );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                }
            }
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// ComputeMSE
bool TEXTest::Test10()
{
    bool success = true;

    // Load source images (must be same size)

    // Windows Logo
    wchar_t szPath[MAX_PATH] = {};
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"windowslogo_X8R8G8B8.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#if defined(_DEBUG) && defined(VERBOSE)
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageLogo;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, nullptr, imageLogo );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }

    // Windows 95 Logo
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"win95.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#if defined(_DEBUG) && defined(VERBOSE)
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageWin95;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, nullptr, imageWin95 );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }

    // DX logo
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"dx5_logo.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#if defined(_DEBUG) && defined(VERBOSE)
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageDX;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, nullptr, imageDX );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }

    // Alpha Edge
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"alphaedge.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#if defined(_DEBUG) && defined(VERBOSE)
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageAlpha;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, nullptr, imageAlpha );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }

    // Compute MSE between various images

    // First MSE
    float mse, mseV[4];
    hr = ComputeMSE( *imageLogo.GetImage(0,0,0), *imageLogo.GetImage(0,0,0), mse, mseV );
    if ( FAILED(hr) )
    {
        success = false;
        printe( "Failed computing first MSE (HRESULT %08X):\n", hr );
    }
    else if ( !IsEqual( mse, 0.f ) )
    {
        success = false;
        printe( "MSE = %f (%f %f %f %f)... 0.f\n", mse, mseV[0], mseV[1], mseV[2], mseV[3] );
    }

    // Second MSE
    hr = ComputeMSE( *imageLogo.GetImage(0,0,0), *imageWin95.GetImage(0,0,0), mse, mseV );
    if ( FAILED(hr) )
    {
        success = false;
        printe( "Failed computing second MSE (HRESULT %08X):\n", hr );
    }
    else if ( !IsEqual( mse, 0.435924f ) )
    {
        success = false;
        printe( "MSE = %f (%f %f %f %f)... 0.435924\n", mse, mseV[0], mseV[1], mseV[2], mseV[3] );
    }

    // Third MSE
    hr = ComputeMSE( *imageDX.GetImage(0,0,0), *imageAlpha.GetImage(0,0,0), mse, mseV );
    if ( FAILED(hr) )
    {
        success = false;
        printe( "Failed computing third MSE (HRESULT %08X):\n", hr );
    }
    else if ( !IsEqual( mse, 0.728756f ) )
    {
        success = false;
        printe( "MSE = %f (%f %f %f %f)... 0.728756\n", mse, mseV[0], mseV[1], mseV[2], mseV[3] );
    }

    return success;
}


//-------------------------------------------------------------------------------------
// IsAlphaAllOpaque
bool TEXTest::Test14()
{
    bool success = true;

    {
        ScratchImage image;
        HRESULT hr = image.Initialize1D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1);
        if (FAILED(hr))
        {
            success = false;
            printe("Failed creating test image (HRESULT %08X)\n", hr);
        }
        else
        {
            auto img = image.GetImage(0, 0, 0);

            for (unsigned j = 0; j < 256; ++j)
            {
                DWORD pixel = j | (j << 8) | (j << 16) | (j << 24);
                *reinterpret_cast<uint32_t*>(img->pixels) = pixel;

                bool isao = image.IsAlphaAllOpaque();
                if (isao != (j >= 255))
                {
                    success = false;
                    printe("Failed IsAlphaAllOpaque (%u): %s ... %s\n", j, (isao) ? "true" : "false", (j >= 255) ? "true" : "false");
                }
            }
        }
    }

    struct TestMedia
    {
        TexMetadata metadata;
        const wchar_t *fname;
        bool isAlphaAllOpaque;
    };

    static const TestMedia s_TestMedia[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename | bool
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds", true },
        { { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds", true },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds", true },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds", true },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds", true },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds", true },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5.dds", true },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5mip.dds", true },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1.dds", true },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1mip.dds", true },
        { { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds", true },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds", true },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvoldxt1mip.dds", true },
        { { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds", true },
        { { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bc7_unorm.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bc7_unorm_srgb.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_DXT3.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds", true },
        { { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A8R3G3B2.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16f.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba32f.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds", false },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4L4.dds", true },
        { { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X4R4G4B4.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4R4G4B4.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.dds", true },
        { { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lobbycube.dds", true },
        { { 8192, 4096, 1, 1, 14, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"world8192.dds", true },
        { { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds", false },
        { { 304, 268, 1, 1, 9, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S_pmalpha.dds", false },
        { { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.dds", false },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SplashScreen2.dds", false },
        { { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.dds", true },
    };
    
    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(s_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(s_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );

        const TexMetadata* check = &s_TestMedia[index].metadata;
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
            bool isao = image.IsAlphaAllOpaque();
            if ( isao != s_TestMedia[index].isAlphaAllOpaque )
            {
                success = false;
                printe( "Failed IsAlphaAllOpaque: %s ... %s\n%ls\n", (isao) ? "true" : "false", (s_TestMedia[index].isAlphaAllOpaque) ? "true" : "false", szPath );
            }
            else
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// EvaluateImage
bool TEXTest::Test16()
{
    struct TestMedia
    {
        TexMetadata metadata;
        const wchar_t *fname;
        float maxLuminance;
        float maxLuminanceC;
    };

    static const TestMedia s_TestMedia[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename | float
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds", 1.f, 1.f },
        { { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds", 0.931935f, 0.f },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds", 0.11f, 0.f },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds", 0.11f, 0.11f },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds", 0.11f, 0.962863f },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds", 0.11f, 0.962863f },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5.dds", 0.11f, 0.985806f },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5mip.dds", 0.11f, 0.985806f },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1.dds", 0.11f, 0.f },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1mip.dds", 0.11f, 0.11f },
        { { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds", 0.11f, 0.886275f },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds", 0.11f, 0.886275f },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvoldxt1mip.dds", 0.11f, 0.936088f },
        { { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds", 0.984360f, 0.984360f },
        { { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds", 0.984360f, 0.984360f },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bc7_unorm.dds", 1.f, 1.f },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bc7_unorm_srgb.dds", 1.f, 1.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_DXT3.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A8R3G3B2.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16f.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba32f.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds", 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4L4.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X4R4G4B4.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4R4G4B4.dds", 1.f, 0.f },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.dds", 0.968588f, 0.968588f },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.dds", 1.f, 1.f },
        { { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lobbycube.dds", 0.996452f, 1.f },
        { { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds", 0.731412f, 0.731412f },
        { { 304, 268, 1, 1, 9, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S_pmalpha.dds", 0.731412f, 0.731412f },
        { { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.dds", 0.970588f, 0.970588f },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SplashScreen2.dds", 0.910078f, 0.f },
        { { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.dds", 0.933569f, 0.f },
    };

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(s_TestMedia); ++index)
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(s_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, image);

        const TexMetadata* check = &s_TestMedia[index].metadata;
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

            XMVECTOR maxLum = XMVectorZero();

            hr = EvaluateImage(*image.GetImage(0, 0, 0), [&](const XMVECTOR* pixels, size_t width, size_t y)
            {
                UNREFERENCED_PARAMETER(y);

                assert(y <= metadata.height);

                for (size_t j = 0; j < width; ++j)
                {
                    static const XMVECTORF32 s_luminance = { 0.3f, 0.59f, 0.11f, 0.f };
                    static const XMVECTORF32 s_nitsNormalize = { 100.f, 100.f, 100.f, 1.f };

                    XMVECTOR v = *pixels++;

                    v = XMVectorDivide(v, s_nitsNormalize);
                    v = XMVectorSaturate(v);
                    v = XMVector3Dot(v, s_luminance);

                    maxLum = XMVectorMax(v, maxLum);
                }
            });
            if (FAILED(hr))
            {
                success = false;
                pass = false;
                printe("ERROR: EvaluateImage failed (%08X)\n%ls\n", hr, szPath);
            }
            else
            {
                float maxLuminance = XMVectorGetX(maxLum);
                maxLuminance *= 100.f;

                if (!IsEqual(maxLuminance, s_TestMedia[index].maxLuminance))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: EvaluateImage result unexpected %f...%f\n%ls\n", maxLuminance, s_TestMedia[index].maxLuminance, szPath);
                }
            }

            //--- Complex transform -----------------------------------------------
            if (image.GetImageCount() > 1)
            {
                maxLum = XMVectorZero();

                hr = EvaluateImage(image.GetImages(), image.GetImageCount(), metadata, [&](const XMVECTOR* pixels, size_t width, size_t y)
                {
                    UNREFERENCED_PARAMETER(y);

                    assert(y <= metadata.height);

                    for (size_t j = 0; j < width; ++j)
                    {
                        static const XMVECTORF32 s_luminance = { 0.3f, 0.59f, 0.11f, 0.f };
                        static const XMVECTORF32 s_nitsNormalize = { 100.f, 100.f, 100.f, 1.f };

                        XMVECTOR v = *pixels++;

                        v = XMVectorDivide(v, s_nitsNormalize);
                        v = XMVectorSaturate(v);
                        v = XMVector3Dot(v, s_luminance);

                        maxLum = XMVectorMax(v, maxLum);
                    }
                });
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: EvaluateImage [complex] failed (%08X)\n%ls\n", hr, szPath);
                }
                else
                {
                    float maxLuminance = XMVectorGetX(maxLum);
                    maxLuminance *= 100.f;

                    if (!IsEqual(maxLuminance, s_TestMedia[index].maxLuminanceC))
                    {
                        success = false;
                        pass = false;
                        printe("ERROR: EvaluateImage [complex] result unexpected %f...%f\n%ls\n", maxLuminance, s_TestMedia[index].maxLuminanceC, szPath);
                    }
                }
            }

            if (pass)
                ++npass;

            ++ncount;
        }
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    return success;
}


//-------------------------------------------------------------------------------------
// TransformImage

namespace
{
    void xform_copy(_Out_writes_(width) XMVECTOR* outPixels, _In_reads_(width) const XMVECTOR* inPixels, size_t width, size_t y)
    {
        UNREFERENCED_PARAMETER(y);

        assert(inPixels != nullptr);
        assert(outPixels != nullptr);
        assert(width > 0);

        for (size_t j = 0; j < width; ++j)
        {
            outPixels[j] = inPixels[j];
        }
    }

    void xform_inverse(_Out_writes_(width) XMVECTOR* outPixels, _In_reads_(width) const XMVECTOR* inPixels, size_t width, size_t y)
    {
        UNREFERENCED_PARAMETER(y);

        assert(inPixels != nullptr);
        assert(outPixels != nullptr);
        assert(width > 0);

        for (size_t j = 0; j < width; ++j)
        {
            XMVECTOR orig = inPixels[j];
            XMVECTOR value = XMVectorSaturate(orig);
            value = XMVectorSubtract(g_XMOne, value);
            value = XMVectorSelect(orig, value, g_XMSelect1110); // Preserve original alpha channel
            outPixels[j] = value;
        }
    }
}
bool TEXTest::Test17()
{
    struct TestMedia
    {
        TexMetadata metadata;
        const wchar_t *fname;
        uint8_t copy_md5[16];
        uint8_t inverse_md5[16];
        uint8_t chromakey_md5[16];
    };

    static const TestMedia s_TestMedia[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
            // copy_md5
            // inverse_md5
            // chromakey_md5
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds",
            { 0x9d,0xf2,0x92,0xf3,0x17,0x9f,0x43,0xad,0x47,0xbe,0x57,0x0c,0x10,0x14,0x6d,0xa7 },
            { 0x74,0x67,0xf6,0x3c,0x61,0x8c,0xc7,0x98,0x6f,0x4a,0x7e,0x92,0x0b,0x03,0x0c,0x4c },
            { 0xea,0x69,0x1a,0xca,0xce,0x98,0x0b,0x7f,0x3a,0xb4,0x60,0x39,0x41,0xe0,0x1c,0x90 } },
        { { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds",
            { 0x00,0x5c,0x5d,0x42,0x6c,0xed,0x64,0xc4,0xfd,0x2d,0xc5,0x67,0x89,0x2f,0x34,0x8c },
            { 0x42,0x84,0xfb,0x2c,0xf4,0x9c,0x4e,0x45,0x63,0x1b,0xd9,0x1e,0xa5,0x01,0x0c,0x38 },
            { 0x00,0x5c,0x5d,0x42,0x6c,0xed,0x64,0xc4,0xfd,0x2d,0xc5,0x67,0x89,0x2f,0x34,0x8c } /*same as copy*/},
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds",
            { 0x38,0x9b,0x4e,0x56,0x4f,0xd1,0x5f,0x0d,0x63,0x05,0x44,0x35,0xba,0x66,0xdd,0x9b },
            { 0x63,0x3b,0xa4,0x71,0x89,0xdd,0xc6,0x22,0xc1,0xb6,0xe6,0xb5,0x00,0x1d,0x92,0x74 },
            { 0x38,0x9b,0x4e,0x56,0x4f,0xd1,0x5f,0x0d,0x63,0x05,0x44,0x35,0xba,0x66,0xdd,0x9b /*same as copy*/ } },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds",
            { 0x4c,0xe7,0x4a,0xd7,0x0f,0xee,0xfb,0x6e,0x5b,0x27,0xd1,0x5b,0x45,0xb9,0xd5,0x5a },
            { 0x93,0xde,0x1e,0x20,0x91,0xc6,0x46,0x8f,0x2b,0x96,0x1f,0xf4,0xef,0xac,0x23,0xbd },
            { 0x4c,0xe7,0x4a,0xd7,0x0f,0xee,0xfb,0x6e,0x5b,0x27,0xd1,0x5b,0x45,0xb9,0xd5,0x5a /*same as copy*/ } },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds",
            { 0x38,0x39,0x63,0x45,0x9a,0x28,0x85,0x96,0xcd,0x57,0x3e,0x38,0x60,0xcc,0xdc,0x99 },
            { 0x4c,0xbb,0x2d,0xdd,0xe9,0x8b,0xf6,0x53,0x38,0x11,0xb1,0xfa,0x1b,0x4f,0x67,0x04 },
            { 0x38,0x39,0x63,0x45,0x9a,0x28,0x85,0x96,0xcd,0x57,0x3e,0x38,0x60,0xcc,0xdc,0x99 /*same as copy*/ } },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds",
            { 0xa6,0xd9,0xb8,0xcd,0x81,0x3c,0x30,0xd6,0xcd,0x57,0x81,0xd0,0x4a,0x87,0xdf,0x9f },
            { 0x70,0x0a,0xb9,0x4f,0xb8,0x2f,0x20,0x60,0x87,0x33,0x6f,0xed,0xfc,0x3b,0x8a,0x29 },
            { 0xa6,0xd9,0xb8,0xcd,0x81,0x3c,0x30,0xd6,0xcd,0x57,0x81,0xd0,0x4a,0x87,0xdf,0x9f /*same as copy*/ } },
        { { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds",
            { 0x4e,0xa9,0xb0,0x2b,0xfc,0x5f,0xbd,0x8a,0x0e,0x43,0x31,0x93,0xf5,0x82,0x19,0x87 },
            { 0xc2,0xd6,0x9d,0xa4,0xce,0xea,0x50,0x38,0xb0,0xe8,0x76,0x38,0x0f,0xb2,0x3c,0x90 },
            { 0x4e,0xa9,0xb0,0x2b,0xfc,0x5f,0xbd,0x8a,0x0e,0x43,0x31,0x93,0xf5,0x82,0x19,0x87 /*same as copy*/ } },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds",
            { 0x4e,0x65,0xb5,0xc5,0x85,0x1f,0x1a,0x2e,0x2f,0x29,0xf2,0x9e,0x48,0x20,0xb7,0x87 },
            { 0x5c,0xc8,0x81,0x96,0x6d,0x7a,0xc2,0x7a,0x25,0x8a,0xfd,0xbb,0x8b,0xb4,0x9c,0x20 },
            { 0x4e,0x65,0xb5,0xc5,0x85,0x1f,0x1a,0x2e,0x2f,0x29,0xf2,0x9e,0x48,0x20,0xb7,0x87 /*same as copy*/ } },
    };

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(s_TestMedia); ++index)
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(s_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, srcimage);

        const TexMetadata* check = &s_TestMedia[index].metadata;
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

            // Verify the image data
            uint8_t digest[16];
            hr = MD5Checksum(srcimage, digest);
            if (FAILED(hr))
            {
                success = false;
                pass = false;
                printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath);
            }
            else if (memcmp(digest, s_TestMedia[index].copy_md5, 16) != 0)
            {
                success = false;
                pass = false;
                printe("Failed comparing image MD5 checksum:\n%ls\n", szPath);
                printdigest("computed", digest);
                printdigest("expected", s_TestMedia[index].copy_md5);
            }

            // Identity transform
            {
                ScratchImage image;
                hr = TransformImage(*srcimage.GetImage(0, 0, 0), xform_copy, image);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: TransformImage (copy) failed (%08X)\n%ls\n", hr, szPath);
                }
                else
                {
                    // TESTTEST-SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image);

                    if (srcimage.GetImageCount() == 1)
                    {
                        hr = MD5Checksum(image, digest);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath);
                        }
                        else if (memcmp(digest, s_TestMedia[index].copy_md5, 16) != 0)
                        {
                            success = false;
                            pass = false;
                            printe("Failed comparing image MD5 checksum (copy):\n%ls\n", szPath);
                            printdigest("computed", digest);
                            printdigest("expected", s_TestMedia[index].copy_md5);
                        }
                    }
                }
            }

            // Inverse RGB
            {
                ScratchImage image;
                hr = TransformImage(*srcimage.GetImage(0, 0, 0), xform_inverse, image);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: TransformImage (inverse) failed (%08X)\n%ls\n", hr, szPath);
                }
                else
                {
                    // TESTTEST-SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image);

                    if (srcimage.GetImageCount() == 1)
                    {
                        hr = MD5Checksum(image, digest);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath);
                        }
                        else if (memcmp(digest, s_TestMedia[index].inverse_md5, 16) != 0)
                        {
                            success = false;
                            pass = false;
                            printe("Failed comparing image MD5 checksum (inverse):\n%ls\n", szPath);
                            printdigest("computed", digest);
                            printdigest("expected", s_TestMedia[index].inverse_md5);
                        }
                    }
                }
            }

            // Chroma-key
            {
                ScratchImage image;
                hr = TransformImage(*srcimage.GetImage(0, 0, 0), [&](XMVECTOR* outPixels, const XMVECTOR* inPixels, size_t width, size_t y)
                {
                    static const XMVECTORF32 s_chromaKey = { 0.f, 1.f, 0.f, 0.f };
                    static const XMVECTORF32 s_tolerance = { 0.2f, 0.2f, 0.2f, 0.f };
                    
                    UNREFERENCED_PARAMETER(y);

                    assert(y <= metadata.height);

                    for (size_t j = 0; j < width; ++j)
                    {
                        XMVECTOR value = inPixels[j];

                        // For 'inverse alpha' swap the cases below
                        if (XMVector3NearEqual(value, s_chromaKey, s_tolerance))
                        {
                            value = g_XMZero;
                        }
                        else
                        {
                            value = XMVectorSelect(g_XMOne, value, g_XMSelect1110);
                        }

                        outPixels[j] = value;
                    }
                }, image);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: TransformImage (colorkey) failed (%08X)\n%ls\n", hr, szPath);
                }
                else
                {
                    // TESTTEST-SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image);

                    if (srcimage.GetImageCount() == 1)
                    {
                        hr = MD5Checksum(image, digest);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath);
                        }
                        else if (memcmp(digest, s_TestMedia[index].chromakey_md5, 16) != 0)
                        {
                            success = false;
                            pass = false;
                            printe("Failed comparing image MD5 checksum (colorkey):\n%ls\n", szPath);
                            printdigest("computed", digest);
                            printdigest("expected", s_TestMedia[index].chromakey_md5);
                        }
                    }
                }
            }

            //--- Complex transform -----------------------------------------------
            if (srcimage.GetImageCount() > 1)
            {
                // Identity transform
                {
                    ScratchImage imageComplex;
                    hr = TransformImage(srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), xform_copy, imageComplex);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("ERROR: TransformImage (copy complex) failed (%08X)\n%ls\n", hr, szPath);
                    }
                    else
                    {
                        // TESTTEST-SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, imageComplex);

                        hr = MD5Checksum(imageComplex, digest);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath);
                        }
                        else if (memcmp(digest, s_TestMedia[index].copy_md5, 16) != 0)
                        {
                            success = false;
                            pass = false;
                            printe("Failed comparing image MD5 checksum (copy complex):\n%ls\n", szPath);
                            printdigest("computed", digest);
                            printdigest("expected", s_TestMedia[index].copy_md5);
                        }
                    }
                }

                // Inverse RGB
                {
                    ScratchImage imageComplex;
                    hr = TransformImage(srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), xform_inverse, imageComplex);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("ERROR: TransformImage (inverse complex) failed (%08X)\n%ls\n", hr, szPath);
                    }
                    else
                    {
                        // TESTTEST-SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, imageComplex);

                        hr = MD5Checksum(imageComplex, digest);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath);
                        }
                        else if (memcmp(digest, s_TestMedia[index].inverse_md5, 16) != 0)
                        {
                            success = false;
                            pass = false;
                            printe("Failed comparing image MD5 checksum (inverse complex):\n%ls\n", szPath);
                            printdigest("computed", digest);
                            printdigest("expected", s_TestMedia[index].inverse_md5);
                        }
                    }
                }

                // Chroma-key
                {
                    ScratchImage imageComplex;
                    hr = TransformImage(srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(),
                        [&](XMVECTOR* outPixels, const XMVECTOR* inPixels, size_t width, size_t y)
                    {
                        static const XMVECTORF32 s_chromaKey = { 0.f, 1.f, 0.f, 0.f };
                        static const XMVECTORF32 s_tolerance = { 0.2f, 0.2f, 0.2f, 0.f };

                        UNREFERENCED_PARAMETER(y);

                        assert(y <= metadata.height);

                        for (size_t j = 0; j < width; ++j)
                        {
                            XMVECTOR value = inPixels[j];

                            // For 'inverse alpha' swap the cases below
                            if (XMVector3NearEqual(value, s_chromaKey, s_tolerance))
                            {
                                value = g_XMZero;
                            }
                            else
                            {
                                value = XMVectorSelect(g_XMOne, value, g_XMSelect1110);
                            }

                            outPixels[j] = value;
                        }
                    }, imageComplex);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("ERROR: TransformImage (colorkey complex) failed (%08X)\n%ls\n", hr, szPath);
                    }
                    else
                    {
                        // TESTTEST-SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, imageComplex);

                        hr = MD5Checksum(imageComplex, digest);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath);
                        }
                        else if (memcmp(digest, s_TestMedia[index].chromakey_md5, 16) != 0)
                        {
                            success = false;
                            pass = false;
                            printe("Failed comparing image MD5 checksum (colorkey complex):\n%ls\n", szPath);
                            printdigest("computed", digest);
                            printdigest("expected", s_TestMedia[index].chromakey_md5);
                        }
                    }
                }
            }

            if (pass)
                ++npass;

            ++ncount;
        }
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    return success;
}
