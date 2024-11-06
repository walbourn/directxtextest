//-------------------------------------------------------------------------------------
// planar.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "tex.h"

#include "DirectXTexP.h"

#include "getname.h"

using namespace DirectX;

namespace
{
    struct TestMedia
    {
        DXGI_FORMAT tformat;
        TexMetadata metadata;
        const wchar_t *fname;
    };

    const TestMedia g_TestMedia[] =
    {
        //  resultFormat | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { DXGI_FORMAT_YUY2, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv12.dds" },
        { DXGI_FORMAT_YUY2, { 3264, 2448, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"imagenv12.dds" },
        { DXGI_FORMAT_Y210, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P010, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP010.dds" },
        { DXGI_FORMAT_Y216, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P016, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP016.dds" },
        { DXGI_FORMAT_YUY2, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV11, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv11.dds" },
    };
}

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// ConvertToSinglePlane
bool TEXTest::Test15()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"plane", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );

        const TexMetadata* check = &g_TestMedia[index].metadata;
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

            //--- Simple convert --------------------------------------------------
            ScratchImage image;
            hr = ConvertToSinglePlane( *srcimage.GetImage(0,0,0), image );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed single plane conversion (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else if ( image.GetMetadata().format != g_TestMedia[index].tformat
                      || image.GetMetadata().width != srcimage.GetMetadata().width || image.GetMetadata().height != srcimage.GetMetadata().height )
            {
                success = false;
                pass = false;
                printe( "Failed conversion result is %zu x %zu (format %ls):\n",
                        image.GetMetadata().width, image.GetMetadata().height, GetName( image.GetMetadata().format ) );
                printe( "\n...(check) %zu x %zu (format %ls):\n%ls\n",
                        srcimage.GetMetadata().width, srcimage.GetMetadata().height, GetName( g_TestMedia[index].tformat ), szPath );
            }
            else
            {
                // TODO - Verify the image data (perhaps MD5 checksum)

                wchar_t tname[MAX_PATH] = {};
                wcscpy_s( tname, fname );
                wcscat_s( tname, L"_" );
                wcscat_s( tname, GetName(g_TestMedia[index].tformat) );

                wchar_t szDestPath[MAX_PATH] = {};
                _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );

                if ( g_TestMedia[index].tformat != DXGI_FORMAT_YUY2 )
                {
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_DXGI_FORMAT_YUY2" );

                    ScratchImage yuy2;
                    hr = Convert( *image.GetImage(0,0,0), DXGI_FORMAT_YUY2, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, yuy2 );
                    if ( SUCCEEDED(hr) )
                    {
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );
                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, yuy2 );
                    }
                }
            }

            //--- Complex convert -------------------------------------------------
            if ( srcimage.GetImageCount() > 1 )
            {
                // TODO -
            }

            if ( pass )
                ++npass;

            ++ncount;
        }
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        ScratchImage image;
        Image nullin = {};
        nullin.width = nullin.height = 256;
        nullin.format = DXGI_FORMAT_NV11;
        HRESULT hr = ConvertToSinglePlane(nullin, image);
        if (hr != E_INVALIDARG && hr != E_POINTER)
        {
            success = false;
            printe("Failed invalid arg test\n");
        }

        nullin.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        hr = ConvertToSinglePlane(nullin, image);
        if (hr != E_INVALIDARG && hr != E_POINTER)
        {
            success = false;
            printe("Failed invalid format arg test\n");
        }

        TexMetadata metadata = {};
        metadata.width = metadata.height = 256;
        metadata.format = DXGI_FORMAT_NV11;
        metadata.depth = metadata.arraySize = metadata.mipLevels = 1;
        metadata.dimension = TEX_DIMENSION_TEXTURE2D;
        hr = ConvertToSinglePlane(nullptr, 0,  metadata, image);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid arg complex test\n");
        }

        metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        hr = ConvertToSinglePlane(nullptr, 0, metadata, image);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid format arg complex test\n");
        }

        metadata.format = DXGI_FORMAT_NV11;
        metadata.dimension = TEX_DIMENSION_TEXTURE3D;
        hr = ConvertToSinglePlane(nullptr, 0, metadata, image);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid dimension arg test\n");
        }
    #pragma warning(pop)
    }

    return success;
}
