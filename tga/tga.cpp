//-------------------------------------------------------------------------------------
// tga.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

struct TestMedia
{
    TexMetadata metadata;
    const wchar_t *fname;
    uint8_t md5[16];
};

static const TestMedia g_TestMedia[] = 
{
// width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CBW8.TGA", { 0x57,0x38,0x4e,0xb8,0x4c,0x01,0xb8,0x1a,0x08,0x91,0xe1,0xc1,0x11,0x76,0x93,0xfd } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CTC16.TGA", { 0x6c,0x11,0xa9,0x94,0xa2,0x84,0x0c,0xe5,0x8c,0x34,0xbf,0xbb,0x6f,0x1d,0x3e,0x8c } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CTC24.TGA", { 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CTC32.TGA", { 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth.tga", { 0x28,0xe6,0x0f,0x9b,0xc5,0x66,0xf3,0x4e,0xb0,0x0e,0x93,0xff,0x59,0x57,0xb5,0xc5 } },
{ { 124, 124, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_B16.TGA", { 0xd0,0x77,0x67,0x8c,0x1b,0x6c,0xf4,0x38,0x5f,0xa6,0x04,0x66,0xbc,0xf6,0xd7,0x42 } },
{ { 124, 124, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_B24.TGA", { 0x41,0x7c,0x52,0x54,0x51,0x70,0xf8,0x39,0x17,0xf4,0xf4,0x8b,0xa4,0x75,0xdf,0x44 } },
{ { 124, 124, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_B32.TGA", { 0x41,0x7c,0x52,0x54,0x51,0x70,0xf8,0x39,0x17,0xf4,0xf4,0x8b,0xa4,0x75,0xdf,0x44 } },
{ { 124, 124, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_T16.TGA", { 0xd0,0x77,0x67,0x8c,0x1b,0x6c,0xf4,0x38,0x5f,0xa6,0x04,0x66,0xbc,0xf6,0xd7,0x42 } },
{ { 124, 124, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_T32.TGA", { 0x41,0x7c,0x52,0x54,0x51,0x70,0xf8,0x39,0x17,0xf4,0xf4,0x8b,0xa4,0x75,0xdf,0x44 } },
{ { 1419, 1001, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"MARBLES.TGA", { 0x2a,0x10,0x31,0x6e,0xee,0x64,0x5b,0xb8,0x3e,0x9c,0x6e,0xca,0xc6,0xd0,0x5e,0x0b } },
{ { 200, 150, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.tga", { 0x00,0x5c,0x5d,0x42,0x6c,0xed,0x64,0xc4,0xfd,0x2d,0xc5,0x67,0x89,0x2f,0x34,0x8c } },
{ { 320, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testunc.tga", { 0x0a,0x27,0xfb,0x35,0x57,0x6a,0x07,0x10,0x4a,0xf0,0x11,0x79,0xec,0x64,0x13,0x2e } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex.tga", { 0x15,0xf3,0xd1,0x76,0x0c,0xb0,0x39,0x8d,0xd3,0x0f,0x8c,0x03,0x4f,0xd1,0x09,0x95 } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UBW8.TGA", { 0x57,0x38,0x4e,0xb8,0x4c,0x01,0xb8,0x1a,0x08,0x91,0xe1,0xc1,0x11,0x76,0x93,0xfd } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UTC16.TGA", { 0x6c,0x11,0xa9,0x94,0xa2,0x84,0x0c,0xe5,0x8c,0x34,0xbf,0xbb,0x6f,0x1d,0x3e,0x8c } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UTC24.TGA", { 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
{ { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UTC32.TGA", { 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
{ { 240, 164, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_B16.TGA", { 0x57,0x9f,0x98,0xdd,0x7d,0x65,0x26,0x2f,0xf1,0x68,0x98,0x84,0x25,0x65,0x62,0x6b } },
{ { 240, 164, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_B24.TGA", { 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },
{ { 240, 164, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_B32.TGA", { 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },
{ { 240, 164, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_T16.TGA", { 0x57,0x9f,0x98,0xdd,0x7d,0x65,0x26,0x2f,0xf1,0x68,0x98,0x84,0x25,0x65,0x62,0x6b } },
{ { 240, 164, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_T24.TGA", { 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },
{ { 240, 164, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_T32.TGA", { 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },
};

//-------------------------------------------------------------------------------------

enum
{
    FLAGS_NONE          = 0x0,
    FLAGS_ALTMD5_MASK   = 0xf0,
};

#define ALTMD5(n) (n << 4)

struct SaveMedia
{
    DWORD options;
    DXGI_FORMAT src_format;
    DXGI_FORMAT sav_format;
    const wchar_t *source;
};

static const SaveMedia g_SaveMedia[] = 
{
// flags | source-dxgi-format save-dxgi-format | source-filename
//{ ALTMD5(1), DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
{ ALTMD5(2), DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"test8888.dds" },
//{ ALTMD5(3), DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
//{ FLAGS_NONE, DXGI_FORMAT_B5G5R5A1_UNORM, DXGI_FORMAT_B5G5R5A1_UNORM, MEDIA_PATH L"test555.dds" },
//{ FLAGS_NONE, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"windowslogo_L8.dds" },
//{ FLAGS_NONE, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"alphaedge.dds" },
};

struct AltMD5
{
    uint8_t md5[16];
};

static const AltMD5 g_AltMD5[] =
{
    { 0xa9,0xc6,0xf1,0xfa,0xe0,0x92,0x4f,0x44,0x31,0x24,0x3e,0xa2,0xc2,0xec,0x3a,0xa1 }, // ALTMD5(1)
    { 0x35,0x80,0x55,0xf9,0xd6,0x91,0x58,0x0a,0x8c,0xf3,0x37,0x29,0x57,0xc1,0x85,0xf0 }, // ALTMD5(2)
    { 0x8d,0x2c,0xc5,0xd3,0xb1,0x44,0x64,0x39,0x30,0xe0,0xca,0x36,0x1c,0xeb,0x17,0xaa }, // ALTMD5(3)
};

//-------------------------------------------------------------------------------------

extern HRESULT LoadBlobFromFile( _In_z_ LPCWSTR szFile, Blob& blob );
extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest );
extern HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// GetMetadataFromTGAMemory/File
bool Test01()
{
    bool success = true;

    // GetMetadataFromTGAMemory is used internally to the File version, so we only need to explicitly test the File version

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        HRESULT hr = GetMetadataFromTGAFile( szPath, metadata );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting data from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in:\n%S\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            ++npass;
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromTGAMemory
bool Test02()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        Blob blob;
        HRESULT hr = LoadBlobFromFile( szPath, blob );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting raw file data from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else
        {
            TexMetadata metadata;
            ScratchImage image;
            hr = LoadFromTGAMemory( blob.GetBufferPointer(), blob.GetBufferSize(), &metadata, image );

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed loading tga from memory (HRESULT %08X):\n%S\n", hr, szPath );
            }
            else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
            {
                success = false;
                printe( "Metadata error in:\n%S\n", szPath );
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
                    printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%S\n", hr, szPath );
                }
                else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0 )
                {
                    success = false;
                    printe( "Failed comparing MD5 checksum:\n%S\n", szPath );
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

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromTGAFile
bool Test03()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromTGAFile( szPath, &metadata, image );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading tga from memory (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in:\n%S\n", szPath );
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
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%S\n", hr, szPath );
            }
            else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0 )
            {
                success = false;
                printe( "Failed comparing MD5 checksum:\n%S\n", szPath );
                printdigest( "computed", digest );
                printdigest( "expected", g_TestMedia[index].md5 );
            }
            else
                ++npass;

            // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveTGAToMemory
bool Test04()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_SaveMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_SaveMedia[index].source, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading DDS from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else
        {
            if ( metadata.format != g_SaveMedia[index].src_format )
            {
                success = false;
                printe( "Unexpected DDS pixel format (found %d, expected %d):\n%S\n", metadata.format, g_SaveMedia[index].src_format, szPath );
            }

            uint8_t digest[16];
            hr = MD5Checksum( image, digest );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%S\n", hr, szPath );
            }
            else
            {
                Blob blob;
                hr = SaveToTGAMemory( *image.GetImage(0,0,0), blob );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed writing tga to memory (HRESULT %08X):\n%S\n", hr, szPath );
                }
                else
                {
                    //TESTEST- { HANDLE h = CreateFileW( L"C:\\TEMP\\XXX.TGA", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0 ); DWORD bytesWritten; WriteFile( h, blob.GetBufferPointer(), static_cast<DWORD>( blob.GetBufferSize() ), &bytesWritten, 0 ); CloseHandle(h); }
                    TexMetadata metadata2;
                    ScratchImage image2;
                    hr = LoadFromTGAMemory( blob.GetBufferPointer(), blob.GetBufferSize(), &metadata2, image2 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed reading back written tga to memory (HRESULT %08X):\n%S\n", hr, szPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata2.mipLevels != 1
                              || metadata.dimension != metadata2.dimension
                              || g_SaveMedia[index].sav_format != metadata2.format )
                    { // Formats can vary for readback, and miplevel is going to be 1 for TGA images
                        success = false;
                        printe( "Metadata error in tga memory readback:\n%S\n", szPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else
                    {
    // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX1.DDS", DDS_FLAGS_NONE, image2 );

                        const uint8_t* expected = digest;
                        if ( g_SaveMedia[index].options & FLAGS_ALTMD5_MASK )
                        {
                            expected = g_AltMD5[ ((g_SaveMedia[index].options & 0xf0) >> 4) - 1 ].md5;
                        }

                        uint8_t digest2[16];
                        hr = MD5Checksum( image2, digest2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed computing MD5 checksum of reloaded image data (HRESULT %08X):\n%S\n", hr, szPath );
                        }
                        else if ( memcmp( expected, digest2, 16 ) != 0 )
                        {
                            success = false;
                            printe( "MD5 checksum of reloaded data doesn't match original:\n%S\n", szPath );
                        }
                        else
                            ++npass;
                    }
                }
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveTGAToFile
bool Test05()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_SaveMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(g_SaveMedia[index].source, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#ifdef DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        WCHAR ext[_MAX_EXT];
        WCHAR fname[_MAX_FNAME];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        WCHAR tempDir[MAX_PATH];
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"tga", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, NULL );

        WCHAR szDestPath[MAX_PATH];
        _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, fname, L".tga" );

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading DDS from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else
        {
            if ( metadata.format != g_SaveMedia[index].src_format )
            {
                success = false;
                printe( "Unexpected DDS pixel format (found %d, expected %d):\n%S\n", metadata.format, g_SaveMedia[index].src_format, szPath );
            }

            uint8_t digest[16];
            hr = MD5Checksum( image, digest );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%S\n", hr, szPath );
            }
            else
            {
                hr = SaveToTGAFile( *image.GetImage(0,0,0), szDestPath );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed writing tga to (HRESULT %08X):\n%S\n", hr, szDestPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;
                    hr = LoadFromTGAFile( szDestPath, &metadata2, image2 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed reading back written tga to (HRESULT %08X):\n%S\n", hr, szDestPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata2.mipLevels != 1
                              || metadata.dimension != metadata2.dimension
                              || g_SaveMedia[index].sav_format != metadata2.format  )
                    {   // Formats can vary for readback, and miplevel is going to be 1 for TGA images
                        success = false;
                        printe( "Metadata error in tga readback:\n%S\n", szDestPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else
                    {
    // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX2.DDS", DDS_FLAGS_NONE, image2 );

                        const uint8_t* expected = digest;
                        if ( g_SaveMedia[index].options & FLAGS_ALTMD5_MASK )
                        {
                            expected = g_AltMD5[ ((g_SaveMedia[index].options & 0xf0) >> 4) - 1 ].md5;
                        }

                        uint8_t digest2[16];
                        hr = MD5Checksum( image2, digest2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed computing MD5 checksum of reloaded image data (HRESULT %08X):\n%S\n", hr, szPath );
                        }
                        else if ( memcmp( expected, digest2, 16 ) != 0 )
                        {
                            success = false;
                            printe( "MD5 checksum of reloaded data doesn't match original:\n%S\n", szPath );
                        }
                        else
                            ++npass;
                    }
                }
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}
