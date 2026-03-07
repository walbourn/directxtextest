//-------------------------------------------------------------------------------------
// exr.cpp
//
// Test for Auxiliary implementation of EXR using OpenEXR
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"

#include "scoped.h"
#include "DirectXTexEXR.h"

using namespace DirectX;

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
        { { 100, 50, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"stripes.exr", { 0x5b,0x9e,0x70,0x61,0x7e,0x2b,0x24,0xc8,0xba,0x78,0xff,0xa4,0x65,0x76,0x72,0xb1 } },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"GammaChart.exr", { 0xc5,0x25,0x22,0x66,0xee,0xb5,0xfc,0x76,0xbb,0xd8,0x4d,0xbd,0x8e,0x90,0xa4,0x60 } },

    #ifndef BUILD_BVT_ONLY
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"AllHalfValues.exr", { 0xbd,0x40,0x99,0x26,0x3e,0xe7,0xd9,0xc6,0x4d,0x82,0x35,0x9e,0x0c,0x0c,0x4b,0x18 } },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BrightRings.exr", { 0xe7,0xa7,0xb2,0x20,0x17,0xfd,0x9e,0x77,0x28,0x13,0x5f,0xca,0x80,0x5c,0x2d,0xfc } },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BrightRingsNanInf.exr", { 0x03,0x5b,0xca,0xd1,0x92,0x1d,0x92,0x66,0xfd,0xa5,0x2e,0x2c,0x5b,0x59,0xf0,0x41 } },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"GrayRampsDiagonal.exr", { 0x03,0x08,0x42,0xf0,0x9d,0x1f,0x74,0xed,0x34,0xba,0x62,0xe6,0x5e,0x2e,0x02,0xc7 } },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"GrayRampsHorizontal.exr", { 0x8c,0xf2,0xa1,0x42,0x5d,0x13,0xc9,0xb5,0x47,0xfc,0x24,0x62,0xd7,0x23,0x08,0xe6 } },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"RgbRampsDiagonal.exr", { 0x82,0x27,0x99,0x40,0x4d,0x91,0xef,0x4f,0xd3,0xfc,0x94,0x95,0x21,0x89,0x78,0x11 } },
        { { 1000, 1000, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SquaresSwirls.exr", { 0xd9,0x5f,0x33,0x3b,0x43,0xf8,0x98,0x5b,0xaf,0x21,0x95,0x7d,0x80,0xa9,0xbd,0xf4 } },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"WideColorGamut.exr", { 0xcf,0xa1,0xcb,0x3a,0xe8,0xa8,0x38,0xf3,0x50,0xde,0x88,0x4b,0x03,0x28,0x68,0x08 } },
        { { 500, 500, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"WideFloatRange.exr", { 0x14,0x91,0x97,0xce,0x62,0xdd,0x31,0xa5,0x7a,0x88,0x74,0x13,0xe5,0xbd,0x42,0x19 } },
        { { 1000, 810, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CandleGlass.exr", { 0x4f,0x82,0x7a,0x0e,0xa8,0x2e,0x43,0xad,0x25,0x30,0x5f,0x7b,0x43,0x3c,0x2b,0xcc } },
        { { 1200, 865, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"PrismsLenses.exr", { 0xee,0x39,0x1f,0x8c,0x26,0xc5,0x00,0xfe,0xa7,0x9b,0x72,0x48,0x8c,0x2b,0x01,0x0b } },
        { { 1262, 860, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"GoldenGate.exr", { 0x99,0x8a,0x8f,0xa8,0xc0,0x60,0x94,0x0f,0x13,0x53,0xbb,0x49,0x63,0x67,0x2e,0xd5 } },
        { { 610, 406, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Rec709.exr", { 0x89,0x8d,0xb7,0x36,0x3c,0xc5,0x2f,0x3c,0x43,0xbb,0x8e,0xb9,0x8e,0x46,0xc8,0x58 } },
        { { 610, 406, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XYZ.exr", { 0x14,0xee,0x5b,0x66,0x60,0x7e,0x17,0x33,0xda,0xfa,0x98,0xc4,0x57,0xbc,0xc0,0x54 } },
        { { 874, 493, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Garden.exr", { 0xac,0x81,0xa2,0xa4,0xc4,0x00,0x1e,0x4f,0xde,0x39,0xa4,0xc9,0x6b,0x1f,0x12,0x72 } },

        // Cubemap
        { { 256, 256, 1, 6, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"KernerEnvCube.exr", { 0x2b,0xfe,0xb1,0xf1,0x06,0x92,0x38,0x6b,0x8a,0x22,0x3f,0x4b,0x26,0xca,0xf2,0x96 } },

        // Stereo
        // TODO - { { 1431, 761, 2, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Balls.exr", { 0 } },

    #endif // !BUILD_BVT_ONLY
        };

    //-------------------------------------------------------------------------------------

    struct SaveMedia
    {
        DXGI_FORMAT src_format;
        DXGI_FORMAT sav_format;
        const wchar_t *source;
    };

    const SaveMedia g_SaveMedia[] =
    {
        // source-dxgi-format save-dxgi-format | source-filename
        { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, MEDIA_PATH L"GammaChart.exr" },

    #ifndef BUILD_BVT_ONLY
    #endif // !BUILD_BVT_ONLY
    };

    inline bool IsErrorTooLarge(float f, float threshold)
    {
        return (fabsf(f) > threshold) != 0;
    }
}

//-------------------------------------------------------------------------------------

extern HRESULT LoadBlobFromFile( _In_z_ const wchar_t* szFile, Blob& blob );
extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest );
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// GetMetadataFromEXRFile
bool Test01()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
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

        const TexMetadata* check = &g_TestMedia[index].metadata;

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = GetMetadataFromEXRFile(szPath, metadata);
        if (FAILED(hr))
        {
            success = false;
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if (memcmp(&metadata, check, sizeof(TexMetadata)) != 0)
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta(&metadata);
            printmetachk(check);
        }
        else
        {
            ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        TexMetadata metadata;
        HRESULT hr = GetMetadataFromEXRFile(nullptr, metadata);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid arg file test\n");
        }
    #pragma warning(pop)
    }

    return success;
}

//-------------------------------------------------------------------------------------
// LoadFromEXRMemory
bool Test02()
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

        Blob blob;
        HRESULT hr = LoadBlobFromFile( szPath, blob );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting raw file data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else
        {
            TexMetadata metadata;
            ScratchImage image;
            hr = LoadFromEXRMemory(blob.GetBufferPointer(), blob.GetBufferSize(), &metadata, image);

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed loading exr from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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

            image.Release();
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        ScratchImage image;
        HRESULT hr = LoadFromEXRMemory(static_cast<const uint8_t*>(nullptr), 0, nullptr, image);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid arg test\n");
        }
    #pragma warning(pop)
    }

    return success;
}

//-------------------------------------------------------------------------------------
// LoadFromEXRFile
bool Test03()
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
        HRESULT hr = LoadFromEXRFile(szPath, &metadata, image);
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading exr from file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        ScratchImage image;
        HRESULT hr = LoadFromEXRFile(nullptr, nullptr, image);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid arg test\n");
        }

        hr = LoadFromEXRFile(L"TestFileDoesNotExist.EXR", nullptr, image);
        if (hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            success = false;
            printe("Failed missing file test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
        }
    #pragma warning(pop)
    }

    return success;
}


//-------------------------------------------------------------------------------------
// SaveToEXRFile
bool Test04()
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"exr", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        wchar_t szDestPath[MAX_PATH] = {};
        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".exr");

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromEXRFile( szPath, &metadata, image );

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading EXR from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else
        {
            if ( metadata.format != g_SaveMedia[index].src_format )
            {
                success = false;
                printe( "Unexpected EXR pixel format (found %d, expected %d):\n%ls\n", metadata.format, g_SaveMedia[index].src_format, szPath );
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

                hr = SaveToEXRFile(*image.GetImage(0, 0, 0), szDestPath);
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing exr to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;

                    hr = LoadFromEXRFile(szDestPath, &metadata2, image2);
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written exr to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata2.mipLevels != 1
                              || metadata.dimension != metadata2.dimension
                              || g_SaveMedia[index].sav_format != metadata2.format  )
                    {   // Formats can vary for readback, and miplevel is going to be 1 for exr images
                        success = false;
                        pass = false;
                        printe( "Metadata error in exr readback:\n%ls\n", szDestPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else
                    {
                        float targMSE = 0.2f;
                        float mse = 0, mseV[4] = {};
                        CMSE_FLAGS cflags = CMSE_DEFAULT;
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

                hr = SaveToEXRFile(*image.GetImage(0, 0, 0),  nullptr);
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    pass = false;
                    printe("Failed null fname test (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath);
                }

                if (pass)
                    ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        Image nullin = {};
        nullin.width = nullin.height = 256;
        nullin.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        HRESULT hr = SaveToEXRFile(nullin, nullptr);
        if (hr != E_INVALIDARG && hr != E_POINTER)
        {
            success = false;
            printe("Failed invalid arg test\n");
        }

        hr = SaveToEXRFile(nullin, L"TestFileInvalid.exr");
        if (hr != E_POINTER)
        {
            success = false;
            printe("Failed invalid image arg test (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        }
    #pragma warning(pop)
    }

    return success;
}


//-------------------------------------------------------------------------------------
// Fuzz
bool Test05()
{
    bool success = true;

    wchar_t szMediaPath[MAX_PATH] = {};
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH, szMediaPath, MAX_PATH);
    if (!ret || ret > MAX_PATH)
    {
        printe("ERROR: ExpandEnvironmentStrings FAILED\n");
        return false;
    }

    bool second = false;

    wchar_t szPath[MAX_PATH] = {};
    wcscpy_s(szPath, szMediaPath);
    wcscat_s(szPath, L"*.*");

    WIN32_FIND_DATA findData = {};
    ScopedFindHandle hFile(safe_handle(FindFirstFileExW(szPath,
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

            wcscpy_s(szPath, (second) ? L"" : szMediaPath);
            wcscat_s(szPath, findData.cFileName);

            bool isexr = false;
            {
                wchar_t ext[_MAX_EXT];
                wchar_t fname[_MAX_FNAME];
                _wsplitpath_s(findData.cFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

                isexr = (_wcsicmp(ext, L".exr") == 0);
            }

            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");

            {
                TexMetadata metadata;
                ScratchImage image;
                HRESULT hr = LoadFromEXRFile(szPath, &metadata, image);

                if (FAILED(hr) && isexr)
                {
                    if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && !second)
                    {
                        success = false;
                        printe("ERROR: fromfile expected success ! (%08X)\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                }
                else if (SUCCEEDED(hr) && !isexr)
                {
                    success = false;
                    printe("ERROR: fromfile expected failure\n%ls\n", szPath);
                }
            }
        }

        if (!FindNextFileW(hFile.get(), &findData))
        {
            if (second)
                break;

            hFile.reset(safe_handle(FindFirstFileExW(L"*.exr",
                FindExInfoBasic, &findData,
                FindExSearchNameMatch, nullptr,
                FIND_FIRST_EX_LARGE_FETCH)));
            if (!hFile)
                break;

            second = true;
        }
    }

    if (!ncount)
    {
        printe("ERROR: expected to find test images\n");
        return false;
    }

    print(" %zu images tested ", ncount);

    return success;
}
