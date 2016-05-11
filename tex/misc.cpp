//-------------------------------------------------------------------------------------
// misc.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------

inline bool IsEqual( float f1, float f2 )
{
    return ( fabs(f1 - f2) < 0.000001 ) != 0;
}

//-------------------------------------------------------------------------------------
// CopyRectangle
bool Test09()
{
    bool success = true;

    // Test image
    wchar_t szPath[MAX_PATH];
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"test8888.dds", szPath, MAX_PATH);
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
    ret = ExpandEnvironmentStringsW(TEMP_PATH L"cpy", tempDir, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

    CreateDirectoryW( tempDir, NULL );

    const TexMetadata checkTest = { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    TexMetadata metadata;
    ScratchImage imageTest;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_FORCE_RGB, &metadata, imageTest );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkTest, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%S\n", szPath );
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

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    const TexMetadata checkLogo = { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    ScratchImage imageLogo;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_FORCE_RGB, &metadata, imageLogo );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkLogo, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%S\n", szPath );
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

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    const TexMetadata checkEarth = { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D };

    ScratchImage imageEarth;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, imageEarth );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkEarth, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%S\n", szPath );
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

                    wchar_t tname[MAX_PATH];
                    swprintf_s( tname, L"%s_%d_%d", fname, x, y );

                    wchar_t szDestPath[MAX_PATH];
                    _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

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
                    printe( "Failed doing test->logo (%Iu,%Iu,%Iu,%Iu) offset %Iu by %Iu (HRESULT %08X)\n",
                            rct.x, rct.y, rct.w, rct.h, x, y, hr );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH];
                    swprintf_s( tname, L"offset_%s_%Iu_%Iu", fname, x, y );

                    wchar_t szDestPath[MAX_PATH];
                    _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

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
                    printe( "Failed doing logo->earth offset %Iu by %Iu (HRESULT %08X)\n", x, y, hr );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH];
                    swprintf_s( tname, L"cvt_%s_%Iu_%Iu", fname, x, y );

                    wchar_t szDestPath[MAX_PATH];
                    _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

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
                    printe( "Failed doing logo->earth (%Iu,%Iu,%Iu,%Iu) offset %Iu by %Iu (HRESULT %08X)\n",
                            rct.x, rct.y, rct.w, rct.h, x, y, hr );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH];
                    swprintf_s( tname, L"cvt_offset_%s_%Iu_%Iu", fname, x, y );

                    wchar_t szDestPath[MAX_PATH];
                    _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                }
            }
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// ComputeMSE
bool Test10()
{
    bool success = true;

    // Load source images (must be same size)

    // Windows Logo
    wchar_t szPath[MAX_PATH];
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"windowslogo_X8R8G8B8.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageLogo;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, NULL, imageLogo );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
        return false;
    }

    // Windows 95 Logo
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"win95.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageWin95;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, NULL, imageWin95 );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
        return false;
    }

    // DX logo
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"dx5_logo.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageDX;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, NULL, imageDX );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
        return false;
    }

    // Alpha Edge
    ret = ExpandEnvironmentStringsW(MEDIA_PATH L"alphaedge.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    ScratchImage imageAlpha;
    hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, NULL, imageAlpha );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
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
bool Test14()
{
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
        { { 200, 150, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds", true },
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
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A8R3G3B2.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16f.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba32f.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds", false },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4L4.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X4R4G4B4.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4R4G4B4.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.dds", true },
        { { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lobbycube.dds", true },
        { { 8192, 4096, 1, 1, 14, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"world8192.dds", true },
        { { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds", false },
        { { 304, 268, 1, 1, 9, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S_pmalpha.dds", false },
        { { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.dds", false },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SplashScreen2.dds", false },
        { { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.dds", true },
    };

    bool success = true;
    
    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(s_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(s_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
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
            printe( "Failed getting data from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in:\n%S\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool isao = image.IsAlphaAllOpaque();
            if ( isao != s_TestMedia[index].isAlphaAllOpaque )
            {
                success = false;
                printe( "Failed IsAlphaAllOpaque: %s ... %s\n%S\n", (isao) ? "true" : "false", (s_TestMedia[index].isAlphaAllOpaque) ? "true" : "false", szPath );
            }
            else
                ++npass;
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}