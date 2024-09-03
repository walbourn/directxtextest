//-------------------------------------------------------------------------------------
// png.cpp
//
// Test for Auxiliary implementation of PNG using LIBPNG directly instead of WIC.
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"

#include "scoped.h"
#include "DirectXTexPNG.h"

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
//basi0g01.png
//basi0g02.png
//basi0g04.png
//basi0g08.png
//basi0g16.png
//basi2c08.png
//basi2c16.png
//basi3p01.png
//basi3p02.png
//basi3p04.png
//basi3p08.png
//basi4a08.png
//basi4a16.png
//basi6a08.png
//basi6a16.png
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G01.PNG", { 0xec,0x0f,0x63,0xf3,0xee,0xd0,0x6b,0x8f,0xf1,0x9b,0xbc,0x85,0x83,0x78,0x06,0xbb } },
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G02.PNG", { 0x0e, 0xd9, 0x17, 0x65, 0xa5, 0xc7, 0xe9, 0xd7, 0x99, 0xa7, 0xa9, 0xbb, 0xc5, 0x8e, 0x26, 0xa6 } },
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G04.PNG", { 0xb8, 0xfe, 0x64, 0xf7, 0xcd, 0x3b, 0x78, 0x38, 0xaf, 0x3d, 0xae, 0xcf, 0x0d, 0xd1, 0x12, 0x9e } },
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G08.PNG", { 0x2e, 0x13, 0xa2, 0x50, 0xfc, 0xed, 0x46, 0x97, 0xf4, 0x85, 0x41, 0x17, 0x3d, 0x46, 0xdc, 0x76 } },
    { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G16.PNG", { 0x06,0x66,0x19,0x2b,0x33,0x05,0x13,0x19,0x81,0xa7,0xf7,0x47,0x5c,0x81,0x13,0xb9 } },
    { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C08.PNG", { 0x56,0x08,0x18,0x52,0x3c,0xea,0x7a,0x75,0x23,0xd1,0x22,0xbf,0x10,0x2a,0xbb,0xe3 } },
    { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C16.PNG", { 0x27,0xd2,0x2e,0xa5,0xbc,0x5f,0xd3,0x9c,0xf1,0x5b,0xf7,0xac,0x6d,0x39,0x88,0x2b } },
    { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P01.PNG", { 0x45,0x42,0xdf,0x5d,0x3f,0xc0,0xf0,0x44,0xf5,0x40,0xb3,0x27,0x75,0x4c,0x96,0x48 } },
    { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P02.PNG", { 0x56,0x76,0xee,0xa4,0xf1,0x4d,0x22,0xfd,0x9f,0x1c,0x21,0xb8,0xa8,0xc4,0xd9,0x85 } },
    { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P04.PNG", { 0x8b,0x30,0xe8,0xd2,0xc7,0x95,0x9f,0x3f,0x15,0xae,0x5a,0xda,0xc7,0x59,0xd4,0xe3 } },
    { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P08.PNG", { 0xd6,0x23,0x58,0x8d,0xe2,0x40,0x4c,0x0c,0xa9,0xb3,0xe0,0xc0,0x59,0xac,0x8d,0x46 } },
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A08.PNG", { 0xb6,0xe7,0x7d,0x4e,0x1e,0x53,0x0a,0xcc,0xbd,0x77,0xbc,0x0b,0xa7,0xb8,0xa6,0x6e } },
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A16.PNG", { 0xe4,0x94,0x8d,0xbf,0xcd,0xce,0xe7,0x66,0x58,0xc7,0xfa,0xe6,0xa3,0xf7,0x74,0x7a } },
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A08.PNG", { 0xbd,0xf3,0x18,0xd2,0xe7,0xd0,0xd7,0x85,0xf0,0xd0,0x08,0x15,0x01,0xb3,0x08,0xa5 } },
    { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A16.PNG", { 0x2f,0x67,0x4d,0xe2,0x46,0xc6,0x4d,0xbf,0xf1,0xaf,0x0c,0x8c,0x39,0xe2,0x0c,0x80 } },
//bgai4a08.png
//bgai4a16.png
//bgan6a08.png
//bgan6a16.png
//bgbn4a08.png
//bggn4a16.png
//bgwn6a08.png
//bgyn6a16.png
//ccwn2c08.png
//ccwn3p08.png
//cdfn2c08.png
//cdhn2c08.png
//cdsn2c08.png
//cdun2c08.png
//ch1n3p04.png
//ch2n3p08.png
//cm0n0g04.png
//cm7n0g04.png
//cm9n0g04.png
//cs3n2c16.png
//cs3n3p08.png
//cs5n2c08.png
//cs5n3p08.png
//cs8n2c08.png
//cs8n3p08.png
//ct0n0g04.png
//ct1n0g04.png
//cten0g04.png
//ctfn0g04.png
//ctgn0g04.png
//cthn0g04.png
//ctjn0g04.png
//ctzn0g04.png
//exif2c08.png
//f00n0g08.png
//f00n2c08.png
//f01n0g08.png
//f01n2c08.png
//f02n0g08.png
//f02n2c08.png
//f03n0g08.png
//f03n2c08.png
//f04n0g08.png
//f04n2c08.png
//f99n0g04.png
//g03n0g16.png
//g03n2c08.png
//g03n3p04.png
//g04n0g16.png
//g04n2c08.png
//g04n3p04.png
//g05n0g16.png
//g05n2c08.png
//g05n3p04.png
//g07n0g16.png
//g07n2c08.png
//g07n3p04.png
//g10n0g16.png
//g10n2c08.png
//g10n3p04.png
//g25n0g16.png
//g25n2c08.png
//g25n3p04.png
//oi1n0g16.png
//oi1n2c16.png
//oi2n0g16.png
//oi2n2c16.png
//oi4n0g16.png
//oi4n2c16.png
//oi9n0g16.png
//oi9n2c16.png
//pp0n2c16.png
//pp0n6a08.png
//ps1n0g08.png
//ps1n2c16.png
//ps2n0g08.png
//ps2n2c16.png
//s01i3p01.png
//s01n3p01.png
//s02i3p01.png
//s02n3p01.png
//s03i3p01.png
//s03n3p01.png
//s04i3p01.png
//s04n3p01.png
//s05i3p02.png
//s05n3p02.png
//s06i3p02.png
//s06n3p02.png
//s07i3p02.png
//s07n3p02.png
//s08i3p02.png
//s08n3p02.png
//s09i3p02.png
//s09n3p02.png
//s32i3p04.png
//s32n3p04.png
//s33i3p04.png
//s33n3p04.png
//s34i3p04.png
//s34n3p04.png
//s35i3p04.png
//s35n3p04.png
//s36i3p04.png
//s36n3p04.png
//s37i3p04.png
//s37n3p04.png
//s38i3p04.png
//s38n3p04.png
//s39i3p04.png
//s39n3p04.png
//s40i3p04.png
//s40n3p04.png
//tbbn0g04.png
//tbbn2c16.png
//tbbn3p08.png
//tbgn2c16.png
//tbgn3p08.png
//tbrn2c08.png
//tbwn0g16.png
//tbwn3p08.png
//tbyn3p08.png
//tm3n3p02.png
//tp0n0g08.png
//tp0n2c08.png
//tp0n3p08.png
//tp1n3p08.png
//xc1n0g08.png
//xc9n2c08.png
//xcrn0g04.png
//xcsn0g01.png
//xd0n2c08.png
//xd3n2c08.png
//xd9n2c08.png
//xdtn0g01.png
//xhdn0g08.png
//xlfn0g04.png
//xs1n0g01.png
//xs2n0g01.png
//xs4n0g01.png
//xs7n0g01.png
//z00n2c08.png
//z03n2c08.png
//z06n2c08.png
//z09n2c08.png

        // Kodak Lossless True Color Image Suite
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim01.png", { 0xce,0x40,0x54,0x04,0xe6,0xf9,0x80,0xef,0xf0,0xbc,0xea,0xcf,0x81,0xc1,0x0b,0xc7 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim02.png", { 0x4d,0x31,0x60,0x43,0x4e,0xe2,0xf7,0x5c,0x00,0xb1,0x28,0x12,0xb0,0xfa,0x26,0x0c } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim03.png", { 0xd2,0x30,0x76,0x90,0xf1,0x7a,0xf2,0x79,0x46,0xb7,0xa6,0xe2,0xd0,0x45,0x9d,0x8c } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim04.png", { 0x30,0x54,0x4a,0x7e,0x3e,0xc9,0xb5,0xad,0x3f,0xd4,0x5b,0x26,0xa2,0xfb,0xef,0xeb } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim05.png", { 0xc6,0xbf,0x7b,0xa1,0x89,0x3b,0x8d,0x0d,0x23,0x96,0x39,0xa6,0x7a,0xd5,0x14,0x56 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim06.png", { 0x37,0x93,0xc7,0xfc,0x72,0x6f,0x3c,0xff,0x8f,0xed,0xc7,0xa4,0x23,0x4f,0xe4,0xc0 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim07.png", { 0x98,0xeb,0x24,0xa6,0x7f,0x2c,0xc8,0xd2,0x4e,0xfe,0x71,0xa6,0x3a,0xc5,0x33,0xf6 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim08.png", { 0xd2,0x29,0x0a,0x9e,0xc4,0x77,0x83,0xcc,0xd4,0x52,0x8a,0xae,0x32,0x5b,0x3e,0x87 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim09.png", { 0xfc,0x34,0x4b,0x64,0x09,0x67,0xac,0xbf,0x03,0x67,0x45,0x7c,0xec,0x5b,0x31,0x50 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim10.png", { 0xce,0x20,0x56,0x84,0x14,0x85,0xed,0x27,0xa2,0xb4,0x28,0x55,0x6c,0xea,0x1f,0x7a } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim11.png", { 0x14,0xd8,0x9f,0x13,0xfb,0x6a,0x10,0xce,0xe5,0x04,0x44,0x40,0x4b,0xcd,0x42,0x1f } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim12.png", { 0x61,0x94,0x49,0x06,0xa2,0xdd,0xb7,0x7a,0x04,0x5c,0x1e,0xb0,0xff,0xdb,0x89,0xf3 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim13.png", { 0x09,0x8c,0x1c,0x0a,0x54,0x6a,0x16,0x7c,0x82,0x6c,0x1f,0x4b,0xfc,0xf5,0xcc,0x1e } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim14.png", { 0xcf,0xaf,0xaf,0x0f,0x79,0xe8,0x80,0x4a,0x0c,0x72,0xa6,0x2d,0x1e,0xd4,0xd0,0xd6 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim15.png", { 0x01,0x30,0xc9,0xb9,0xa6,0x41,0xd8,0x05,0x0d,0x91,0xeb,0x76,0xbb,0x5b,0xed,0x3d } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim16.png", { 0xc6,0xbb,0x6d,0xf6,0xcc,0xb4,0x53,0xa4,0xe6,0x4d,0x53,0x24,0xd0,0xe7,0x9d,0x51 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim17.png", { 0x18,0xfd,0xfb,0x48,0xac,0xe4,0xe0,0x4e,0x6e,0x5f,0xf7,0x13,0xc2,0xef,0xc6,0x78 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim18.png", { 0xda,0xaa,0x11,0x19,0xf4,0xb4,0xd1,0x23,0xa0,0xd7,0x5d,0xe3,0xc6,0x4d,0x72,0xf8 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim19.png", { 0xec,0x0a,0xa2,0x0d,0x03,0x77,0xb9,0x67,0x98,0xb0,0x64,0x8e,0x03,0xdd,0x6b,0x0d } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim20.png", { 0xee,0xe3,0x10,0x3c,0x5d,0xa0,0xdf,0x76,0xaf,0x90,0x9b,0xb2,0xaf,0x20,0x50,0xde } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim21.png", { 0x8b,0xae,0x9c,0x6e,0x6f,0x30,0x87,0xb2,0xaf,0x38,0x81,0x50,0x59,0x68,0xc8,0xa7 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim22.png", { 0xbb,0x85,0x32,0xeb,0x33,0x41,0xc9,0x95,0x37,0x88,0x80,0xba,0x24,0x40,0xe9,0xd3 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim23.png", { 0x7d,0xfe,0x0d,0x8c,0xfa,0xfe,0x5e,0xc9,0x02,0x52,0x9c,0x30,0x5f,0x05,0xde,0x04 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim24.png", { 0xd8,0xde,0x06,0xc9,0xdb,0xcb,0x99,0xf3,0xee,0x1c,0x60,0x63,0x2a,0x21,0x5a,0xcc } },

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

        { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"test8888.dds" },
        { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"reftexture.dds" },
        { DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"cookie.dds" },

        { DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"windowslogo_L8.dds" },
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
        HRESULT hr = GetMetadataFromPNGFile(szPath, metadata);
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
        HRESULT hr = LoadFromPNGFile(szPath, &metadata, image);
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading png from file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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
        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".png");

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

                hr = SaveToPNGFile(*image.GetImage(0, 0, 0), szDestPath);
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing png to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;

                    hr = LoadFromPNGFile(szDestPath, &metadata2, image2);
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written png to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
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
                        printe( "Metadata error in png readback:\n%ls\n", szDestPath );
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

            bool ispng = false;
            {
                wchar_t ext[_MAX_EXT];
                wchar_t fname[_MAX_FNAME];
                _wsplitpath_s(findData.cFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

                ispng = (_wcsicmp(ext, L".png") == 0);
            }

            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");

            {
                TexMetadata metadata;
                ScratchImage image;
                HRESULT hr = LoadFromPNGFile(szPath, &metadata, image);

                if (FAILED(hr) && ispng)
                {
                    if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && !second)
                    {
                        success = false;
                        printe("ERROR: fromfile expected success ! (%08X)\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                }
                else if (SUCCEEDED(hr) && !ispng)
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

            hFile.reset(safe_handle(FindFirstFileEx(L"*.png",
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
