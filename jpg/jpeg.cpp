//-------------------------------------------------------------------------------------
// jpeg.cpp
//
// Test for Auxiliary implementation of LIB using LIBJPEG directly instead of WIC.
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"

#include "scoped.h"
#include "DirectXTexJPEG.h"

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

        { { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"fishingboat.jpg", { 0xef,0xea,0x23,0xe7,0x85,0xd2,0xd9,0x10,0x55,0x1d,0xa8,0x14,0xd4,0xaf,0x53,0xca } },
        {  { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg", { 0xef,0x7a,0x90,0x3e,0xa3,0x25,0x3d,0xf9,0x65,0x37,0x77,0x5a,0x74,0xe4,0x53,0x1b } },
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
        { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"lena.dds" },
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
        HRESULT hr = GetMetadataFromJPEGFile(szPath, metadata);
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

    return success;
}

//-------------------------------------------------------------------------------------
// LoadFromPNGFile
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

        const TexMetadata* check = &g_TestMedia[index].metadata;

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromJPEGFile(szPath, &metadata, image);
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading jpg from file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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

            // TESTTEST-
            SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveToPNGFile
bool Test03()
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
        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".jpg");

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

                hr = SaveToJPEGFile(*image.GetImage(0, 0, 0), szDestPath);
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing jpg to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;

                    hr = LoadFromJPEGFile(szDestPath, &metadata2, image2);
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written jpg to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
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
                        printe( "Metadata error in jpg readback:\n%ls\n", szDestPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else
                    {
                        float targMSE = 0.0001f;
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
bool Test04()
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

            wcscpy_s(szPath, (second) ? L"" : szMediaPath);
            wcscat_s(szPath, findData.cFileName);

            bool isjpg = false;
            {
                wchar_t ext[_MAX_EXT];
                wchar_t fname[_MAX_FNAME];
                _wsplitpath_s(findData.cFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

                isjpg = (_wcsicmp(ext, L".jpg") == 0);
            }

            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");

            {
                TexMetadata metadata;
                ScratchImage image;
                HRESULT hr = LoadFromJPEGFile(szPath, &metadata, image);

                if (FAILED(hr) && isjpg)
                {
                    if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && !second)
                    {
                        success = false;
                        printe("ERROR: fromfile expected success ! (%08X)\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                }
                else if (SUCCEEDED(hr) && !isjpg)
                {
                    success = false;
                    printe("ERROR: fromfile expected failure\n%ls\n", szPath);
                }
            }
        }

        if (!FindNextFile(hFile.get(), &findData))
        {
            if (second)
                break;

            hFile.reset(safe_handle(FindFirstFileEx(L"*.jpg",
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
