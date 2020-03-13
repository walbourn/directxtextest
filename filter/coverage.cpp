//-------------------------------------------------------------------------------------
// coverage.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"
#include "filter.h"

#include "directxtex.h"

using namespace DirectX;

#pragma warning(disable:6262) // test code doesn't need conservative stack size

namespace
{
    enum
    {
        FLAGS_NONE = 0x0,
    };

    struct MipMapMedia
    {
        DWORD options;
        TexMetadata metadata;
        const wchar_t *fname;
    };

    const MipMapMedia g_MipMapMedia[] =
    {
        // <source> test-options | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds" },
        { FLAGS_NONE, { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds"  },
        { FLAGS_NONE, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds" },
        { FLAGS_NONE, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds" },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds" },
        { FLAGS_NONE, { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds" },
        { FLAGS_NONE, { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.dds" }
    };
}

//-------------------------------------------------------------------------------------

extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages=0 );
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// ScaleMipMapsAlphaForCoverage
bool FilterTest::Test04()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_MipMapMedia); ++index )
    {
        ++ncount;

        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_MipMapMedia[index].fname, szPath, MAX_PATH);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"cvg", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr;

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );
        }
        else
        {
            hr = LoadFromWICFile( szPath, WIC_FLAGS_NONE, &metadata, srcimage );
        }

        const TexMetadata* check = &g_MipMapMedia[index].metadata;
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
            assert( metadata.dimension == TEX_DIMENSION_TEXTURE1D || metadata.dimension == TEX_DIMENSION_TEXTURE2D );

            uint8_t srcdigest[16];
            hr = MD5Checksum( srcimage, srcdigest, 1 );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing MD5 checksum (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else
            {
                bool pass = true;

                ScratchImage mipChain;
                hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_DEFAULT, 0, mipChain, false );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Fant/Box generation (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                }
                else if ( mipChain.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Fant/Box generation result is %zu mip levels:\n%ls\n", mipChain.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChain, digest, 1 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing Fant/Box MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Fant/Box top MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Scale with alpha coverage
                    auto& coverageMeta = mipChain.GetMetadata();
                    ScratchImage coverageMipChain;
                    hr = coverageMipChain.Initialize(coverageMeta);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failing creating coverage mipchain (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                        // 0.5f
                        for (size_t item = 0; item < coverageMeta.arraySize; ++item)
                        {
                            auto img = mipChain.GetImage(0, item, 0);
                            assert(img);

                            hr = ScaleMipMapsAlphaForCoverage(img, coverageMeta.mipLevels, coverageMeta, item, 0.5f,
                                coverageMipChain);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failing scaling mips coverage 0.5 mipchain (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                            }
                            else
                            {
                                wchar_t fname2[_MAX_FNAME] = {};
                                wcscpy_s(fname2, fname);
                                wcscat_s(fname2, L"_0_5");

                                wchar_t szDestPath[MAX_PATH] = {};
                                _wmakepath_s(szDestPath, MAX_PATH, nullptr, tempDir, fname2, L".dds");

                                SaveScratchImage(szDestPath, DDS_FLAGS_NONE, coverageMipChain);
                            }
                        }

                        // 0.8
                        for (size_t item = 0; item < coverageMeta.arraySize; ++item)
                        {
                            auto img = mipChain.GetImage(0, item, 0);
                            assert(img);

                            hr = ScaleMipMapsAlphaForCoverage(img, coverageMeta.mipLevels, coverageMeta, item, 0.8f,
                                coverageMipChain);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failing scaling mips coverage 0.8 mipchain (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                            }
                            else
                            {
                                wchar_t fname2[_MAX_FNAME] = {};
                                wcscpy_s(fname2, fname);
                                wcscat_s(fname2, L"_0_8");

                                wchar_t szDestPath[MAX_PATH] = {};
                                _wmakepath_s(szDestPath, MAX_PATH, nullptr, tempDir, fname2, L".dds");

                                SaveScratchImage(szDestPath, DDS_FLAGS_NONE, coverageMipChain);
                            }
                        }
                    }
                }

                if ( pass )
                    ++npass;
            }
        }
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}
