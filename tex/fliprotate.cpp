//-------------------------------------------------------------------------------------
// fliprotate.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

struct FlipRotateMedia
{
    DWORD flags;
    TexMetadata metadata;
    const wchar_t *fname;
};

static const FlipRotateMedia g_FlipRotateMedia[] = 
{

// DWORD flags | <source> width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
{ TEX_FR_ROTATE90, { 200, 150, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds" },
{ TEX_FR_ROTATE90, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds" },
{ TEX_FR_ROTATE90, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds" },
{ TEX_FR_ROTATE90, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds" },
{ TEX_FR_ROTATE90, { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds" },
{ TEX_FR_ROTATE90, { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds" },
{ TEX_FR_ROTATE90, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
{ TEX_FR_ROTATE90, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgb565.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_L8.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_L16.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_r16f.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_r32f.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16f.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba32f.dds" },
{ TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds" },
{ TEX_FR_ROTATE90, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-BlackWhite.dds" },
{ TEX_FR_ROTATE90, { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testpatternYUY2.dds" },
{ TEX_FR_ROTATE90, { 200, 200, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaCubeYUY2.dds" },
{ TEX_FR_ROTATE90, { 200, 200, 4, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"lenaVolYUY2.dds" },
};

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// FlipRotate
bool Test04()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_FlipRotateMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_FlipRotateMedia[index].fname, szPath, MAX_PATH);
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
        WCHAR ext[_MAX_EXT];
        WCHAR fname[_MAX_FNAME];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        WCHAR tempDir[MAX_PATH];
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"flip", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, NULL );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );

        const TexMetadata* check = &g_FlipRotateMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting DDS data from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in DDS:\n%S\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;
            
            DWORD dwFlags = g_FlipRotateMedia[index].flags;

            //--- Simple flip/rotate --------------------------------------------------
            ScratchImage image;
            hr = FlipRotate( *srcimage.GetImage(0,0,0), dwFlags, image );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed flip/rotating (HRESULT %08X) using %08X:\n%S\n", hr, dwFlags, szPath );
            }
            else
            {
                if ( (dwFlags & (TEX_FR_ROTATE90|TEX_FR_ROTATE270))
                      && ( image.GetMetadata().width != srcimage.GetMetadata().height
                           || image.GetMetadata().height != srcimage.GetMetadata().width ) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed flip/rotating result is %Iu x %Iu:\n", 
                            image.GetMetadata().width, image.GetMetadata().height );
                    printe( "\n...(check) %Iu x %Iu:\n%S\n", srcimage.GetMetadata().height, srcimage.GetMetadata().width, szPath );
                }
                else if ( !(dwFlags & (TEX_FR_ROTATE90|TEX_FR_ROTATE270))
                          && ( image.GetMetadata().width != srcimage.GetMetadata().width
                               || image.GetMetadata().height != srcimage.GetMetadata().height ) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed flip/rotating result is %Iu x %Iu:\n",
                            image.GetMetadata().width, image.GetMetadata().height );
                    printe( "\n...(check) %Iu x %Iu:\n%S\n", srcimage.GetMetadata().width, srcimage.GetMetadata().height, szPath );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    WCHAR szDestPath[MAX_PATH];
                    _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, fname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                }
            }

            //--- Complex flip/rotate -------------------------------------------------
            if ( srcimage.GetImageCount() > 1 )
            {
                ScratchImage imageComplex;
                hr = FlipRotate( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), dwFlags, imageComplex );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed flip/rotating complex (HRESULT %08X) using %08X:\n%S\n", hr, dwFlags, szPath );
                }
                else
                {
                    const TexMetadata& metadata2 = imageComplex.GetMetadata();

                    if ( (dwFlags & (TEX_FR_ROTATE90|TEX_FR_ROTATE270))
                          && ( metadata2.width != srcimage.GetMetadata().height
                               || metadata2.height != srcimage.GetMetadata().width ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed flip/rotating complex result is %Iu x %Iu:\n", 
                                metadata2.width, metadata2.height );
                        printe( "\n...(check) %Iu x %Iu:\n%S\n", srcimage.GetMetadata().height, srcimage.GetMetadata().width, szPath );
                    }
                    else if ( !(dwFlags & (TEX_FR_ROTATE90|TEX_FR_ROTATE270))
                              && ( metadata2.width != srcimage.GetMetadata().width
                                   || metadata2.height != srcimage.GetMetadata().height ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed flip/rotating complex result is %Iu x %Iu:\n",
                                metadata2.width, metadata2.height );
                        printe( "\n...(check) %Iu x %Iu:\n%S\n", srcimage.GetMetadata().width, srcimage.GetMetadata().height, szPath );
                    }
                    else if ( metadata2.mipLevels != check->mipLevels
                              || metadata2.depth != check->depth
                              || metadata2.arraySize != check->arraySize
                              || metadata2.miscFlags != check->miscFlags
                              || metadata2.format != check->format
                              || metadata2.dimension != check->dimension )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed flip/rotating complex result:\n%S\n", szPath );
                        printmeta( &metadata2 );
                        printmetachk( check );
                    }
                    else
                    {
                        // TODO - Verify the image data (perhaps MD5 checksum)

                        WCHAR tname[MAX_PATH] = { 0 };
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_complex" );

                        WCHAR szDestPath[MAX_PATH];
                        _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

                        SaveToDDSFile( imageComplex.GetImages(), imageComplex.GetImageCount(), imageComplex.GetMetadata(), DDS_FLAGS_NONE, szDestPath );
                    }
                }
            }

            if ( pass )
                ++npass;            
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}
