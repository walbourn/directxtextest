//-------------------------------------------------------------------------------------
// hdr.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"

#include "scoped.h"

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

        // "Adaptive RLE" test cases
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d.hdr",{ 0xa2,0xad,0x45,0x5d,0x73,0xa5,0xf1,0xc1,0xfd,0xc8,0x07,0xe9,0xdc,0x07,0xcc,0x5a } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"memorial.hdr",{ 0xa9,0x7d,0x42,0x3d,0x26,0xb0,0xe8,0xf9,0xfc,0x24,0x9c,0x3c,0x5a,0x7e,0x04,0x78 } },
        { { 666, 1002, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BigTree.hdr",{ 0x72,0x03,0xe2,0xbb,0x36,0x16,0x72,0x7f,0x96,0x2a,0x2f,0xca,0xf5,0x31,0x77,0x7b } },
        { { 1536, 2048, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"uffizi_cross.hdr",{ 0x8f,0x72,0x59,0xc8,0x54,0x98,0x14,0x11,0x89,0x1d,0xd4,0xd5,0x8d,0x1c,0x27,0xce } },
        { { 852, 1136, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"sfmoma1.hdr",{ 0xae,0xa9,0x1b,0xaa,0x4d,0xdd,0x16,0x3a,0xb6,0x63,0xb9,0xc4,0x88,0xcd,0xfd,0xba } },
        { { 852, 1136, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"EMPstair.hdr",{ 0x3c,0x76,0xf0,0xb1,0xfc,0x40,0x6b,0xf6,0x43,0xcd,0x64,0x96,0x4c,0x07,0xfd,0x9a } },
        { { 1840, 1224, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"aura_shop.hdr",{ 0x53,0xcd,0x02,0x23,0x12,0x4b,0x1f,0x3d,0x94,0x7f,0xf0,0x45,0x84,0x65,0x3d,0x0a } },
        { { 2048, 1024, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SnowPano_4k_Ref.hdr",{ 0x74,0x91,0xda,0xe8,0x1a,0xa5,0x4a,0x13,0xb2,0x99,0x6d,0x36,0x06,0x9f,0x59,0xea } },

        // Uncompressed test cases
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4dunc.hdr",{ 0xc8,0xd6,0x09,0xbd,0x6d,0x59,0x97,0x5d,0xe4,0xa1,0xda,0xf1,0x22,0x22,0x43,0x55 } },
        { { 666, 1002, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BigTreeUnc.hdr",{ 0x68,0xef,0x8c,0x7b,0x38,0x21,0x1f,0x27,0xc0,0xb5,0x96,0x2a,0x4f,0x5b,0x3d,0x0d } },

        // Legacy "Standard RLE" test case
        { { 1536, 2048, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"uffizi_crossold.hdr",{ 0x24,0xce,0x9d,0x81,0xd7,0x3d,0xad,0x4b,0xd8,0xfe,0xfa,0xa7,0xa2,0x52,0x25,0xa4 } },

        // ?#RADIANCE header test cases
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BlueLIght_4EV_PS.hdr",{ 0x6b,0x73,0x18,0xa4,0x44,0xfc,0x66,0x7f,0x77,0x85,0xf5,0xe0,0xe1,0x2d,0xbf,0x43 } },
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BlueLIght_6EV_PS.hdr",{ 0xc4,0x42,0x33,0xe5,0x06,0x34,0xc8,0xbf,0x31,0xff,0x2d,0x51,0x8b,0x63,0x08,0xdc } },
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BreakLight_4EV_PS.hdr",{ 0xae,0x12,0x7e,0x67,0xce,0xe6,0xef,0x36,0xe2,0x6c,0x53,0x27,0xe9,0x61,0xee,0xfe } },
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BreakLight_6EV_PS.hdr",{ 0xd0,0xe2,0x9e,0xa7,0xd0,0xa2,0xe3,0x57,0xfa,0x44,0xa4,0x93,0x4e,0xb9,0x47,0x32 } },

        // ?#RGBE alternative header test cases
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BlueLIght_4EV.hdr",{ 0x6b,0x73,0x18,0xa4,0x44,0xfc,0x66,0x7f,0x77,0x85,0xf5,0xe0,0xe1,0x2d,0xbf,0x43 } },
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BlueLIght_6EV.hdr",{ 0xc4,0x42,0x33,0xe5,0x06,0x34,0xc8,0xbf,0x31,0xff,0x2d,0x51,0x8b,0x63,0x08,0xdc } },
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BreakLight_4EV.hdr",{ 0xae,0x12,0x7e,0x67,0xce,0xe6,0xef,0x36,0xe2,0x6c,0x53,0x27,0xe9,0x61,0xee,0xfe } },
        { { 1920, 1080, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BreakLight_6EV.hdr",{ 0xd0,0xe2,0x9e,0xa7,0xd0,0xa2,0xe3,0x57,0xfa,0x44,0xa4,0x93,0x4e,0xb9,0x47,0x32 } },
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
// GetMetadataFromHDRMemory/File
bool Test01()
{
    bool success = true;

    // GetMetadataFromHDRMemory is used internally to the File version, so we only need to explicitly test the File version

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

        TexMetadata metadata;
        HRESULT hr = GetMetadataFromHDRFile( szPath, metadata );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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
            ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromHDRMemory
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
            hr = LoadFromHDRMemory( blob.GetBufferPointer(), blob.GetBufferSize(), &metadata, image );

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed loading hdr from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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

                //TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );
            }

            image.Release();
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromHDRFile
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

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromHDRFile( szPath, &metadata, image );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading hdr from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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

            // TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveHDRToMemory
bool Test04()
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

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromHDRFile(szPath, &metadata, image);

        if (FAILED(hr))
        {
            success = false;
            printe("Failed loading HJDR from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else
        {
            bool pass = true;

            if (metadata.format != DXGI_FORMAT_R32G32B32A32_FLOAT)
            {
                success = false;
                pass = false;
                printe("Unexpected HDR pixel format (found %d):\n%ls\n", metadata.format, szPath);
            }

            Blob blob;
            hr = SaveToHDRMemory(*image.GetImage(0, 0, 0), blob);
            if (FAILED(hr))
            {
                success = false;
                pass = false;
                printe("Failed writing hdr to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromHDRMemory(blob.GetBufferPointer(), blob.GetBufferSize(), &metadata2, image2);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("Failed reading back written hdr to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else if (metadata.width != metadata2.width
                    || metadata.height != metadata2.height
                    || metadata.arraySize != metadata2.arraySize
                    || metadata2.mipLevels != 1
                    || metadata.dimension != metadata2.dimension
                    || DXGI_FORMAT_R32G32B32A32_FLOAT != metadata2.format)
                {
                    success = false;
                    pass = false;
                    printe("Metadata error in hdr memory readback:\n%ls\n", szPath);
                    printmeta(&metadata2);
                    printmetachk(&metadata);
                }
                else
                {
                    // TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX1.DDS", DDS_FLAGS_NONE, image2 );

                    float mse, mseV[4];
                    hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed computing hdr memory readback MSE (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else if (IsErrorTooLarge(mse, 0.001f))
                    {
                        success = false;
                        pass = false;
                        printe("MSE = %f (%f %f %f %f)... 0.f\n%ls\n", mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath);
                    }
                }
            }

            // DXGI_FORMAT_R32G32B32_FLOAT
            {
                ScratchImage image96bpp;
                hr = Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R32G32B32_FLOAT, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, image96bpp);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: Failed to create 96bpp test data\n%ls\n", szPath);
                }
                else
                {
                    assert(image96bpp.GetImage(0, 0, 0)->format == DXGI_FORMAT_R32G32B32_FLOAT);

                    Blob blob2;
                    hr = SaveToHDRMemory(*image96bpp.GetImage(0, 0, 0), blob2);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed writing hdr to memory 96bpp (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                        TexMetadata metadata2;
                        ScratchImage image2;
                        hr = LoadFromHDRMemory(blob2.GetBufferPointer(), blob2.GetBufferSize(), &metadata2, image2);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed reading back written hdr to memory 96bpp (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                        }
                        else if (metadata.width != metadata2.width
                            || metadata.height != metadata2.height
                            || metadata.arraySize != metadata2.arraySize
                            || metadata2.mipLevels != 1
                            || metadata.dimension != metadata2.dimension
                            || DXGI_FORMAT_R32G32B32A32_FLOAT != metadata2.format)
                        {
                            success = false;
                            pass = false;
                            printe("Metadata error in hdr memory readback 96bpp :\n%ls\n", szPath);
                            printmeta(&metadata2);
                            printmetachk(&metadata);
                        }
                        else
                        {
                            // TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX1.DDS", DDS_FLAGS_NONE, image2 );

                            float mse, mseV[4];
                            hr = ComputeMSE(*image96bpp.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed computing hdr memory readback MSE 96bpp (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                            }
                            else if (IsErrorTooLarge(mse, 0.001f))
                            {
                                success = false;
                                pass = false;
                                printe("96bpp MSE = %f (%f %f %f %f)... 0.f\n%ls\n", mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath);
                            }
                        }
                    }
                }
            }

            // DXGI_FORMAT_R16G16B16A16_FLOAT
            {
                ScratchImage imagehalf;
                hr = Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, imagehalf);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: Failed to create half16 test data\n%ls\n", szPath);
                }
                else
                {
                    assert(imagehalf.GetImage(0, 0, 0)->format == DXGI_FORMAT_R16G16B16A16_FLOAT);

                    Blob blob2;
                    hr = SaveToHDRMemory(*imagehalf.GetImage(0, 0, 0), blob2);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed writing hdr to memory half16 (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                        TexMetadata metadata2;
                        ScratchImage image2;
                        hr = LoadFromHDRMemory(blob2.GetBufferPointer(), blob2.GetBufferSize(), &metadata2, image2);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed reading back written hdr to memory half16 (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                        }
                        else if (metadata.width != metadata2.width
                            || metadata.height != metadata2.height
                            || metadata.arraySize != metadata2.arraySize
                            || metadata2.mipLevels != 1
                            || metadata.dimension != metadata2.dimension
                            || DXGI_FORMAT_R32G32B32A32_FLOAT != metadata2.format)
                        {
                            success = false;
                            pass = false;
                            printe("Metadata error in hdr memory readback half16 :\n%ls\n", szPath);
                            printmeta(&metadata2);
                            printmetachk(&metadata);
                        }
                        else
                        {
                            // TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX1.DDS", DDS_FLAGS_NONE, image2 );

                            float mse, mseV[4];
                            hr = ComputeMSE(*imagehalf.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed computing hdr memory readback MSE half16 (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                            }
                            else if (IsErrorTooLarge(mse, 0.001f))
                            {
                                success = false;
                                pass = false;
                                printe("half16 MSE = %f (%f %f %f %f)... 0.f\n%ls\n", mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath);
                            }
                        }
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


//-------------------------------------------------------------------------------------
// SaveHDRToFile
bool Test05()
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

        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"hdr", tempDir, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

        CreateDirectoryW(tempDir, nullptr);

        wchar_t szDestPath[MAX_PATH] = {};
        _wmakepath_s(szDestPath, MAX_PATH, nullptr, tempDir, fname, L".hdr");

        wchar_t fname2[_MAX_FNAME] = {};
        wcscpy_s(fname2, fname);
        wcscat_s(fname2, L"_96bpp");

        wchar_t szDestPath2[MAX_PATH] = {};
        _wmakepath_s(szDestPath2, MAX_PATH, nullptr, tempDir, fname2, L".hdr");

        wcscpy_s(fname2, fname);
        wcscat_s(fname2, L"_half16");

        wchar_t szDestPath3[MAX_PATH] = {};
        _wmakepath_s(szDestPath3, MAX_PATH, nullptr, tempDir, fname2, L".hdr");

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromHDRFile(szPath, &metadata, image);

        if (FAILED(hr))
        {
            success = false;
            printe("Failed loading JDR from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else
        {
            bool pass = true;

            if (metadata.format != DXGI_FORMAT_R32G32B32A32_FLOAT)
            {
                success = false;
                pass = false;
                printe("Unexpected HDR pixel format (found %d):\n%ls\n", metadata.format, szPath);
            }

            hr = SaveToHDRFile(*image.GetImage(0, 0, 0), szDestPath);
            if (FAILED(hr))
            {
                success = false;
                pass = false;
                printe("Failed writing hdr to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath);
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromHDRFile(szDestPath, &metadata2, image2);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("Failed reading back written hdr to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath);
                }
                else if (metadata.width != metadata2.width
                    || metadata.height != metadata2.height
                    || metadata.arraySize != metadata2.arraySize
                    || metadata2.mipLevels != 1
                    || metadata.dimension != metadata2.dimension
                    || DXGI_FORMAT_R32G32B32A32_FLOAT != metadata2.format)
                {
                    success = false;
                    pass = false;
                    printe("Metadata error in hdr readback:\n%ls\n", szDestPath);
                    printmeta(&metadata2);
                    printmetachk(&metadata);
                }
                else
                {
                    // TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX2.DDS", DDS_FLAGS_NONE, image2 );

                    float mse, mseV[4];
                    hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed computing hdr reloaded readback MSE (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath);
                    }
                    else if (IsErrorTooLarge(mse, 0.001f))
                    {
                        success = false;
                        pass = false;
                        printe("MSE = %f (%f %f %f %f)... 0.f\n%ls\n", mse, mseV[0], mseV[1], mseV[2], mseV[3], szDestPath);
                    }
                }
            }

            // DXGI_FORMAT_R32G32B32_FLOAT
            {
                ScratchImage image96bpp;
                hr = Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R32G32B32_FLOAT, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, image96bpp);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: Failed to create 96bpp test data\n%ls\n", szPath);
                }
                else
                {
                    assert(image96bpp.GetImage(0, 0, 0)->format == DXGI_FORMAT_R32G32B32_FLOAT);

                    hr = SaveToHDRFile(*image96bpp.GetImage(0, 0, 0), szDestPath2);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed writing hdr 96bpp to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                    }
                    else
                    {
                        TexMetadata metadata2;
                        ScratchImage image2;
                        hr = LoadFromHDRFile(szDestPath2, &metadata2, image2);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed reading back written hdr 96bpp to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                        }
                        else if (metadata.width != metadata2.width
                            || metadata.height != metadata2.height
                            || metadata.arraySize != metadata2.arraySize
                            || metadata2.mipLevels != 1
                            || metadata.dimension != metadata2.dimension
                            || DXGI_FORMAT_R32G32B32A32_FLOAT != metadata2.format)
                        {
                            success = false;
                            pass = false;
                            printe("Metadata error in hdr 96bpp readback:\n%ls\n", szDestPath2);
                            printmeta(&metadata2);
                            printmetachk(&metadata);
                        }
                        else
                        {
                            // TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX2.DDS", DDS_FLAGS_NONE, image2 );

                            float mse, mseV[4];
                            hr = ComputeMSE(*image96bpp.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed computing hdr 96bpp reloaded readback MSE (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                            }
                            else if (IsErrorTooLarge(mse, 0.001f))
                            {
                                success = false;
                                pass = false;
                                printe("96bpp MSE = %f (%f %f %f %f)... 0.f\n%ls\n", mse, mseV[0], mseV[1], mseV[2], mseV[3], szDestPath2);
                            }
                        }
                    }
                }
            }

            // DXGI_FORMAT_R16G16B16A16_FLOAT
            {
                ScratchImage imagehalf;
                hr = Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, imagehalf);
                if (FAILED(hr))
                {
                    success = false;
                    pass = false;
                    printe("ERROR: Failed to create half16 test data\n%ls\n", szPath);
                }
                else
                {
                    assert(imagehalf.GetImage(0, 0, 0)->format == DXGI_FORMAT_R16G16B16A16_FLOAT);

                    hr = SaveToHDRFile(*imagehalf.GetImage(0, 0, 0), szDestPath3);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed writing hdr half16 to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                    }
                    else
                    {
                        TexMetadata metadata2;
                        ScratchImage image2;
                        hr = LoadFromHDRFile(szDestPath3, &metadata2, image2);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed reading back written hdr half16 to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                        }
                        else if (metadata.width != metadata2.width
                            || metadata.height != metadata2.height
                            || metadata.arraySize != metadata2.arraySize
                            || metadata2.mipLevels != 1
                            || metadata.dimension != metadata2.dimension
                            || DXGI_FORMAT_R32G32B32A32_FLOAT != metadata2.format)
                        {
                            success = false;
                            pass = false;
                            printe("Metadata error in hdr half16 readback:\n%ls\n", szDestPath2);
                            printmeta(&metadata2);
                            printmetachk(&metadata);
                        }
                        else
                        {
                            // TESTTEST-SaveScratchImage( L"C:\\Temp\\XXX2.DDS", DDS_FLAGS_NONE, image2 );

                            float mse, mseV[4];
                            hr = ComputeMSE(*imagehalf.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed computing hdr half16 reloaded readback MSE (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                            }
                            else if (IsErrorTooLarge(mse, 0.001f))
                            {
                                success = false;
                                pass = false;
                                printe("half16 MSE = %f (%f %f %f %f)... 0.f\n%ls\n", mse, mseV[0], mseV[1], mseV[2], mseV[3], szDestPath2);
                            }
                        }
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


//-------------------------------------------------------------------------------------
// Fuzz
bool Test06()
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

            bool ishdr = false;
            {
                wchar_t ext[_MAX_EXT];
                wchar_t fname[_MAX_FNAME];
                _wsplitpath_s(findData.cFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

                ishdr = (_wcsicmp(ext, L".hdr") == 0);
            }

            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");

            // memory
            {
                Blob blob;
                HRESULT hr = LoadBlobFromFile(szPath, blob);
                if (hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE))
                {
                    if (FAILED(hr))
                    {
                        success = false;
                        printe("Failed getting raw file data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else
                    {
                        TexMetadata metadata;
                        ScratchImage image;
                        hr = LoadFromHDRMemory(blob.GetBufferPointer(), blob.GetBufferSize(), &metadata, image);

                        if (FAILED(hr) && ishdr)
                        {
                            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                            {
                                success = false;
                                printe("ERROR: frommemory expected success! (%08X)\n%ls\n", static_cast<unsigned int>(hr), szPath);
                            }
                        }
                        else if (SUCCEEDED(hr) && !ishdr)
                        {
                            success = false;
                            printe("ERROR: frommemory expected failure\n%ls\n", szPath);
                        }
                    }
                }
            }

            // file
            {
                TexMetadata metadata;
                ScratchImage image;
                HRESULT hr = LoadFromHDRFile(szPath, &metadata, image);

                if (FAILED(hr) && ishdr)
                {
                    if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                    {
                        success = false;
                        printe("ERROR: fromfile expected success ! (%08X)\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                }
                else if (SUCCEEDED(hr) && !ishdr)
                {
                    success = false;
                    printe("ERROR: fromfile expected failure\n%ls\n", szPath);
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
