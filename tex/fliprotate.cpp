//-------------------------------------------------------------------------------------
// fliprotate.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "tex.h"

#include "DirectXTex.h"

using namespace DirectX;

namespace
{
    struct FlipRotateMedia
    {
        TEX_FR_FLAGS flags;
        TexMetadata metadata;
        const wchar_t *fname;
    };

    const FlipRotateMedia g_FlipRotateMedia[] =
    {
        // DWORD flags | <source> width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { TEX_FR_ROTATE90, { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds" },
        { TEX_FR_ROTATE90, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds" },
        { TEX_FR_ROTATE90, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds" },
        { TEX_FR_ROTATE90, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds" },
        { TEX_FR_ROTATE90, { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds" },
        { TEX_FR_ROTATE90, { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds" },

        { TEX_FR_ROTATE90, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE90, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds" },

        { TEX_FR_ROTATE180, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE270, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds" },

        { TEX_FR_FLIP_HORIZONTAL,  { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_FLIP_HORIZONTAL
          | TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },

        { TEX_FR_ROTATE90
          | TEX_FR_FLIP_HORIZONTAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE90
          | TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE90
          | TEX_FR_FLIP_HORIZONTAL
          | TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },

        { TEX_FR_ROTATE180
          | TEX_FR_FLIP_HORIZONTAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE180
          | TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE180
          | TEX_FR_FLIP_HORIZONTAL
          | TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },

        { TEX_FR_ROTATE270
          | TEX_FR_FLIP_HORIZONTAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE270
          | TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { TEX_FR_ROTATE270
          | TEX_FR_FLIP_HORIZONTAL
          | TEX_FR_FLIP_VERTICAL, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },

        { TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds" },
        { TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds" },
        { TEX_FR_ROTATE90, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
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

        #ifdef _M_X64
        { TEX_FR_ROTATE90, { 16384, 16384, 1, 1, 15, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k.dds" },
        { TEX_FR_ROTATE90, { 16384, 16384, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_SNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k_snorm.dds" },
        #endif
    };
}

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// FlipRotate
bool TEXTest::Test04()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_FlipRotateMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_FlipRotateMedia[index].fname, szPath, MAX_PATH);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"flip", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );

        const TexMetadata* check = &g_FlipRotateMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting DDS data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in DDS:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;
            
            TEX_FR_FLAGS dwFlags = g_FlipRotateMedia[index].flags;

            int rotateMode = static_cast<int>(dwFlags & (TEX_FR_ROTATE0 | TEX_FR_ROTATE90 | TEX_FR_ROTATE180 | TEX_FR_ROTATE270));

            //--- Simple flip/rotate --------------------------------------------------
            ScratchImage image;
            hr = FlipRotate( *srcimage.GetImage(0,0,0), dwFlags, image );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed flip/rotating (HRESULT %08X) using %08lX:\n%ls\n", static_cast<unsigned int>(hr), dwFlags, szPath );
            }
            else
            {
                if ( ((rotateMode == TEX_FR_ROTATE90) || (rotateMode == TEX_FR_ROTATE270))
                      && ( image.GetMetadata().width != srcimage.GetMetadata().height
                           || image.GetMetadata().height != srcimage.GetMetadata().width ) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed flip/rotating result is %zu x %zu:\n", 
                            image.GetMetadata().width, image.GetMetadata().height );
                    printe( "\n...(check) %zu x %zu:\n%ls\n", srcimage.GetMetadata().height, srcimage.GetMetadata().width, szPath );
                }
                else if ( ((rotateMode == TEX_FR_ROTATE0) || (rotateMode == TEX_FR_ROTATE180))
                          && ( image.GetMetadata().width != srcimage.GetMetadata().width
                               || image.GetMetadata().height != srcimage.GetMetadata().height ) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed flip/rotating result is %zu x %zu:\n",
                            image.GetMetadata().width, image.GetMetadata().height );
                    printe( "\n...(check) %zu x %zu:\n%ls\n", srcimage.GetMetadata().width, srcimage.GetMetadata().height, szPath );
                }
                else
                {
                    // TODO - Verify the image data (perhaps MD5 checksum)

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s(tname, fname);

                    switch (rotateMode)
                    {
                    case TEX_FR_ROTATE0: wcscat_s(tname, L"_0"); break;
                    case TEX_FR_ROTATE90: wcscat_s(tname, L"_90"); break;
                    case TEX_FR_ROTATE180: wcscat_s(tname, L"_180"); break;
                    case TEX_FR_ROTATE270: wcscat_s(tname, L"_270"); break;
                    }

                    if (dwFlags & TEX_FR_FLIP_HORIZONTAL)
                        wcscat_s(tname, L"_hf");

                    if (dwFlags & TEX_FR_FLIP_VERTICAL)
                        wcscat_s(tname, L"_vf");

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

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
                    printe( "Failed flip/rotating complex (HRESULT %08X) using %08lX:\n%ls\n", static_cast<unsigned int>(hr), dwFlags, szPath );
                }
                else
                {
                    const TexMetadata& metadata2 = imageComplex.GetMetadata();

                    if ( ((rotateMode == TEX_FR_ROTATE90) || (rotateMode == TEX_FR_ROTATE270))
                          && ( metadata2.width != srcimage.GetMetadata().height
                               || metadata2.height != srcimage.GetMetadata().width ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed flip/rotating complex result is %zu x %zu:\n", 
                                metadata2.width, metadata2.height );
                        printe( "\n...(check) %zu x %zu:\n%ls\n", srcimage.GetMetadata().height, srcimage.GetMetadata().width, szPath );
                    }
                    else if ( ((rotateMode == TEX_FR_ROTATE0) || (rotateMode == TEX_FR_ROTATE180))
                              && ( metadata2.width != srcimage.GetMetadata().width
                                   || metadata2.height != srcimage.GetMetadata().height ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed flip/rotating complex result is %zu x %zu:\n",
                                metadata2.width, metadata2.height );
                        printe( "\n...(check) %zu x %zu:\n%ls\n", srcimage.GetMetadata().width, srcimage.GetMetadata().height, szPath );
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
                        printe( "Failed flip/rotating complex result:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                        printmetachk( check );
                    }
                    else
                    {
                        // TODO - Verify the image data (perhaps MD5 checksum)

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_complex" );

                        switch (rotateMode)
                        {
                        case TEX_FR_ROTATE0: wcscat_s(tname, L"_0"); break;
                        case TEX_FR_ROTATE90: wcscat_s(tname, L"_90"); break;
                        case TEX_FR_ROTATE180: wcscat_s(tname, L"_180"); break;
                        case TEX_FR_ROTATE270: wcscat_s(tname, L"_270"); break;
                        }

                        if (dwFlags & TEX_FR_FLIP_HORIZONTAL)
                            wcscat_s(tname, L"_hf");

                        if (dwFlags & TEX_FR_FLIP_VERTICAL)
                            wcscat_s(tname, L"_vf");

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveToDDSFile( imageComplex.GetImages(), imageComplex.GetImageCount(), imageComplex.GetMetadata(), DDS_FLAGS_NONE, szDestPath );
                    }
                }
            }

            if ( pass )
                ++npass;            
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}
