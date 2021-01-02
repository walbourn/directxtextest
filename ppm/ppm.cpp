//-------------------------------------------------------------------------------------
// ppm.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"

#include "scoped.h"

using namespace DirectX;

HRESULT __cdecl LoadFromPortablePixMap(
    _In_z_ const wchar_t* szFile,
    _Out_opt_ TexMetadata* metadata,
    _Out_ ScratchImage& image) noexcept;

HRESULT __cdecl SaveToPortablePixMap(
    _In_ const Image& image,
    _In_z_ const wchar_t* szFile) noexcept;

HRESULT __cdecl LoadFromPortablePixMapHDR(
    _In_z_ const wchar_t* szFile,
    _Out_opt_ TexMetadata* metadata,
    _Out_ ScratchImage& image) noexcept;

HRESULT __cdecl SaveToPortablePixMapHDR(
    _In_ const Image& image,
    _In_z_ const wchar_t* szFile) noexcept;

namespace
{
    struct TestMedia
    {
        TexMetadata metadata;
        const wchar_t *fname;
        uint8_t md5[16];
    };

    const TestMedia g_TestMedia[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { { 615, 461, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"image.ppm",
            { 0x32,0x27,0xc7,0x8d,0x57,0x23,0xff,0x3f,0xe5,0x55,0xf5,0xd7,0xb5,0x94,0x94,0xd1 } },
        { { 501, 501, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"mandelbrot.ppm",
            { 0xdb,0xfa,0x11,0xaf,0xc1,0xdc,0x89,0x12,0x4b,0x9f,0x90,0xa4,0xea,0x46,0x72,0x35 } },
        { { 227, 149, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testimg.ppm",
            { 0xaa,0xfc,0x17,0x0b,0x55,0x56,0x5d,0xc1,0x3e,0xe2,0x08,0x85,0x59,0xdd,0xe4,0x11 } },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"input_small.ppm",
            { 0x1d,0x5c,0x6a,0x69,0xb6,0x02,0x3e,0x03,0x29,0xb0,0x61,0xd6,0x1c,0x5f,0x3c,0xd6 } },
        { { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"input_large.ppm",
            { 0x19,0xf5,0xbc,0x0a,0x5b,0x73,0x7c,0xcc,0x76,0xa7,0x0e,0xc2,0xf1,0x2d,0xd5,0x0e } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d.pfm",
            { 0x44,0xa9,0xbb,0x73,0x1d,0xf5,0x29,0x20,0x1a,0x53,0x6c,0xb1,0x7f,0x5d,0x68,0x81 } },
        { { 1500, 1500, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"uffizi_probe.pfm",
            { 0x7f,0x51,0xf1,0x70,0x9b,0x47,0x1d,0x15,0x9d,0x9d,0x81,0x12,0xfd,0xc6,0x81,0xe4 } },
        { { 512, 768, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"memorial.pfm",
            { 0x9d,0xb5,0xa2,0xac,0x55,0x9f,0xd2,0x0e,0x10,0xa9,0xeb,0xc0,0xb1,0x7b,0x24,0x6b } },
        { { 512, 768, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"memorial_be.pfm",
            { 0x3b,0x2f,0x29,0x1b,0x44,0xfa,0x41,0x8a,0xe7,0xda,0xa6,0x82,0x6e,0xc1,0x0e,0xd0 } },
        { { 512, 768, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"memorial_be_com.pfm",
            { 0x3b,0x2f,0x29,0x1b,0x44,0xfa,0x41,0x8a,0xe7,0xda,0xa6,0x82,0x6e,0xc1,0x0e,0xd0 } },
    };

    //-------------------------------------------------------------------------------------

    struct SaveMedia
    {
        bool pfm;
        DXGI_FORMAT src_format;
        DXGI_FORMAT sav_format;
        const wchar_t *source;
    };

    const SaveMedia g_SaveMedia[] =
    {
        // source-dxgi-format save-dxgi-format | source-filename
        { false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"lena.dds" },
        { false, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"test8888.dds" },
        { false, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"reftexture.dds" },
        { false, DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"ball.dds" },
        { false, DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"cookie.dds" },

        { true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, MEDIA_PATH L"SnowPano_4k_Ref.DDS" },
        { true, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, MEDIA_PATH L"windowslogo_r32f.dds" },
        { true, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, MEDIA_PATH L"SnowPano_4k_Ref_96bpp.DDS" },
        { true, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, MEDIA_PATH L"yucca.dds" },

        #ifdef _M_X64
        // Very large images
        { false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"earth16kby16k.dds" },
        #endif
    };

    inline bool IsErrorTooLarge(float f, float threshold)
    {
        return (fabsf(f) > threshold) != 0;
    }
}

//-------------------------------------------------------------------------------------

extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest );
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// LoadFromPortablePixMap
bool Test01()
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

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        const TexMetadata* check = &g_TestMedia[index].metadata;

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr;
        switch (check->format)
        {
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
            hr = LoadFromPortablePixMapHDR(szPath, &metadata, image);
            break;

        default:
            hr = LoadFromPortablePixMap(szPath, &metadata, image);
            break;
        }

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading ppm/pfm from file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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
            uint8_t digest[16];
            hr = MD5Checksum( image, digest );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0 )
            {
                success = false;
                printe( "Failed comparing MD5 checksum:\n%ls\n", szPath );
                printdigest( "computed", digest );
                printdigest( "expected", g_TestMedia[index].md5 );
            }
            else
                ++npass;

            // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveToPortablePixMap
bool Test02()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_SaveMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_SaveMedia[index].source, szPath, MAX_PATH);
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
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"ppm", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        wchar_t szDestPath[MAX_PATH] = {};
        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, g_SaveMedia[index].pfm ? L".pfm" : L".ppm" );

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading DDS from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else
        {
            if ( metadata.format != g_SaveMedia[index].src_format )
            {
                success = false;
                printe( "Unexpected DDS pixel format (found %d, expected %d):\n%ls\n", metadata.format, g_SaveMedia[index].src_format, szPath );
            }

            uint8_t digest[16];
            hr = MD5Checksum( image, digest );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else
            {
                bool pass = true;

                if (g_SaveMedia[index].pfm)
                {
                    hr = SaveToPortablePixMapHDR(*image.GetImage(0, 0, 0), szDestPath);
                }
                else
                {
                    hr = SaveToPortablePixMap(*image.GetImage(0, 0, 0), szDestPath);
                }
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing ppm/pfm to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;

                    if (g_SaveMedia[index].pfm)
                    {
                        hr = LoadFromPortablePixMapHDR(szDestPath, &metadata2, image2);
                    }
                    else
                    {
                        hr = LoadFromPortablePixMap(szDestPath, &metadata2, image2);
                    }
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written ppm/pfm to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata2.mipLevels != 1
                              || metadata.dimension != metadata2.dimension
                              || g_SaveMedia[index].sav_format != metadata2.format  )
                    {   // Formats can vary for readback, and miplevel is going to be 1 for ppm images
                        success = false;
                        pass = false;
                        printe( "Metadata error in ppm/pfm readback:\n%ls\n", szDestPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else if (g_SaveMedia[index].src_format == g_SaveMedia[index].sav_format)
                    {
                        // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX2.DDS", DDS_FLAGS_NONE, image2 );

                        const uint8_t* expected = digest;

                        uint8_t digest2[16];
                        hr = MD5Checksum( image2, digest2 );
                        if ( FAILED(hr) )
                        {
                            pass = false;
                            success = false;
                            printe( "Failed computing MD5 checksum of reloaded image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( memcmp( expected, digest2, 16 ) != 0 )
                        {
                            pass = false;
                            success = false;
                            printe( "MD5 checksum of reloaded data doesn't match original:\n%ls\n", szPath );
                        }
                    }
                    else
                    {
                        float targMSE = 0.00000001f;
                        float mse = 0, mseV[4] = {};
                        CMSE_FLAGS cflags = CMSE_IGNORE_ALPHA;
                        if (IsSRGB(g_SaveMedia[index].src_format))
                            cflags |= CMSE_IMAGE2_SRGB;

                        hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV, cflags);
                        if (FAILED(hr))
                        {
                            success = false;
                            printe("Failed comparing original vs. reloaded image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                        }
                        else if (IsErrorTooLarge(mse, targMSE))
                        {
                            success = false;
                            printe("Failed comparing original vs. reloaded image MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath);
                        }

                    }

                }

                if (pass)
                    ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// Fuzz
bool Test03()
{
    bool success = true;

    wchar_t szMediaPath[MAX_PATH] = {};
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH, szMediaPath, MAX_PATH);
    if (!ret || ret > MAX_PATH)
    {
        printe("ERROR: ExpandEnvironmentStrings FAILED\n");
        return false;
    }

    wchar_t szPath[MAX_PATH] = {};
    wcscpy_s(szPath, szMediaPath);
    wcscat_s(szPath, L"*.*");

    WIN32_FIND_DATA findData = {};
    ScopedFindHandle hFile(safe_handle(FindFirstFileEx(szPath,
        FindExInfoBasic, &findData,
        FindExSearchNameMatch, nullptr,
        FIND_FIRST_EX_LARGE_FETCH)));
    if (!hFile)
    {
        printe("ERROR: FindFirstFileEx FAILED (%lu)\n", GetLastError());
        return false;
    }

    size_t ncount = 0;

    for (;;)
    {
        if (!(findData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
        {
            ++ncount;

            if (!(ncount % 100))
            {
                print(".");
            }

            wcscpy_s(szPath, szMediaPath);
            wcscat_s(szPath, findData.cFileName);

            bool isppm = false;
            bool ispfm = false;
            {
                wchar_t ext[_MAX_EXT];
                wchar_t fname[_MAX_FNAME];
                _wsplitpath_s(findData.cFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

                isppm = (_wcsicmp(ext, L".ppm") == 0);
                ispfm = (_wcsicmp(ext, L".pfm") == 0);
            }

            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");

            // PPM
            {
                TexMetadata metadata;
                ScratchImage image;
                HRESULT hr = LoadFromPortablePixMap(szPath, &metadata, image);

                if (FAILED(hr) && isppm)
                {
                    if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                    {
                        success = false;
                        printe("ERROR: fromfile ppm expected success ! (%08X)\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                }
                else if (SUCCEEDED(hr) && !isppm)
                {
                    success = false;
                    printe("ERROR: fromfile ppm expected failure\n%ls\n", szPath);
                }
            }

            // PFM
            {
                TexMetadata metadata;
                ScratchImage image;
                HRESULT hr = LoadFromPortablePixMapHDR(szPath, &metadata, image);

                if (FAILED(hr) && ispfm)
                {
                    if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                    {
                        success = false;
                        printe("ERROR: fromfile pfm expected success ! (%08X)\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                }
                else if (SUCCEEDED(hr) && !ispfm)
                {
                    success = false;
                    printe("ERROR: fromfile pfm expected failure\n%ls\n", szPath);
                }
            }
        }

        if (!FindNextFile(hFile.get(), &findData))
            break;
    }

    if (!ncount)
    {
        printe("ERROR: expected to find test images\n");
        return false;
    }

    print(" %zu images tested ", ncount);

    return success;
}
