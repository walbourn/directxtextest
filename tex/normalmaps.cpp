//-------------------------------------------------------------------------------------
// normalmaps.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DWORD flags, _In_ const ScratchImage& image );


//-------------------------------------------------------------------------------------
// ComputeNormalMap
bool Test11()
{
    bool success = true;

    // Heightmap
    wchar_t szPath[MAX_PATH];
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"heightmap.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#ifdef _DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    // Form dest path
    wchar_t ext[_MAX_EXT];
    wchar_t fname[_MAX_FNAME];
    _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

    wchar_t tempDir[MAX_PATH];
    ret = ExpandEnvironmentStringsW(TEMP_PATH L"nmap", tempDir, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

    CreateDirectoryW( tempDir, NULL );

    const TexMetadata checkhmap = { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    TexMetadata metadata;
    ScratchImage imagehmap;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, imagehmap );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkhmap, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%ls\n", szPath );
        printmeta( &metadata );
        printmetachk( &checkhmap );
        return false;
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

            wchar_t szDestPath[MAX_PATH];
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

            wchar_t tname[MAX_PATH] = { 0 };
            wcscpy_s( tname, fname );
            wcscat_s( tname, L"_c" );

            wchar_t szDestPath[MAX_PATH];
            _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, normalMap );
        }
    }

    return success;
}