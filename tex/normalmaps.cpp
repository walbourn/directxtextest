//-------------------------------------------------------------------------------------
// normalmaps.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image );


//-------------------------------------------------------------------------------------
// ComputeNormalMap
bool Test11()
{
    bool success = true;

    // Heightmap
    WCHAR szPath[MAX_PATH];
    ExpandEnvironmentStringsW( MEDIA_PATH L"heightmap.dds", szPath, MAX_PATH );

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    // Form dest path
    WCHAR ext[_MAX_EXT];
    WCHAR fname[_MAX_FNAME];
    _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

    WCHAR tempDir[MAX_PATH];
    ExpandEnvironmentStringsW( TEMP_PATH L"nmap", tempDir, MAX_PATH );

    CreateDirectoryW( tempDir, NULL );

    const TexMetadata checkhmap = { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    TexMetadata metadata;
    ScratchImage imagehmap;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, imagehmap );
    if ( FAILED(hr) )
    {
        success = false;
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
    }
    else if ( memcmp( &metadata, &checkhmap, sizeof(TexMetadata) ) != 0 )
    {
        success = false;
        printe( "Metadata error in DDS:\n%S\n", szPath );
        printmeta( &metadata );
        printmetachk( &checkhmap );
    }

    // ComputeNormalMap (single)
    {
        ScratchImage normalMap;
        hr = ComputeNormalMap( *imagehmap.GetImage(0,0,0), CNMAP_CHANNEL_LUMINANCE | CNMAP_COMPUTE_OCCLUSION, 2.f, DXGI_FORMAT_R8G8B8A8_UNORM, normalMap );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed computing normal map [single] (HRESULT %08X)\n", hr );
        }
        else
        {
            // TODO - Verify the image data (perhaps MD5 checksum)

            WCHAR szDestPath[MAX_PATH];
            _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, fname, L".dds" );

            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, normalMap );
        }
    }

    // ComputeNormalMap (complex)
    {
        ScratchImage normalMap;
        hr = ComputeNormalMap( imagehmap.GetImages(), imagehmap.GetImageCount(), imagehmap.GetMetadata(), 
                               CNMAP_CHANNEL_LUMINANCE | CNMAP_COMPUTE_OCCLUSION, 2.f, DXGI_FORMAT_R8G8B8A8_UNORM, normalMap );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed computing normal map [mipmap] (HRESULT %08X)\n", hr );
        }
        else
        {
            // TODO - Verify the image data (perhaps MD5 checksum)

            WCHAR tname[MAX_PATH];
            wcscpy_s( tname, fname );
            wcscat_s( tname, L"_c" );

            WCHAR szDestPath[MAX_PATH];
            _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, normalMap );
        }
    }

    return success;
}