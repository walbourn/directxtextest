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

        // PNG Test Suite sample files
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G01.PNG", { 0xec,0x0f,0x63,0xf3,0xee,0xd0,0x6b,0x8f,0xf1,0x9b,0xbc,0x85,0x83,0x78,0x06,0xbb } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G02.PNG", { 0x0e, 0xd9, 0x17, 0x65, 0xa5, 0xc7, 0xe9, 0xd7, 0x99, 0xa7, 0xa9, 0xbb, 0xc5, 0x8e, 0x26, 0xa6 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G04.PNG", { 0xb8, 0xfe, 0x64, 0xf7, 0xcd, 0x3b, 0x78, 0x38, 0xaf, 0x3d, 0xae, 0xcf, 0x0d, 0xd1, 0x12, 0x9e } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G08.PNG", { 0x2e, 0x13, 0xa2, 0x50, 0xfc, 0xed, 0x46, 0x97, 0xf4, 0x85, 0x41, 0x17, 0x3d, 0x46, 0xdc, 0x76 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G16.PNG", { 0x88,0x0f,0x06,0x0c,0x52,0x6e,0x17,0xab,0xd6,0x1e,0xd7,0x40,0x8e,0x75,0xa1,0x71 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C08.PNG", { 0x51,0x2c,0x38,0x74,0xe3,0x00,0x61,0xe6,0x23,0x73,0x9e,0x2f,0x9a,0xdc,0x4e,0xba } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C16.PNG", { 0xbc,0xdf,0x95,0x2b,0x45,0x9c,0x5f,0x7d,0xe9,0x57,0x6e,0x10,0xac,0xbe,0x98,0xb2 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P01.PNG", { 0x1b,0xa5,0x9f,0x52,0x7f,0xf2,0xcf,0xdc,0x68,0xbb,0x0c,0x34,0x87,0x86,0x2e,0x91 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P02.PNG", { 0x05,0x28,0xe9,0xac,0x36,0x52,0x52,0xa8,0xc0,0xe2,0xd9,0xce,0xd8,0xa2,0xcc,0x6b } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P04.PNG", { 0xa3,0x39,0x59,0x3b,0x0d,0x82,0x10,0x3e,0x30,0xed,0x7b,0x00,0xaf,0xd6,0x88,0x16 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P08.PNG", { 0xd3,0x6b,0xdb,0xef,0xc1,0x26,0xef,0x50,0xbd,0x57,0xd5,0x1e,0xb3,0x8f,0x2a,0xc4 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A08.PNG", { 0x87,0x07,0x55,0x10,0xc1,0x21,0x38,0xb8,0x02,0xa4,0x39,0xd0,0x45,0x2d,0x43,0x22 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A16.PNG", { 0xc2,0x3b,0x9f,0x38,0x27,0x48,0x7d,0xc2,0xfc,0x40,0x86,0xa4,0x80,0x6e,0x4a,0xe9 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A08.PNG", { 0xbd,0xf3,0x18,0xd2,0xe7,0xd0,0xd7,0x85,0xf0,0xd0,0x08,0x15,0x01,0xb3,0x08,0xa5 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A16.PNG", { 0xc2,0x25,0x74,0x83,0x53,0xf3,0xce,0xf8,0xb0,0x0b,0xa3,0x95,0x1a,0x4a,0xe2,0xc2 } },

    #ifndef BUILD_BVT_ONLY
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"g04n0g16.png", { 0x7e,0x9c,0xc7,0xb3,0x31,0x88,0x55,0x77,0x93,0xf6,0x8a,0xb1,0xb4,0x95,0x1b,0x43 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"g04n2c08.png", { 0xda,0xa8,0x56,0x1d,0x65,0xa6,0x59,0x8e,0x69,0xd6,0xdd,0xb0,0x60,0xd8,0x02,0xb7 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"g04n3p04.png", { 0x0f,0x81,0xd4,0x30,0x77,0x36,0x95,0x44,0x02,0x89,0x0d,0x72,0x03,0x24,0x4b,0xd0 } },

        { { 1, 1, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s01n3p01.png", { 0xc9,0x87,0x21,0x7b,0x78,0xdd,0x44,0x05,0x6a,0x9d,0xa5,0x8c,0xf0,0x6b,0x8c,0x7a } },
        { { 2, 2, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s02n3p01.png", { 0xe1,0xb1,0xf7,0x68,0xe5,0x0f,0x52,0x69,0xdb,0x92,0x78,0x2b,0x4a,0xd6,0x22,0x47 } },
        { { 3, 3, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s03n3p01.png", { 0xb0,0x5c,0x57,0x9e,0xb0,0x95,0xdd,0xac,0x5d,0x3b,0x30,0xe0,0x32,0x9c,0x33,0xf4 } },
        { { 4, 4, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s04n3p01.png", { 0xc2,0x68,0xbd,0x54,0xd9,0x84,0xc2,0x28,0x57,0xd4,0x50,0xe2,0x33,0x76,0x61,0x15 } },
        { { 5, 5, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s05n3p02.png", { 0xfc,0xeb,0x20,0xe2,0x61,0xcb,0x29,0xeb,0xb6,0x34,0x9b,0xc6,0xc2,0x26,0x5b,0xeb } },
        { { 6, 6, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s06n3p02.png", { 0xb5,0xc9,0x90,0x00,0x82,0xb8,0x11,0x95,0x15,0xe3,0xb0,0x06,0x34,0xa3,0x79,0xc5 } },
        { { 7, 7, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s07n3p02.png", { 0xce,0xfe,0x38,0xd2,0xa3,0x5e,0x41,0xb7,0x3b,0x62,0x70,0xa3,0x98,0xc2,0x83,0xe8 } },
        { { 8, 8, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s08n3p02.png", { 0x3f,0x0f,0xc2,0xc8,0x25,0xd2,0xfa,0xd8,0x99,0x35,0x95,0x08,0xe7,0xf6,0x45,0xe1 } },
        { { 9, 9, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s09n3p02.png", { 0x5c,0x55,0xb2,0x48,0x0d,0x62,0x3e,0xae,0x3a,0x3a,0xaa,0xc4,0x44,0xeb,0x95,0x42 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s32n3p04.png", { 0xbb,0xe6,0x3d,0x94,0x33,0x64,0x1d,0xf3,0xfc,0xd2,0xc7,0x45,0xfe,0xd8,0x9a,0x93 } },
        { { 33, 33, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s33n3p04.png", { 0x20,0x70,0x8b,0xc9,0xa6,0xff,0xa8,0xd8,0xca,0x6e,0x00,0x4e,0x1e,0x9a,0xa3,0xae } },
        { { 34, 34, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s34n3p04.png", { 0x09,0x12,0xe0,0xf9,0x72,0x24,0x05,0x7b,0x29,0x8f,0x16,0x37,0x39,0xd1,0x36,0x5f } },
        { { 35, 35, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s35n3p04.png", { 0xb4,0x6d,0x9b,0xa8,0x79,0x63,0xf5,0x26,0x62,0x4a,0x6d,0x48,0x5f,0xf6,0x46,0x5e } },
        { { 36, 36, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s36n3p04.png", { 0x65,0xe5,0x7e,0x33,0xb4,0x76,0x3a,0x3b,0x0c,0x3f,0x0f,0xa9,0x22,0x95,0x40,0x6d } },
        { { 37, 37, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s37n3p04.png", { 0xf2,0x1e,0xff,0x5c,0x07,0xa7,0x55,0x57,0x7f,0xea,0x69,0xc0,0x14,0x59,0xc6,0x5f } },
        { { 38, 38, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s38n3p04.png", { 0xf6,0x23,0x72,0x40,0xa7,0x0b,0x58,0x44,0xde,0xf0,0x40,0x6d,0xc8,0xf6,0x3b,0xbd } },
        { { 39, 39, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s39n3p04.png", { 0xce,0xb3,0xb3,0x36,0x33,0xc1,0x81,0xe1,0x3e,0xce,0xe8,0x0b,0x15,0x3f,0xb6,0x02 } },
        { { 40, 40, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"s40n3p04.png", { 0x14,0x0f,0x0d,0x2e,0xb7,0x78,0xda,0xd4,0xa1,0x87,0x1c,0x17,0x76,0x7e,0x2d,0x48 } },

        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbbn0g04.png", { 0xd3,0x5e,0xca,0xc9,0x90,0xba,0x88,0x2e,0x3d,0xa9,0x99,0x09,0xeb,0xbc,0xca,0xe9 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbbn2c16.png", { 0x90,0x35,0xa0,0x69,0xbd,0x35,0x40,0x83,0xf2,0x4f,0xfe,0xa6,0x6d,0xec,0x76,0x3d } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbbn3p08.png", { 0xc2,0x31,0x57,0x0f,0x38,0x68,0x0a,0xda,0xfa,0x98,0x7f,0x10,0xfd,0xe2,0xd3,0x02 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbgn2c16.png", { 0x90,0x35,0xa0,0x69,0xbd,0x35,0x40,0x83,0xf2,0x4f,0xfe,0xa6,0x6d,0xec,0x76,0x3d } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbgn3p08.png", {  0xc2,0x31,0x57,0x0f,0x38,0x68,0x0a,0xda,0xfa,0x98,0x7f,0x10,0xfd,0xe2,0xd3,0x02 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbrn2c08.png", { 0x76,0xd0,0xed,0x6b,0xd6,0x0b,0x9d,0xd9,0x1e,0x13,0xa1,0xe8,0xfc,0x5b,0x62,0xfc } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM , TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbwn0g16.png", { 0x4c,0xdb,0x97,0x44,0xaa,0x50,0x92,0x83,0xaf,0x6b,0x9a,0x64,0x83,0xf5,0x94,0x76 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbwn3p08.png", { 0xc2,0x31,0x57,0x0f,0x38,0x68,0x0a,0xda,0xfa,0x98,0x7f,0x10,0xfd,0xe2,0xd3,0x02 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tbyn3p08.png", { 0xc2,0x31,0x57,0x0f,0x38,0x68,0x0a,0xda,0xfa,0x98,0x7f,0x10,0xfd,0xe2,0xd3,0x02 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tm3n3p02.png", { 0xea,0x95,0x29,0xa8,0xb0,0x37,0x77,0x33,0x2f,0x7e,0x20,0x22,0xf0,0x72,0xca,0x31 } },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tp0n0g08.png", { 0x38,0x8c,0xb0,0xcd,0x5a,0x3f,0x27,0x9b,0xff,0x71,0x64,0x8e,0xb4,0x3c,0xa9,0xa0 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tp0n2c08.png", { 0xc3,0x7c,0x05,0xb6,0x92,0x90,0x96,0xc1,0x73,0x6f,0x91,0xdc,0xcb,0xe9,0x3d,0x15 } },
        { { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tp0n3p08.png", { 0x98,0x5c,0xcf,0x41,0x5d,0xe9,0x75,0x4f,0xf2,0x12,0x96,0xde,0x6c,0xf6,0x9a,0x38} },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tp1n3p08.png", { 0xc2,0x31,0x57,0x0f,0x38,0x68,0x0a,0xda,0xfa,0x98,0x7f,0x10,0xfd,0xe2,0xd3,0x02} },

        // Kodak Lossless True Color Image Suite
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim01.png", { 0x4b,0xe8,0xe3,0xd8,0x4c,0x78,0x63,0x24,0x8c,0x2a,0xa0,0x55,0x45,0x2e,0x86,0x10 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim02.png", { 0xdb,0x28,0x77,0xe0,0x34,0x59,0xb2,0x19,0x6f,0xfe,0x5a,0xf1,0xdf,0x94,0x92,0x6d } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim03.png", { 0xc5,0x70,0xe9,0x1c,0x02,0x4f,0xb9,0x9f,0x5e,0x7f,0x98,0xfc,0x0c,0x24,0x5f,0xab } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim04.png", { 0x6b,0xb2,0x41,0x1b,0x2c,0xef,0x87,0xf5,0x91,0xe3,0xf5,0x61,0x29,0x67,0x80,0x9e } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim05.png", { 0xef,0x77,0xba,0xd8,0x31,0xb7,0x7f,0x4c,0xb1,0x07,0x9b,0x0a,0x10,0xe7,0xc7,0x99 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim06.png", { 0x32,0x02,0x08,0x66,0x22,0x6b,0x2a,0x6d,0xbd,0xce,0xd5,0x9e,0x14,0xc0,0x79,0xd6 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim07.png", { 0x2c,0x03,0x9b,0x76,0x8e,0x2d,0x7d,0x41,0xb1,0xe4,0xfe,0x49,0x05,0xc4,0xfd,0x64 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim08.png", { 0x9f,0xe3,0xf1,0xda,0x7e,0xa0,0x59,0x1c,0xa1,0xec,0x67,0xa1,0x6e,0x0a,0xfb,0xbf } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim09.png", { 0xf5,0x3e,0x04,0x7d,0xca,0x5b,0xec,0xdf,0x9c,0xdf,0x7e,0x7a,0x58,0xd9,0x03,0x84 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim10.png", { 0x3f,0xaf,0x19,0x6e,0x35,0x81,0xa7,0xcd,0xe5,0xcd,0xa8,0xce,0x25,0x6c,0xb9,0xdf } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim11.png", { 0xa7,0x4a,0x73,0x0f,0xa6,0x3e,0x0e,0x8a,0x8d,0x3b,0x7f,0xca,0xa6,0x34,0x88,0x56 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim12.png", { 0x5b,0xa6,0x25,0xe8,0x84,0x6f,0x7e,0x19,0x67,0xa2,0x4b,0x2e,0x94,0xd2,0xd8,0x25 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim13.png", { 0xb5,0xe1,0x31,0x75,0xe8,0xc6,0x3f,0x65,0x2b,0x4d,0x9c,0x77,0xae,0xd6,0xf8,0x4e } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim14.png", { 0xf3,0xd7,0xbb,0x64,0x8f,0x9e,0x6e,0xd8,0xca,0xad,0x12,0x5b,0x2c,0x3a,0xd7,0xfd } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim15.png", { 0xb9,0xb4,0x11,0x5f,0x5d,0xb4,0xb7,0x74,0x48,0x0f,0x02,0x6f,0xfd,0x94,0xaf,0x13 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim16.png", { 0xf6,0x25,0x05,0x55,0x91,0x3d,0xbb,0x0b,0xe6,0xcb,0xb7,0xad,0x67,0x8f,0x47,0x17 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim17.png", { 0x6e,0xf3,0x56,0xeb,0xfa,0x93,0x5f,0x1b,0xe6,0x3b,0xdb,0xf9,0x4f,0x66,0xf9,0xcc } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim18.png", { 0x32,0x52,0xd4,0x65,0x82,0x22,0x79,0x25,0x26,0x84,0x77,0xcd,0xc0,0x48,0x7c,0xa2 } },
        { { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim19.png", { 0xa4,0x99,0xbc,0xe2,0x85,0xa6,0xb5,0x87,0x0a,0x6c,0xc3,0x68,0xa6,0xa7,0x13,0xdd } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim20.png", { 0xec,0xe0,0x08,0x3e,0x68,0x87,0x52,0xf1,0x02,0x1f,0x71,0x91,0x62,0x80,0x17,0x34 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim21.png", { 0xa3,0x09,0x47,0xa8,0x75,0x62,0x23,0xb6,0x6b,0x1a,0x1d,0xe6,0x02,0x3c,0x69,0xd8 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim22.png", { 0x92,0xfb,0xfc,0xb7,0x92,0x5f,0x6c,0xdf,0xac,0x3c,0x82,0xbb,0x57,0xcb,0x8c,0xa4 } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim23.png", { 0xf9,0xb3,0xeb,0x87,0xb3,0x04,0x13,0xa6,0xcd,0x59,0xff,0x3b,0x85,0x79,0x00,0x0a } },
        { { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim24.png", { 0x29,0xf6,0x07,0x51,0xcd,0x4c,0x31,0x8a,0x7b,0x60,0x2a,0x52,0x58,0xf6,0xd9,0xe3 } },

        #if defined(_M_X64) || defined(_M_ARM64)
        // Very large images
        { { 16384, 16384, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k.png", { 0x59,0x27,0x14,0xf9,0x22,0x6e,0x09,0xc2,0x6b,0x43,0xff,0xc3,0x98,0x4d,0x37,0x2c } },
        #endif
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
        { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MEDIA_PATH L"test8888.dds" },
        { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MEDIA_PATH L"reftexture.dds" },
        { DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"windowslogo_L8.dds" },

    #ifndef BUILD_BVT_ONLY
        { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MEDIA_PATH L"lena.dds" },
        { DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MEDIA_PATH L"cookie.dds" },

        { DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R16_UNORM, MEDIA_PATH L"windowslogo_L16.dds" },

        { DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM, MEDIA_PATH L"windowslogo_rgba16.dds" },

        // TODO: DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB

        #if defined(_M_X64) || defined(_M_ARM64)
        // Very large images
        { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MEDIA_PATH L"earth16kby16k.dds" },
        #endif
    #endif // !BUILD_BVT_ONLY
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
        HRESULT hr = GetMetadataFromPNGFile(szPath, PNG_FLAGS_NONE, metadata);
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

        // TODO: PNG_FLAGS_BGR, PNG_FLAGS_IGNORE_SRGB, PNG_FLAGS_DEFAULT_LINEAR

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        TexMetadata metadata;
        HRESULT hr = GetMetadataFromPNGFile(nullptr, PNG_FLAGS_NONE, metadata);
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
        HRESULT hr = LoadFromPNGFile(szPath, PNG_FLAGS_NONE, &metadata, image);
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

        // TODO: PNG_FLAGS_BGR, PNG_FLAGS_IGNORE_SRGB, PNG_FLAGS_DEFAULT_LINEAR

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        ScratchImage image;
        HRESULT hr = LoadFromPNGFile(nullptr, PNG_FLAGS_NONE, nullptr, image);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"png", tempDir, MAX_PATH);
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

                hr = SaveToPNGFile(*image.GetImage(0, 0, 0), PNG_FLAGS_NONE, szDestPath);
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

                    hr = LoadFromPNGFile(szDestPath, PNG_FLAGS_NONE, &metadata2, image2);
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
                    {   // Formats can vary for readback, and miplevel is going to be 1 for png images
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

                // TODO: PNG_FLAGS_FORCE_LINEAR

                hr = SaveToPNGFile(*image.GetImage(0, 0, 0), PNG_FLAGS_NONE, nullptr);
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
        HRESULT hr = SaveToPNGFile(nullin, PNG_FLAGS_NONE, nullptr);
        if (hr != E_INVALIDARG && hr != E_POINTER)
        {
            success = false;
            printe("Failed invalid arg test\n");
        }
    #pragma warning(pop)
    }

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
                HRESULT hr = LoadFromPNGFile(szPath, PNG_FLAGS_NONE, &metadata, image);

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

        if (!FindNextFileW(hFile.get(), &findData))
        {
            if (second)
                break;

            hFile.reset(safe_handle(FindFirstFileExW(L"*.png",
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
