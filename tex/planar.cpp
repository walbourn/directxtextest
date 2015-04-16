//-------------------------------------------------------------------------------------
// planar.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

struct TestMedia
{
    DXGI_FORMAT tformat;
    TexMetadata metadata;
    const wchar_t *fname;
};

static const TestMedia g_TestMedia[] = 
{
//  resultFormat | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
{ DXGI_FORMAT_YUY2, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv12.dds" },
{ DXGI_FORMAT_YUY2, { 3264, 2448, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"imagenv12.dds" },
{ DXGI_FORMAT_Y210, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P010, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP010.dds" },
{ DXGI_FORMAT_Y216, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P016, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP016.dds" },
{ DXGI_FORMAT_YUY2, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV11, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv11.dds" },
};

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image );
extern const wchar_t* GetName( DXGI_FORMAT fmt );

//-------------------------------------------------------------------------------------
// ConvertToSinglePlane
bool Test15()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        ExpandEnvironmentStringsW( g_TestMedia[index].fname, szPath, MAX_PATH );

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        WCHAR ext[_MAX_EXT];
        WCHAR fname[_MAX_FNAME];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        WCHAR tempDir[MAX_PATH];
        ExpandEnvironmentStringsW( TEMP_PATH L"plane", tempDir, MAX_PATH );

        CreateDirectoryW( tempDir, NULL );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );

        const TexMetadata* check = &g_TestMedia[index].metadata;
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

            //--- Simple convert --------------------------------------------------
            ScratchImage image;
            hr = ConvertToSinglePlane( *srcimage.GetImage(0,0,0), image );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed single plane conversion (HRESULT %08X):\n%S\n", hr, szPath );
            }
            else if ( image.GetMetadata().format != g_TestMedia[index].tformat
                      || image.GetMetadata().width != srcimage.GetMetadata().width || image.GetMetadata().height != srcimage.GetMetadata().height )
            {
                success = false;
                pass = false;
                printe( "Failed conversion result is %Iu x %Iu (format %S):\n",
                        image.GetMetadata().width, image.GetMetadata().height, GetName( image.GetMetadata().format ) );
                printe( "\n...(check) %Iu x %Iu (format %S):\n%S\n",
                        srcimage.GetMetadata().width, srcimage.GetMetadata().height, GetName( g_TestMedia[index].tformat ), szPath );
            }
            else
            {
                // TODO - Verify the image data (perhaps MD5 checksum)

                WCHAR tname[MAX_PATH];
                wcscpy_s( tname, fname );
                wcscat_s( tname, L"_" );
                wcscat_s( tname, GetName(g_TestMedia[index].tformat) );

                WCHAR szDestPath[MAX_PATH];
                _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

                SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );

                if ( g_TestMedia[index].tformat != DXGI_FORMAT_YUY2 )
                {
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_DXGI_FORMAT_YUY2" );

                    ScratchImage yuy2;
                    hr = Convert( *image.GetImage(0,0,0), DXGI_FORMAT_YUY2, TEX_FILTER_DEFAULT, 0.f, yuy2 );
                    if ( SUCCEEDED(hr) )
                    {
                        _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );
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

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}