//-------------------------------------------------------------------------------------
// normalmaps.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "tex.h"

#include "DirectXTex.h"

using namespace DirectX;

namespace
{
    struct TestMedia
    {
        TexMetadata metadata;
        const wchar_t* fname;
    };

    const TestMedia g_TestMedia[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { { 1024, 1024, 1, 1, 11, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH "heightmap.dds" },
        { { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bump_NM_height.DDS" },
        { { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dent_NM_height.DDS" },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"four_NM_height.DDS" },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rocks_NM_height.dds" },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saint_NM_height.DDS" },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"stones_NM_height.dds" },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wall_NM_height.DDS" },
    };
}

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );
extern const wchar_t* GetName(DXGI_FORMAT fmt);


//-------------------------------------------------------------------------------------
// ComputeNormalMap
bool TEXTest::Test11()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(g_TestMedia); ++index)
    {
        wchar_t szPath[MAX_PATH] = {};
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
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"nmap", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        TexMetadata metadata;
        ScratchImage imagehmap;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, imagehmap );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading dds (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in DDS:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
            return false;
        }
        else
        {
            bool pass = true;

            static const DXGI_FORMAT s_formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT };

            for (size_t f = 0; f < _countof(s_formats); ++f)
            {
                DXGI_FORMAT fmt = s_formats[f];

                // ComputeNormalMap (single)
                {
                    ScratchImage normalMap;
                    hr = ComputeNormalMap(*imagehmap.GetImage(0, 0, 0), CNMAP_CHANNEL_LUMINANCE | CNMAP_COMPUTE_OCCLUSION, 2.f, fmt, normalMap);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed computing normal map for format %u [single] (HRESULT %08X)\n", static_cast<unsigned int>(hr), fmt);
                    }
                    else
                    {
                        // TODO - Verify the image data (perhaps MD5 checksum)

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s(tname, fname);
                        wcscat_s(tname, L"_");
                        wcscat_s(tname, GetName(fmt));

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s(szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds");

                        SaveScratchImage(szDestPath, DDS_FLAGS_NONE, normalMap);
                    }
                }

                // ComputeNormalMap (complex)
                {
                    ScratchImage normalMap;
                    hr = ComputeNormalMap(imagehmap.GetImages(), imagehmap.GetImageCount(), imagehmap.GetMetadata(),
                        CNMAP_CHANNEL_LUMINANCE | CNMAP_COMPUTE_OCCLUSION, 2.f, fmt, normalMap);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed computing normal map for format %u [mipmap] (HRESULT %08X)\n", static_cast<unsigned int>(hr), fmt);
                    }
                    else
                    {
                        // TODO - Verify the image data (perhaps MD5 checksum)

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s(tname, fname);
                        wcscat_s(tname, L"_");
                        wcscat_s(tname, GetName(fmt));
                        wcscat_s(tname, L"_c");

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s(szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds");

                        SaveScratchImage(szDestPath, DDS_FLAGS_NONE, normalMap);
                    }
                }
            }

            if (pass)
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    return success;
}
