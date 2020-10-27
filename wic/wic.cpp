//-------------------------------------------------------------------------------------
// wic.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"
#include "wic.h"

#include "DirectXTexP.h"

#include <wrl/client.h>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wswitch"
#endif

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define ALTMD5(n) (n << 4)

DEFINE_GUID(GUID_WICPixelFormat24bppBGR, 0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0c);

namespace
{
    enum
    {
        FLAGS_NONE = 0x0,
        FLAGS_WIC2 = 0x1, // Requires WIC factory 2 to function
        FLAGS_GDI = 0x2, // Requires GDI to load
        FLAGS_ALTMD5_MASK = 0xff0,
        FLAGS_MQR_ORIENT = 0x1000,
    };

    struct TestMedia
    {
        DWORD options;
        TexMetadata metadata;
        const wchar_t *fname;
        uint8_t md5[16];
    };

    const TestMedia g_TestMedia[] =
    {
        // options | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename

        // Classic image processing sample files
        #ifdef _M_X64
        { ALTMD5(1), { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"fishingboat.jpg", { 0xef,0xea,0x23,0xe7,0x85,0xd2,0xd9,0x10,0x55,0x1d,0xa8,0x14,0xd4,0xaf,0x53,0xca } },
        { ALTMD5(2), { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg", { 0xef,0x7a,0x90,0x3e,0xa3,0x25,0x3d,0xf9,0x65,0x37,0x77,0x5a,0x74,0xe4,0x53,0x1b } },
        #else
        { ALTMD5(1), { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"fishingboat.jpg", { 0x25, 0x17, 0x5a, 0x16, 0xf7, 0xf8, 0x3d, 0x40, 0x9c, 0xdb, 0x82, 0x24, 0x8c, 0xf2, 0x4e, 0x83 } },
        { ALTMD5(2), { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg", { 0x35, 0x8b, 0x3f, 0xdb, 0x55, 0x97, 0x32, 0xbc, 0xa2, 0x15, 0x86, 0x78, 0xf7, 0x18, 0xf3, 0x4b } },
        #endif

        { FLAGS_MQR_ORIENT, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"baboon.tiff", { 0x5b, 0x60, 0x5c, 0xb3, 0x59, 0x6e, 0xb1, 0x05, 0xba, 0x49, 0xb9, 0xe1, 0x0f, 0xfe, 0x97, 0x5f } },
        { FLAGS_MQR_ORIENT, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"f16.tiff", { 0xcf, 0x54, 0xa2, 0xb6, 0x06, 0xf0, 0x25, 0x68, 0x03, 0x02, 0xe2, 0xb6, 0xba, 0x44, 0xdf, 0x3f } },
        { FLAGS_MQR_ORIENT, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"peppers.tiff", { 0x71, 0xe3, 0xf1, 0x0f, 0xfe, 0xb1, 0x4d, 0x00, 0x45, 0xa5, 0xfb, 0x87, 0x88, 0x7d, 0x36, 0x24 } },
        { FLAGS_NONE, { 1024, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pentagon.tiff", { 0xb9, 0x45, 0x3d, 0xa2, 0x13, 0xdc, 0xe0, 0x64, 0xe2, 0x6c, 0x5c, 0xb4, 0x08, 0xe2, 0x1e, 0xb8 } },

        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"cameraman.tif", { 0x02,0x8c,0x5b,0x08,0xb9,0xcc,0xbb,0xa7,0x22,0xf2,0xcf,0x83,0x22,0xcb,0xae,0x52 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"house.tif", { 0x6b,0xb8,0x65,0x51,0x44,0x28,0x7e,0xb6,0x6c,0x44,0xf3,0xa8,0x6b,0x17,0xad,0x68 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jetplane.tif", { 0x76,0x55,0x63,0xef,0x6e,0x48,0x5d,0x3d,0xd1,0x1c,0x57,0xbf,0x9c,0x61,0xde,0xdb } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lake.tif", { 0x50,0x45,0x83,0x6b,0x23,0xfc,0x6e,0x1c,0x58,0x45,0x6c,0xfe,0x97,0x37,0x22,0xc4 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena_color_256.tif", { 0x4d,0x7c,0xbf,0xf6,0x92,0xfe,0xe0,0xf9,0x6a,0xab,0x3d,0x34,0x1a,0x3c,0xe3,0xd2 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena_color_512.tif", { 0x44,0x14,0xcc,0x96,0x03,0xbf,0xcc,0x73,0xb0,0xaa,0xc5,0xae,0x8d,0xf7,0x0c,0xd6 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena_gray_256.tif", { 0xad,0x1f,0x9e,0xa8,0x68,0x60,0xee,0x90,0xb9,0xb6,0xea,0xb3,0x62,0xcf,0x16,0xfc } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena_gray_512.tif", { 0x3d,0x64,0xc5,0xe0,0xda,0x35,0xdf,0xd1,0x78,0x8d,0xa4,0x87,0x90,0xec,0x79,0x62 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"livingroom.tif", { 0x36,0x8f,0x48,0xb0,0x00,0x26,0x92,0xfe,0x2b,0x78,0xe7,0xeb,0x39,0xeb,0xa7,0x3c } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"mandril_color.tif", { 0x14,0x5f,0xc0,0x34,0x27,0xde,0x51,0xa7,0x99,0x2f,0xa1,0xf0,0x76,0xf8,0x18,0xdd } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"mandril_gray.tif", { 0x8c,0xfc,0x46,0x9e,0x56,0x3a,0xa5,0x28,0xd1,0x45,0xc0,0xe7,0x4a,0xb9,0xdc,0xa7 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"peppers_color.tif", { 0xbf,0xc7,0xce,0x71,0x7e,0xd4,0xfc,0xc2,0x6b,0x5f,0xfe,0x79,0x2b,0x0e,0xee,0xdf } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"peppers_gray.tif", { 0xba,0x58,0x65,0x36,0xca,0x76,0x54,0xb8,0xd1,0xa0,0x60,0x10,0x99,0xb0,0x80,0x69 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pirate.tif", { 0x0e,0xf5,0x6f,0xa4,0x2a,0xad,0x03,0xb0,0x87,0x95,0xf3,0x7d,0x84,0xc3,0xe4,0xdf } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"walkbridge.tif", { 0xf0,0xae,0x7b,0xc1,0x0d,0xf6,0x76,0x32,0x07,0x95,0xe9,0x4b,0xee,0xd4,0x4e,0xea } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"woman_blonde.tif", { 0x8c,0xc3,0xcc,0x37,0xe1,0x9d,0x56,0x63,0x03,0xe9,0xd1,0x04,0x9c,0x9e,0x69,0xe6 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"woman_darkhair.tif", { 0x69,0x53,0x35,0xad,0x23,0x0b,0xef,0x72,0x10,0xa2,0x42,0xf4,0x46,0x9e,0xa1,0x72 } },

        // PNG Test Suite sample files
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G01.PNG", { 0x73, 0x14, 0x2a, 0xf2, 0xff, 0xbb, 0x6b, 0x70, 0xa0, 0x9f, 0xa0, 0x8d, 0x99, 0xc5, 0xb8, 0x88 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G02.PNG", { 0x0e, 0xd9, 0x17, 0x65, 0xa5, 0xc7, 0xe9, 0xd7, 0x99, 0xa7, 0xa9, 0xbb, 0xc5, 0x8e, 0x26, 0xa6 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G04.PNG", { 0xb8, 0xfe, 0x64, 0xf7, 0xcd, 0x3b, 0x78, 0x38, 0xaf, 0x3d, 0xae, 0xcf, 0x0d, 0xd1, 0x12, 0x9e } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G08.PNG", { 0x2e, 0x13, 0xa2, 0x50, 0xfc, 0xed, 0x46, 0x97, 0xf4, 0x85, 0x41, 0x17, 0x3d, 0x46, 0xdc, 0x76 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G16.PNG", { 0x88, 0x0f, 0x06, 0x0c, 0x52, 0x6e, 0x17, 0xab, 0xd6, 0x1e, 0xd7, 0x40, 0x8e, 0x75, 0xa1, 0x71 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C08.PNG", { 0x51, 0x2c, 0x38, 0x74, 0xe3, 0x00, 0x61, 0xe6, 0x23, 0x73, 0x9e, 0x2f, 0x9a, 0xdc, 0x4e, 0xba } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C16.PNG", { 0xbc, 0xdf, 0x95, 0x2b, 0x45, 0x9c, 0x5f, 0x7d, 0xe9, 0x57, 0x6e, 0x10, 0xac, 0xbe, 0x98, 0xb2 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P01.PNG", { 0x1b, 0xa5, 0x9f, 0x52, 0x7f, 0xf2, 0xcf, 0xdc, 0x68, 0xbb, 0x0c, 0x34, 0x87, 0x86, 0x2e, 0x91 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P02.PNG", { 0x05, 0x28, 0xe9, 0xac, 0x36, 0x52, 0x52, 0xa8, 0xc0, 0xe2, 0xd9, 0xce, 0xd8, 0xa2, 0xcc, 0x6b } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P04.PNG", { 0xa3, 0x39, 0x59, 0x3b, 0x0d, 0x82, 0x10, 0x3e, 0x30, 0xed, 0x7b, 0x00, 0xaf, 0xd6, 0x88, 0x16 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P08.PNG", { 0xd3, 0x6b, 0xdb, 0xef, 0xc1, 0x26, 0xef, 0x50, 0xbd, 0x57, 0xd5, 0x1e, 0xb3, 0x8f, 0x2a, 0xc4 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A08.PNG", { 0xe2, 0x21, 0x2e, 0xc5, 0xfa, 0x02, 0x6a, 0x41, 0x82, 0x61, 0x36, 0xe9, 0x83, 0xbf, 0x92, 0xb2 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A16.PNG", { 0x2a, 0xd0, 0x3a, 0x20, 0xbe, 0x76, 0xd8, 0xb6, 0x5b, 0x05, 0x31, 0x92, 0x3b, 0x92, 0x73, 0x76 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A08.PNG", { 0x04, 0x6e, 0xa7, 0x73, 0x99, 0xc2, 0x25, 0x9f, 0xee, 0x17, 0xf9, 0x7e, 0x44, 0xdc, 0x6a, 0x00 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A16.PNG", { 0xd5, 0x7e, 0xc1, 0x98, 0x62, 0x87, 0xa5, 0x42, 0x8d, 0xd8, 0x40, 0x5b, 0xd5, 0x95, 0x27, 0x32 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"AlphaBall.png", { 0x25, 0x35, 0x0c, 0x71, 0xea, 0x44, 0x04, 0xfb, 0xe2, 0xae, 0x2a, 0xfd, 0x8b, 0x5a, 0xea, 0xcc } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"AlphaEdge.png", { 0x02, 0x8c, 0x1b, 0x06, 0x61, 0x06, 0x12, 0x7e, 0x36, 0x6f, 0xda, 0x0b, 0x61, 0x19, 0x45, 0xf2 } },
        { FLAGS_NONE, { 480, 360, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"black817-480x360-3_5.png", { 0x36, 0x3d, 0x9e, 0x53, 0x7f, 0xf6, 0x1c, 0xba, 0x66, 0x1d, 0xee, 0x10, 0xb7, 0x9a, 0x19, 0x85 } },
        { FLAGS_NONE, { 393, 501, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"globe-scene-fish-bowl-pngcrush.png", { 0x26, 0xd3, 0x7c, 0x42, 0x37, 0x6d, 0x72, 0x5b, 0xf9, 0x86, 0xeb, 0x51, 0x05, 0xaf, 0x65, 0x91 } },
        { FLAGS_NONE, { 440, 330, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"imgcomp-440x330.png", { 0x89, 0xa8, 0x9d, 0xb7, 0xa6, 0x6d, 0x06, 0x29, 0xb3, 0xf5, 0x2f, 0x46, 0xb0, 0x76, 0x90, 0x3c } },

        // Windows BMP files
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.bmp", { 0x94, 0xb1, 0xfe, 0x8e, 0x54, 0x3f, 0xa6, 0x29, 0x2b, 0xdd, 0x84, 0x93, 0x9b, 0x9a, 0x00, 0x23 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.bmp", { 0x38, 0x97, 0x4f, 0x56, 0x61, 0x72, 0x7c, 0x11, 0xae, 0xf7, 0x82, 0xe0, 0x5a, 0x9b, 0x56, 0xd3 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-8bppGray.bmp", { 0x7a, 0x20, 0xc7, 0x73, 0xa7, 0xbb, 0x95, 0x27, 0x44, 0x7f, 0x93, 0x95, 0x25, 0x14, 0x20, 0x1e } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR555.bmp", { 0x6c, 0x0a, 0xda, 0x95, 0x0b, 0x3f, 0x61, 0x12, 0xc0, 0xb7, 0xaf, 0x6c, 0xeb, 0x52, 0x35, 0x30 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR565.bmp", { 0xfc, 0x8f, 0xcb, 0x01, 0x11, 0xc9, 0xd6, 0xa2, 0x11, 0x2c, 0xba, 0xe0, 0x8a, 0x3b, 0x91, 0x3f } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-24bppBGR.bmp", { 0x3c, 0x7e, 0xf9, 0x77, 0xa5, 0xef, 0xc7, 0xba, 0x0d, 0x15, 0x0e, 0x66, 0x09, 0x51, 0xad, 0x1f } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGRA.bmp", { 0xc6, 0x4f, 0x63, 0xd7, 0xc4, 0x67, 0x1f, 0x10, 0xc3, 0x62, 0x08, 0x35, 0x49, 0xae, 0x39, 0x2b } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"sample.bmp",{ 0xe6,0x05,0x1a,0x1e,0x2e,0x58,0xf7,0xc4,0xa2,0x45,0x2f,0x0e,0x93,0xf0,0x29,0x91 } },
        { FLAGS_NONE,{ 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"concrete.bmp",{ 0x55,0x2f,0xa8,0x76,0x2d,0x8e,0xc9,0x29,0xa4,0xb6,0x8d,0x74,0xb9,0x50,0x03,0x55 } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"seafloor.bmp",{ 0x04,0xf1,0x89,0x11,0xe3,0x0b,0x7f,0x9a,0xbf,0xa0,0xad,0x23,0x88,0x3f,0x83,0x4e } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"spheremap.bmp",{ 0xc2,0x7e,0x3b,0x54,0x91,0xa9,0x28,0xc1,0x1d,0x6d,0xf0,0xbc,0x8b,0xf4,0xeb,0xc5 } },
        { FLAGS_NONE,{ 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"stones.bmp",{ 0x85,0xde,0xfa,0xbe,0x87,0xc9,0x6d,0x18,0xc5,0x37,0x75,0xc5,0xf6,0x43,0x9c,0xb4 } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex1.bmp",{ 0x86,0x48,0x95,0x02,0x8b,0xf6,0xdd,0xe6,0xba,0x18,0xc0,0xd8,0xde,0x95,0x12,0xef } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex2.bmp",{ 0xe4,0xcc,0xba,0x46,0x6c,0x5b,0xd9,0xad,0x71,0x3f,0xed,0x40,0xd4,0x78,0xb9,0x0a } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex3.bmp",{ 0x53,0x64,0x8e,0xd1,0x62,0xde,0x87,0xd8,0x6d,0xaa,0x02,0x61,0x34,0xb0,0xaa,0x58 } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex4.bmp",{ 0xbc,0x10,0x69,0x40,0x4c,0x07,0x6f,0xb5,0x7d,0x34,0x01,0x6c,0x48,0xf1,0x7a,0xff } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex5.bmp",{ 0xa2,0x0a,0x82,0xaf,0x97,0xae,0xd1,0x8e,0x39,0xe9,0xe9,0xa3,0x58,0x68,0x7f,0xc6 } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex6.bmp",{ 0xe0,0x23,0x4e,0xbb,0xb3,0xf6,0xfe,0x81,0x0e,0x6f,0xe1,0xab,0x15,0x61,0x13,0x8a } },
        { FLAGS_NONE,{ 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex7.bmp",{ 0x00,0x4d,0xde,0x0f,0x18,0x8a,0xca,0xf3,0x33,0xe2,0x38,0x19,0x89,0xb3,0x93,0x89 } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a1r5g5b5.bmp",{ 0xe3,0x32,0x13,0xd0,0xd2,0xd1,0xfe,0xfb,0x43,0xb8,0x7e,0xe0,0x39,0x50,0x08,0x88 } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a1r5g5b5_flip.bmp",{ 0xe3,0x32,0x13,0xd0,0xd2,0xd1,0xfe,0xfb,0x43,0xb8,0x7e,0xe0,0x39,0x50,0x08,0x88 } },
        { FLAGS_GDI,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a4r4g4b4.bmp",{ 0x92,0x00,0x62,0xb5,0x43,0x5f,0xbd,0xaf,0xa4,0xd3,0xd8,0x8b,0x4e,0x5e,0x65,0x1e } },
        { FLAGS_GDI,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a4r4g4b4_flip.bmp",{ 0x92,0x00,0x62,0xb5,0x43,0x5f,0xbd,0xaf,0xa4,0xd3,0xd8,0x8b,0x4e,0x5e,0x65,0x1e } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a8r8g8b8.bmp",{ 0xc1,0x11,0xdf,0x70,0x0c,0xcf,0xd9,0x38,0x67,0x5a,0x26,0x52,0x78,0x6d,0x5f,0x3b } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a8r8g8b8_flip.bmp",{ 0xc1,0x11,0xdf,0x70,0x0c,0xcf,0xd9,0x38,0x67,0x5a,0x26,0x52,0x78,0x6d,0x5f,0x3b } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_r5g6b5.bmp",{ 0xda,0x42,0x18,0x8b,0x03,0x3b,0x68,0xda,0x93,0x8b,0xa8,0x10,0xb3,0x30,0xcd,0x1d } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_r5g6b5_flip.bmp",{ 0xda,0x42,0x18,0x8b,0x03,0x3b,0x68,0xda,0x93,0x8b,0xa8,0x10,0xb3,0x30,0xcd,0x1d } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_r8g8b8.bmp",{ 0x55,0xb2,0xc2,0x63,0xe3,0x42,0x6c,0x5c,0x66,0xac,0xe0,0xf4,0x7f,0xcb,0x46,0xd1 } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_r8g8b8_flip.bmp",{ 0x55,0xb2,0xc2,0x63,0xe3,0x42,0x6c,0x5c,0x66,0xac,0xe0,0xf4,0x7f,0xcb,0x46,0xd1 } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_r8g8b8os2.bmp",{ 0x55,0xb2,0xc2,0x63,0xe3,0x42,0x6c,0x5c,0x66,0xac,0xe0,0xf4,0x7f,0xcb,0x46,0xd1 } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_x1r5g5b5.bmp",{ 0xe3,0x32,0x13,0xd0,0xd2,0xd1,0xfe,0xfb,0x43,0xb8,0x7e,0xe0,0x39,0x50,0x08,0x88 } },
        { FLAGS_NONE,{ 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_x1r5g5b5_flip.bmp",{ 0xe3,0x32,0x13,0xd0,0xd2,0xd1,0xfe,0xfb,0x43,0xb8,0x7e,0xe0,0x39,0x50,0x08,0x88 } },
        { FLAGS_GDI,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_x4r4g4b4.bmp",{ 0x92,0x00,0x62,0xb5,0x43,0x5f,0xbd,0xaf,0xa4,0xd3,0xd8,0x8b,0x4e,0x5e,0x65,0x1e } },
        { FLAGS_GDI,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_x4r4g4b4_flip.bmp",{ 0x92,0x00,0x62,0xb5,0x43,0x5f,0xbd,0xaf,0xa4,0xd3,0xd8,0x8b,0x4e,0x5e,0x65,0x1e } },
        { FLAGS_GDI,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_x8r8g8b8.bmp",{ 0x11,0x47,0x0d,0x4f,0xfd,0x50,0x15,0x82,0x52,0x86,0xa7,0x64,0xb8,0x62,0xfc,0x55 } },
        { FLAGS_GDI,{ 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_x8r8g8b8_flip.bmp",{ 0x11,0x47,0x0d,0x4f,0xfd,0x50,0x15,0x82,0x52,0x86,0xa7,0x64,0xb8,0x62,0xfc,0x55 } },
        { FLAGS_NONE,{ 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"memorial.bmp",{ 0x8c,0xde,0x6d,0x88,0x89,0x63,0xb5,0x86,0x2c,0x07,0xa8,0xdd,0xf7,0x00,0x64,0x2b } },

        // BMP Test Suite
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-1x1.bmp",{ 0xbb,0xd8,0x22,0x61,0x55,0x35,0xef,0xc5,0x9c,0x07,0x19,0xb8,0x20,0xe0,0x6f,0xd9 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-320x240-color.bmp",{ 0x11,0x43,0xbe,0x1b,0xab,0xe1,0x70,0xaf,0x4e,0xb4,0xf3,0x28,0xbb,0x3f,0x28,0x06 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-320x240-overlappingcolor.bmp",{ 0x53,0x59,0x56,0x49,0x2e,0x1c,0x0f,0x6e,0xcd,0x32,0x84,0xd6,0x4b,0x57,0x04,0x5f } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-320x240.bmp",{ 0xaf,0x24,0x0f,0x8e,0xfb,0x04,0xa9,0x8b,0x0b,0xd6,0x07,0xd6,0x23,0x73,0x95,0xec } },
        { FLAGS_NONE,{ 321, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-321x240.bmp",{ 0x80,0x15,0x71,0x4f,0xe6,0xb4,0x95,0x6b,0x67,0x96,0x8e,0xb9,0xfe,0xfa,0x3b,0x42 } },
        { FLAGS_NONE,{ 322, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-322x240.bmp",{ 0xe2,0xdf,0xb7,0xa9,0x8f,0x91,0x6b,0xc9,0x00,0xc6,0x86,0x4c,0x0a,0x5e,0x7a,0xc3 } },
        { FLAGS_NONE,{ 323, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-323x240.bmp",{ 0x98,0xfb,0x1f,0xa4,0xf8,0x95,0xf1,0x9c,0xbc,0x6e,0x7e,0x76,0x77,0xb3,0x05,0x91 } },
        { FLAGS_NONE,{ 324, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-324x240.bmp",{ 0x6a,0xed,0x06,0xfb,0x30,0x78,0xa1,0x8a,0x1d,0x30,0x14,0x01,0x58,0x74,0x02,0x57 } },
        { FLAGS_NONE,{ 325, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-325x240.bmp",{ 0x80,0x12,0x59,0xb1,0x5b,0x86,0x4f,0x50,0x90,0xc0,0xa0,0x75,0x3b,0x33,0xac,0x6c } },
        { FLAGS_NONE,{ 326, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-326x240.bmp",{ 0x95,0x26,0xc3,0xd9,0x13,0xbe,0xfc,0x20,0x5c,0xd2,0x17,0x4f,0x3a,0x39,0x4b,0x2a } },
        { FLAGS_NONE,{ 327, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-327x240.bmp",{ 0x36,0x97,0x5a,0x72,0x17,0x72,0x2b,0x93,0xf7,0x36,0x03,0x27,0x32,0xdd,0x19,0xb1 } },
        { FLAGS_NONE,{ 328, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-328x240.bmp",{ 0xb4,0xb0,0xb8,0xe2,0x44,0x22,0xfe,0x56,0xab,0x8f,0xba,0x2d,0x40,0x8d,0x2e,0x23 } },
        { FLAGS_NONE,{ 329, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-329x240.bmp",{ 0x5f,0x04,0x54,0xfb,0x6e,0x0f,0x07,0x0b,0xab,0xe0,0xd9,0xcc,0x9d,0x8e,0x0d,0x6c } },
        { FLAGS_NONE,{ 330, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-330x240.bmp",{ 0x46,0x9d,0x42,0xce,0xd7,0x75,0x63,0x8e,0x8e,0xe6,0x26,0xe6,0x2d,0xf0,0xb8,0x31 } },
        { FLAGS_NONE,{ 331, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-331x240.bmp",{ 0xeb,0x56,0xbd,0xa1,0xea,0xdb,0xfb,0xab,0x84,0x54,0xb3,0xbc,0x3e,0xf3,0xf9,0x84 } },
        { FLAGS_NONE,{ 332, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-332x240.bmp",{ 0xb9,0xc7,0x26,0xc4,0xd1,0x49,0x8b,0xef,0xc3,0x67,0xa6,0xdd,0x7e,0xb4,0x5f,0x93 } },
        { FLAGS_NONE,{ 333, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-333x240.bmp",{ 0xca,0xb8,0x18,0x2b,0x27,0x1a,0xb8,0x0a,0x68,0x8f,0x60,0xca,0xf4,0xcc,0x26,0xbc } },
        { FLAGS_NONE,{ 334, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-334x240.bmp",{ 0x9a,0x29,0x2f,0x92,0x29,0xfd,0xa7,0xe6,0x27,0xb8,0x1e,0xa2,0x77,0x77,0x0c,0xa5 } },
        { FLAGS_NONE,{ 335, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-335x240.bmp",{ 0x7d,0x8e,0x2d,0x76,0xea,0x74,0xd0,0x4b,0x35,0xd0,0x9f,0xdf,0xac,0x70,0xb9,0x15 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"1bpp-topdown-320x240.bmp",{ 0xaf,0x24,0x0f,0x8e,0xfb,0x04,0xa9,0x8b,0x0b,0xd6,0x07,0xd6,0x23,0x73,0x95,0xec } },
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-1x1.bmp", { 0xc9,0x87,0x21,0x7b,0x78,0xdd,0x44,0x05,0x6a,0x9d,0xa5,0x8c,0xf0,0x6b,0x8c,0x7a } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-320x240.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 321, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-321x240.bmp",{ 0xd6,0x95,0x47,0xf4,0x81,0x4d,0xe5,0xc6,0xb5,0x48,0xb2,0x6a,0x9a,0x6e,0x8a,0x24 } },
        { FLAGS_NONE,{ 322, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-322x240.bmp",{ 0x11,0x21,0x3f,0x9f,0xee,0x12,0xda,0x05,0xd7,0x84,0xfb,0x52,0x0a,0x61,0xee,0xf7 } },
        { FLAGS_NONE,{ 323, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-323x240.bmp",{ 0x08,0x2d,0xb2,0xfa,0x92,0x84,0xf7,0xf6,0x64,0xff,0xeb,0xae,0x34,0x26,0x05,0xd2 } },
        { FLAGS_NONE,{ 324, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-324x240.bmp",{ 0x21,0x05,0x1c,0xc5,0x54,0x64,0xe6,0xaa,0x25,0x53,0xb8,0xcb,0x04,0xbc,0xdd,0x71 } },
        { FLAGS_NONE,{ 325, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-325x240.bmp",{ 0x40,0xe9,0x12,0xf2,0xac,0x34,0x57,0x2f,0xb0,0xc1,0x69,0x8d,0x18,0x88,0x2e,0x89 } },
        { FLAGS_NONE,{ 326, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-326x240.bmp",{ 0x5e,0xc5,0x7c,0xae,0x45,0xf9,0x7d,0x9c,0x40,0xcd,0xca,0x12,0xb0,0xd5,0x28,0x0c } },
        { FLAGS_NONE,{ 327, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-327x240.bmp",{ 0x5f,0x63,0xd7,0x80,0x20,0x90,0xba,0x73,0x75,0x97,0xb4,0x0d,0xd0,0x37,0x07,0x74 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"4bpp-topdown-320x240.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_GDI | ALTMD5(15),{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle4-absolute-320x240.bmp", { 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_GDI | ALTMD5(16),{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle4-alternate-320x240.bmp",{ 0x29,0x29,0x4d,0x3c,0xd3,0xf5,0xcc,0x7f,0x59,0xa4,0xeb,0x46,0x9a,0xaf,0x44,0xc9 } },
        { FLAGS_GDI | ALTMD5(17),{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle4-delta-320x240.bmp",{ 0xe0,0x7b,0xa0,0xae,0xfd,0x5b,0xf7,0x58,0xfb,0x29,0x27,0xc6,0xfe,0xee,0x12,0x10 } },
        { FLAGS_GDI | ALTMD5(18),{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle4-encoded-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_GDI | ALTMD5(19),{ 64000, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle8-64000x1.bmp",{ 0xd3,0xde,0x2d,0x72,0x4f,0x19,0x55,0xb4,0x2e,0xac,0xb2,0xae,0xb2,0x29,0xde,0x74 } },
        { FLAGS_GDI | ALTMD5(20),{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle8-absolute-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_GDI | ALTMD5(21),{ 160, 120, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle8-blank-160x120.bmp",{ 0x72,0xba,0x51,0x7c,0xbc,0x15,0x1c,0x75,0x46,0x04,0x33,0xbd,0x12,0x6c,0xd1,0x87 } },
        { FLAGS_GDI | ALTMD5(22),{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle8-delta-320x240.bmp",{ 0xe0,0x7b,0xa0,0xae,0xfd,0x5b,0xf7,0x58,0xfb,0x29,0x27,0xc6,0xfe,0xee,0x12,0x10 } },
        { FLAGS_GDI | ALTMD5(23),{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rle8-encoded-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-1x1.bmp", { 0xc9,0x87,0x21,0x7b,0x78,0xdd,0x44,0x05,0x6a,0x9d,0xa5,0x8c,0xf0,0x6b,0x8c,0x7a } },
        { FLAGS_NONE,{ 1, 64000, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-1x64000.bmp",{ 0xda,0x89,0x03,0x12,0x4a,0xa7,0x30,0x46,0x36,0xbf,0x9b,0x77,0x93,0xef,0xb5,0x6d } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-320x240.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 321, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-321x240.bmp",{ 0xd6,0x95,0x47,0xf4,0x81,0x4d,0xe5,0xc6,0xb5,0x48,0xb2,0x6a,0x9a,0x6e,0x8a,0x24 } },
        { FLAGS_NONE,{ 322, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-322x240.bmp",{ 0x11,0x21,0x3f,0x9f,0xee,0x12,0xda,0x05,0xd7,0x84,0xfb,0x52,0x0a,0x61,0xee,0xf7 } },
        { FLAGS_NONE,{ 323, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-323x240.bmp",{ 0x08,0x2d,0xb2,0xfa,0x92,0x84,0xf7,0xf6,0x64,0xff,0xeb,0xae,0x34,0x26,0x05,0xd2 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-colorsimportant-two.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-colorsused-zero.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"8bpp-topdown-320x240.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"555-1x1.bmp", { 0x22,0x31,0x3c,0x2c,0xe6,0xc9,0xc4,0x22,0x3a,0x2a,0x0f,0x4d,0xe7,0xac,0x40,0xb5 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"555-320x240.bmp",{ 0x18,0xd8,0x34,0xe8,0xec,0xdd,0xce,0xb4,0x13,0x61,0x31,0xaa,0x7a,0x04,0xf8,0xde } },
        { FLAGS_NONE,{ 321, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"555-321x240.bmp",{ 0xfe,0x2b,0x1c,0x82,0x65,0xae,0xad,0xfd,0x09,0x8b,0xe8,0xcd,0x9d,0x87,0xff,0xce } },
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"565-1x1.bmp",{ 0x37,0x26,0x31,0x2a,0xf6,0x2a,0xec,0x86,0xb6,0x4a,0x77,0x08,0xd5,0x75,0x17,0x87 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"565-320x240-topdown.bmp",{ 0x3a,0x0b,0x7d,0x2e,0x54,0x89,0xa7,0x3a,0x98,0x7b,0x44,0x50,0x12,0x40,0x91,0x44 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"565-320x240.bmp",{ 0x3a,0x0b,0x7d,0x2e,0x54,0x89,0xa7,0x3a,0x98,0x7b,0x44,0x50,0x12,0x40,0x91,0x44 } },
        { FLAGS_NONE,{ 321, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"565-321x240-topdown.bmp",{ 0xc7,0x02,0x87,0xf4,0x98,0xe7,0x80,0x0e,0x60,0x8b,0xb1,0xa4,0xcc,0xad,0xb8,0xf2 } },
        { FLAGS_NONE,{ 321, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"565-321x240.bmp",{ 0xc7,0x02,0x87,0xf4,0x98,0xe7,0x80,0x0e,0x60,0x8b,0xb1,0xa4,0xcc,0xad,0xb8,0xf2 } },
        { FLAGS_NONE,{ 322, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"565-322x240-topdown.bmp",{ 0xb2,0xa6,0x18,0x7b,0x67,0x8b,0xa9,0x7c,0x2f,0x03,0x0a,0x00,0xa4,0x9e,0x17,0xae } },
        { FLAGS_NONE,{ 322, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"565-322x240.bmp",{ 0xb2,0xa6,0x18,0x7b,0x67,0x8b,0xa9,0x7c,0x2f,0x03,0x0a,0x00,0xa4,0x9e,0x17,0xae } },
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"24bpp-1x1.bmp", { 0xc9,0x87,0x21,0x7b,0x78,0xdd,0x44,0x05,0x6a,0x9d,0xa5,0x8c,0xf0,0x6b,0x8c,0x7a } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"24bpp-320x240.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 321, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"24bpp-321x240.bmp",{ 0xd6,0x95,0x47,0xf4,0x81,0x4d,0xe5,0xc6,0xb5,0x48,0xb2,0x6a,0x9a,0x6e,0x8a,0x24 } },
        { FLAGS_NONE,{ 322, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"24bpp-322x240.bmp",{ 0x11,0x21,0x3f,0x9f,0xee,0x12,0xda,0x05,0xd7,0x84,0xfb,0x52,0x0a,0x61,0xee,0xf7 } },
        { FLAGS_NONE,{ 323, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"24bpp-323x240.bmp",{ 0x08,0x2d,0xb2,0xfa,0x92,0x84,0xf7,0xf6,0x64,0xff,0xeb,0xae,0x34,0x26,0x05,0xd2 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"24bpp-imagesize-zero.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"24bpp-topdown-320x240.bmp",{ 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-1x1.bmp",{ 0xdc,0x84,0xb0,0xd7,0x41,0xe5,0xbe,0xae,0x80,0x70,0x01,0x3a,0xdd,0xcc,0x8c,0x28 } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_GDI,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-101110-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_GDI,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-888-optimalpalette-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-optimalpalette-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-topdown-320x240.bmp",{ 0x72,0x8a,0xbe,0xc6,0x62,0xc6,0x01,0x49,0x54,0x35,0xef,0x3d,0xdf,0x5f,0xf2,0xbe } },
        { FLAGS_NONE,{ 1, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-1x1v5.bmp",{ 0x4b,0xcd,0x77,0x9a,0x6d,0x1c,0xb0,0x05,0xa4,0x73,0x1d,0x44,0x76,0x82,0xd4,0x0b } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bpp-320x240v5.bmp",{ 0xf3,0x6b,0x14,0xb3,0xd8,0xf2,0xc5,0x26,0x25,0xcd,0x29,0xb5,0x96,0x9f,0x8a,0xbf } },
        { FLAGS_NONE,{ 320, 240, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"spaces in  filename.bmp", { 0x7c,0x73,0xc7,0x29,0xfb,0x44,0xbf,0x64,0xe8,0x4f,0xa5,0xdc,0x18,0xee,0xf9,0x04 } },

        // LibTiff test images
        // caspian.tif, dscf0013.tif, off_l16.tif, off_luv24.tif, off_luv32.tif, and ycbcr-cat.tif are not supported by WIC
        { FLAGS_NONE, { 800, 607, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"cramps-tile.tif", { 0x8e,0xaa,0x61,0xa5,0x25,0xeb,0x7c,0xdd,0xe4,0x7f,0x99,0x17,0xf9,0xdb,0x7c,0xa1 } },
        { FLAGS_NONE, { 800, 607, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"cramps.tif", { 0x8e,0xaa,0x61,0xa5,0x25,0xeb,0x7c,0xdd,0xe4,0x7f,0x99,0x17,0xf9,0xdb,0x7c,0xa1 } },
        { FLAGS_NONE, { 1728, 1082, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"fax2d.tif", { 0x07,0x21,0x32,0xfe,0x2d,0xe0,0xb9,0x77,0xbe,0xec,0x12,0x03,0x05,0xfa,0xb8,0x0d } },
        { FLAGS_NONE, { 1728, 1103, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"g3test.tif", { 0x26,0xa2,0x61,0x61,0xb2,0xee,0xf9,0x9d,0xb6,0xcb,0xc7,0x44,0x0b,0x39,0x22,0x9c } },
        { FLAGS_NONE, { 256, 192, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jello.tif", { 0xaa,0x96,0x65,0xc8,0xff,0x75,0xa0,0xb6,0x8c,0xaf,0xfe,0xb3,0x1a,0x0f,0x9b,0x1a } },
        { FLAGS_NONE, { 664, 813, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jim___ah.tif", { 0x9a,0x5d,0x1c,0xb8,0x7c,0x55,0x8d,0xc3,0x40,0x33,0x3b,0xf5,0x3a,0x9a,0x43,0x13 } },
        { FLAGS_NONE, { 277, 339, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jim___cg.tif", { 0x32,0x1e,0x86,0x6e,0x5b,0xfb,0xf8,0x12,0xb3,0x3d,0x3f,0xb8,0xbc,0x52,0x47,0x12 } },
        { FLAGS_NONE, { 277, 339, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jim___dg.tif", { 0x26,0x46,0x93,0x55,0x1f,0x3c,0x07,0x71,0x14,0xe8,0x4e,0xef,0xb7,0x4b,0x48,0x34 } },
        { FLAGS_NONE, { 277, 339, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jim___gg.tif", { 0x26,0x46,0x93,0x55,0x1f,0x3c,0x07,0x71,0x14,0xe8,0x4e,0xef,0xb7,0x4b,0x48,0x34 } },
        { FLAGS_NONE, { 158, 118, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"ladoga.tif", { 0x91,0xdf,0x31,0x9c,0xec,0x05,0xa6,0xcc,0xb2,0xe8,0x1c,0x09,0x7e,0x30,0x3f,0x3e } },
        { FLAGS_NONE, { 601, 81, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"oxford.tif", { 0x8c,0x06,0x92,0xb6,0xe2,0x7e,0xe2,0x7c,0xeb,0x49,0x0f,0x54,0xb3,0xf9,0x07,0x97 } },
        { FLAGS_NONE, { 640, 480, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pc260001.tif", { 0x7c,0x8b,0xa9,0x06,0xf5,0xc4,0xa6,0x2d,0xcc,0x63,0xc7,0x07,0xeb,0x68,0xa8,0x61 } },
        #ifdef _M_X64
        { ALTMD5(12), { 512, 384, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"quad-jpeg.tif",{ 0x4b,0x50,0xcb,0x88,0x6e,0xbf,0x85,0xe4,0xd0,0xd9,0x02,0x1b,0x9f,0x6b,0xd4,0xe0 } },
        #else
        { ALTMD5(12), { 512, 384, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"quad-jpeg.tif",{ 0x53,0x52,0xa4,0x0d,0xe0,0x5a,0xe9,0x0d,0x83,0x6c,0xbd,0x27,0xd1,0xa6,0x3e,0x1e } },
        #endif
        { FLAGS_NONE, { 512, 384, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"quad-lzw.tif", { 0x8f,0xfe,0x10,0xbc,0xd2,0xf3,0x75,0x73,0xdc,0x25,0x3f,0xfc,0x3b,0x08,0x6a,0x58 } },
        { FLAGS_NONE, { 512, 384, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"quad-tile.tif", { 0x8f,0xfe,0x10,0xbc,0xd2,0xf3,0x75,0x73,0xdc,0x25,0x3f,0xfc,0x3b,0x08,0x6a,0x58 } },
        { FLAGS_NONE, { 160, 160, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"smallliz.tif", { 0x10,0xaa,0x1f,0x8c,0x1f,0x22,0x40,0x39,0x7e,0xcb,0x1d,0xd8,0xdb,0x52,0xb7,0x47 } },
        { FLAGS_NONE, { 256, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"strike.tif", { 0xee,0x5c,0xb4,0xc7,0x7a,0xbd,0x8e,0x88,0xcb,0x0b,0x8c,0xa0,0x51,0xc2,0xdc,0xe7 } },
        { FLAGS_NONE, { 1512, 359, 1, 2, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"text.tif", { 0x5b,0x38,0x6d,0x96,0xe5,0xf9,0xc5,0xc2,0x36,0x91,0x16,0xb9,0x14,0x94,0xdc,0xb5 } },
        { FLAGS_NONE, { 234, 213, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"zackthecat.tif",{ 0x40,0x9a,0x08,0x68,0x12,0xf6,0xd9,0xe2,0x2c,0x8e,0x4f,0x06,0x2f,0x78,0x99,0x65 } },

        #ifndef NO_WMP
        // WDP HD Photo (aka JPEG XR) files
        { FLAGS_NONE, { 690, 690, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jpeg-xr.wdp", { 0x28, 0xae, 0x74, 0x0a, 0xa3, 0x28, 0x86, 0x4d, 0x7f, 0x32, 0x88, 0xf2, 0xf1, 0xe2, 0x81, 0x39 } },
        { ALTMD5(3), { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGR101010_64x64.wdp", { 0x8a, 0x07, 0x17, 0x06, 0x64, 0xa0, 0x17, 0xee, 0x46, 0x78, 0x44, 0x07, 0xe9, 0x4e, 0x9f, 0x6e } },
        { ALTMD5(4), { 64, 64, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGRA_64x64.wdp", { 0xae, 0xb0, 0x08, 0x8c, 0x05, 0x79, 0x7d, 0xf4, 0x53, 0xec, 0xa8, 0xd4, 0x01, 0xe1, 0xc4, 0xa6 } },
        { ALTMD5(5), { 64, 64, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR565_64x64.wdp", { 0x5b, 0x2c, 0xab, 0x18, 0x5f, 0x03, 0xeb, 0x0e, 0xee, 0xe8, 0x36, 0x02, 0xa9, 0x0c, 0x28, 0xdc } },
        { ALTMD5(6), { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR555_64x64.wdp", { 0x98, 0x31, 0x18, 0x3e, 0xc1, 0x5f, 0xb6, 0xc6, 0xdf, 0x1d, 0xc9, 0xee, 0xb3, 0x4b, 0x14, 0x96 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-128bppRGBAFixedPoint.wdp", { 0x06, 0xe4, 0xfd, 0x3d, 0xa1, 0xc0, 0xda, 0x37, 0xa4, 0x03, 0x60, 0x62, 0x8e, 0x98, 0xe4, 0x9a } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-128bppRGBAFloat.wdp", { 0x3b, 0xe9, 0x52, 0x8e, 0xbe, 0xa6, 0x7a, 0x9e, 0x7c, 0x26, 0x7d, 0x19, 0x00, 0xeb, 0x33, 0x21 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-128bppRGBFloat.wdp", { 0x3b, 0xe9, 0x52, 0x8e, 0xbe, 0xa6, 0x7a, 0x9e, 0x7c, 0x26, 0x7d, 0x19, 0x00, 0xeb, 0x33, 0x21 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR555.wdp", { 0x6c, 0x0a, 0xda, 0x95, 0x0b, 0x3f, 0x61, 0x12, 0xc0, 0xb7, 0xaf, 0x6c, 0xeb, 0x52, 0x35, 0x30 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR565.wdp", { 0xfc, 0x8f, 0xcb, 0x01, 0x11, 0xc9, 0xd6, 0xa2, 0x11, 0x2c, 0xba, 0xe0, 0x8a, 0x3b, 0x91, 0x3f } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppGray.wdp", { 0x35, 0x9d, 0xaa, 0xc8, 0x30, 0xa8, 0xf7, 0x1a, 0xea, 0xed, 0xbe, 0x03, 0x91, 0x2f, 0x73, 0x55 } },
        { ALTMD5(7), { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppGrayFixedPoint.wdp", { 0xe7, 0x4b, 0x92, 0x44, 0xca, 0xf2, 0x04, 0xe3, 0xca, 0x55, 0xc2, 0x93, 0xe6, 0x66, 0x50, 0x6b } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppGrayHalf.wdp", { 0x29, 0x3d, 0x5d, 0x6d, 0xe2, 0x83, 0x11, 0xb5, 0x00, 0xa9, 0x68, 0x47, 0xb4, 0x21, 0x56, 0x4e } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-24bppBGR.wdp", { 0x3c, 0x7e, 0xf9, 0x77, 0xa5, 0xef, 0xc7, 0xba, 0x0d, 0x15, 0x0e, 0x66, 0x09, 0x51, 0xad, 0x1f } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-24bppRGB.wdp", { 0x3c, 0x7e, 0xf9, 0x77, 0xa5, 0xef, 0xc7, 0xba, 0x0d, 0x15, 0x0e, 0x66, 0x09, 0x51, 0xad, 0x1f } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGRA.wdp", { 0xc6, 0x4f, 0x63, 0xd7, 0xc4, 0x67, 0x1f, 0x10, 0xc3, 0x62, 0x08, 0x35, 0x49, 0xae, 0x39, 0x2b } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppGrayFixedPoint.wdp", { 0x5d, 0x70, 0x75, 0x4e, 0x95, 0x11, 0xd1, 0xa8, 0xcc, 0x93, 0x68, 0xaa, 0xce, 0x1c, 0xf6, 0xc0 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppGrayFloat.wdp", { 0x22, 0x2c, 0x24, 0x4b, 0xa8, 0xb4, 0xd3, 0xa7, 0x16, 0xba, 0x59, 0xf2, 0xe7, 0x00, 0x11, 0xe0 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppRGBE.wdp", { 0xd9, 0xd3, 0xda, 0x62, 0x27, 0xe9, 0x8c, 0xaa, 0xea, 0x22, 0xd7, 0x42, 0xa8, 0x69, 0x2d, 0x3b } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-48bppRGB.wdp", { 0x11, 0xf0, 0xa6, 0x35, 0x59, 0xf8, 0x41, 0x87, 0xa6, 0xb8, 0xda, 0x9e, 0xe4, 0xc4, 0xb3, 0x83 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-48bppRGBFixedPoint.wdp", { 0x77, 0x34, 0xb5, 0x7b, 0xbe, 0xbd, 0x4f, 0x8f, 0x10, 0xec, 0xbc, 0x18, 0xf0, 0xc2, 0x63, 0x2a } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-48bppRGBHalf.wdp", { 0xc5, 0x10, 0x2d, 0x1e, 0x8f, 0xc0, 0x34, 0x78, 0xdd, 0xe3, 0x45, 0x73, 0xde, 0xca, 0xa6, 0xf4 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-64bppRGBA.wdp", { 0x11, 0xf0, 0xa6, 0x35, 0x59, 0xf8, 0x41, 0x87, 0xa6, 0xb8, 0xda, 0x9e, 0xe4, 0xc4, 0xb3, 0x83 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-64bppRGBAFixedPoint.wdp", { 0x77, 0x34, 0xb5, 0x7b, 0xbe, 0xbd, 0x4f, 0x8f, 0x10, 0xec, 0xbc, 0x18, 0xf0, 0xc2, 0x63, 0x2a } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-64bppRGBAHalf.wdp", { 0xc5, 0x10, 0x2d, 0x1e, 0x8f, 0xc0, 0x34, 0x78, 0xdd, 0xe3, 0x45, 0x73, 0xde, 0xca, 0xa6, 0xf4 } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-8bppGray.wdp", { 0x49, 0x82, 0x54, 0x1a, 0xbb, 0x71, 0x9c, 0xe8, 0xa1, 0x64, 0x05, 0x8b, 0xde, 0x0a, 0xf3, 0x53 } },
        #if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) || defined(_WIN7_PLATFORM_UPDATE)
        { FLAGS_WIC2, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-96bppRGBFixedPoint.wdp", { 0x9f, 0xb9, 0x2a, 0xa3, 0xe9, 0x27, 0x55, 0x18, 0xc6, 0x6a, 0xd6, 0x39, 0xb3, 0xe6, 0x6e, 0xa2 } },
        #else
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-96bppRGBFixedPoint.wdp", { 0x06,0xe4,0xfd,0x3d,0xa1,0xc0,0xda,0x37,0xa4,0x03,0x60,0x62,0x8e,0x98,0xe4,0x9a } },
        #endif
        { ALTMD5(14), { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-BlackWhite.wdp", { 0xfe, 0x45, 0xf9, 0xc1, 0xcb, 0xad, 0x46, 0xbe, 0x53, 0xb4, 0x4a, 0xa1, 0x05, 0x08, 0x69, 0x0b } },
        { FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"RefEncDPKCDSP_Maui-64bppRGBAFixedPoint_E_q_1_E_c_24.wdp", { 0x4d,0xbb,0xfb,0x9e,0xa3,0xdc,0x95,0xba,0xd5,0x1e,0xf2,0xa3,0x1b,0xcc,0xdf,0xce } },
        // TODO - GUID_WICPixelFormat64bppRGBHalf (wdp), GUID_WICPixelFormat128bppRGBFixedPoint (wdp)
        #endif

        // Kodak Lossless True Color Image Suite
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim01.png", { 0x4b,0xe8,0xe3,0xd8,0x4c,0x78,0x63,0x24,0x8c,0x2a,0xa0,0x55,0x45,0x2e,0x86,0x10 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim02.png", { 0xdb,0x28,0x77,0xe0,0x34,0x59,0xb2,0x19,0x6f,0xfe,0x5a,0xf1,0xdf,0x94,0x92,0x6d } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim03.png", { 0xc5,0x70,0xe9,0x1c,0x02,0x4f,0xb9,0x9f,0x5e,0x7f,0x98,0xfc,0x0c,0x24,0x5f,0xab } },
        { FLAGS_NONE, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim04.png", { 0x6b,0xb2,0x41,0x1b,0x2c,0xef,0x87,0xf5,0x91,0xe3,0xf5,0x61,0x29,0x67,0x80,0x9e } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim05.png", { 0xef,0x77,0xba,0xd8,0x31,0xb7,0x7f,0x4c,0xb1,0x07,0x9b,0x0a,0x10,0xe7,0xc7,0x99 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim06.png", { 0x32,0x02,0x08,0x66,0x22,0x6b,0x2a,0x6d,0xbd,0xce,0xd5,0x9e,0x14,0xc0,0x79,0xd6 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim07.png", { 0x2c,0x03,0x9b,0x76,0x8e,0x2d,0x7d,0x41,0xb1,0xe4,0xfe,0x49,0x05,0xc4,0xfd,0x64 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim08.png", { 0x9f,0xe3,0xf1,0xda,0x7e,0xa0,0x59,0x1c,0xa1,0xec,0x67,0xa1,0x6e,0x0a,0xfb,0xbf } },
        { FLAGS_NONE, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim09.png", { 0xf5,0x3e,0x04,0x7d,0xca,0x5b,0xec,0xdf,0x9c,0xdf,0x7e,0x7a,0x58,0xd9,0x03,0x84 } },
        { FLAGS_NONE, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim10.png", { 0x3f,0xaf,0x19,0x6e,0x35,0x81,0xa7,0xcd,0xe5,0xcd,0xa8,0xce,0x25,0x6c,0xb9,0xdf } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim11.png", { 0xa7,0x4a,0x73,0x0f,0xa6,0x3e,0x0e,0x8a,0x8d,0x3b,0x7f,0xca,0xa6,0x34,0x88,0x56 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim12.png", { 0x5b,0xa6,0x25,0xe8,0x84,0x6f,0x7e,0x19,0x67,0xa2,0x4b,0x2e,0x94,0xd2,0xd8,0x25 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim13.png", { 0xb5,0xe1,0x31,0x75,0xe8,0xc6,0x3f,0x65,0x2b,0x4d,0x9c,0x77,0xae,0xd6,0xf8,0x4e } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim14.png", { 0xf3,0xd7,0xbb,0x64,0x8f,0x9e,0x6e,0xd8,0xca,0xad,0x12,0x5b,0x2c,0x3a,0xd7,0xfd } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim15.png", { 0xb9,0xb4,0x11,0x5f,0x5d,0xb4,0xb7,0x74,0x48,0x0f,0x02,0x6f,0xfd,0x94,0xaf,0x13 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim16.png", { 0xf6,0x25,0x05,0x55,0x91,0x3d,0xbb,0x0b,0xe6,0xcb,0xb7,0xad,0x67,0x8f,0x47,0x17 } },
        { FLAGS_NONE, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim17.png", { 0x6e,0xf3,0x56,0xeb,0xfa,0x93,0x5f,0x1b,0xe6,0x3b,0xdb,0xf9,0x4f,0x66,0xf9,0xcc } },
        { FLAGS_NONE, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim18.png", { 0x32,0x52,0xd4,0x65,0x82,0x22,0x79,0x25,0x26,0x84,0x77,0xcd,0xc0,0x48,0x7c,0xa2 } },
        { FLAGS_NONE, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim19.png", { 0xa4,0x99,0xbc,0xe2,0x85,0xa6,0xb5,0x87,0x0a,0x6c,0xc3,0x68,0xa6,0xa7,0x13,0xdd } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim20.png", { 0xec,0xe0,0x08,0x3e,0x68,0x87,0x52,0xf1,0x02,0x1f,0x71,0x91,0x62,0x80,0x17,0x34 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim21.png", { 0xa3,0x09,0x47,0xa8,0x75,0x62,0x23,0xb6,0x6b,0x1a,0x1d,0xe6,0x02,0x3c,0x69,0xd8 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim22.png", { 0x92,0xfb,0xfc,0xb7,0x92,0x5f,0x6c,0xdf,0xac,0x3c,0x82,0xbb,0x57,0xcb,0x8c,0xa4 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim23.png", { 0xf9,0xb3,0xeb,0x87,0xb3,0x04,0x13,0xa6,0xcd,0x59,0xff,0x3b,0x85,0x79,0x00,0x0a } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim24.png", { 0x29,0xf6,0x07,0x51,0xcd,0x4c,0x31,0x8a,0x7b,0x60,0x2a,0x52,0x58,0xf6,0xd9,0xe3 } },

        // JPEG
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rocks.jpg",
          { 0xda,0xce,0x15,0x24,0x96,0xc5,0x59,0xde,0xc2,0xf9,0x9a,0x2a,0x1f,0xea,0x5a,0x9c } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wall.jpg",
          { 0x2c,0x79,0x80,0x35,0xc9,0xc0,0x3f,0x12,0x50,0xad,0x6f,0x60,0xa3,0xbb,0xb8,0x0f } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wood.jpg",
          { 0x31,0xc0,0xd2,0x6e,0x8d,0xdc,0x3a,0x60,0xf8,0xda,0x86,0x3d,0x05,0x1a,0x10,0x1b } },
        { FLAGS_NONE,{ 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"memorial.jpg",{ 0x05,0x4e,0x13,0x9f,0xe8,0xb8,0x38,0xc3,0xd0,0x34,0x64,0x3d,0xa3,0x5b,0x95,0xde } },

        // Multi-frame example files
        { FLAGS_NONE, { 500, 350, 1, 15, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Animated.gif", {0xa7,0xdb,0x2e,0x1e,0x35,0x50,0xb0,0xb1,0x4c,0xe7,0x37,0x2e,0xce,0x9e,0x9d,0x95} },
        { FLAGS_MQR_ORIENT | ALTMD5(13), { 512, 512, 1, 4, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"MultiFrame.tif", {0xfb,0x22,0x3e,0x83,0x13,0x99,0x17,0x64,0x5c,0x76,0xde,0x16,0xf7,0xbd,0xff,0x58} },
        { FLAGS_NONE, { 500, 500, 1, 24, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SpinCube.gif", {0x3b,0x32,0x85,0x22,0xc5,0xb9,0x48,0x4b,0xa6,0x93,0xba,0x9d,0x30,0xd4,0x6a,0x0e} },
        { FLAGS_NONE, { 400, 600, 1, 50, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"flame.gif", {0xbd,0xd5,0x4b,0x3e,0x5e,0x8f,0xba,0x17,0x10,0xff,0x9a,0x9a,0x60,0x8d,0xe1,0x77} },
        { FLAGS_NONE, { 256, 64, 1, 253, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"MeshFluid.gif", {0xda,0xd5,0x94,0xda,0x34,0x70,0x19,0xb6,0x00,0xb1,0x09,0x88,0x96,0x96,0x13,0x88} },

        // Direct2D Test Images
        { FLAGS_NONE, { 300, 227, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bppRGBAI.png", { 0x8e,0x69,0x69,0xa1,0x04,0x0b,0xf7,0x90,0xbf,0x19,0x31,0xbd,0x0e,0xa1,0x9c,0x4f } },
        { FLAGS_NONE, { 300, 227, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bppRGBAN.png", { 0x8e,0x69,0x69,0xa1,0x04,0x0b,0xf7,0x90,0xbf,0x19,0x31,0xbd,0x0e,0xa1,0x9c,0x4f } },
        { FLAGS_NONE, { 4096, 4096, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"PNG-interlaced-compression9-24bit-4096x4096.png", { 0x77,0x4e,0x80,0xbd,0x40,0xed,0xc8,0x04,0x9d,0x1d,0x38,0xb1,0xf5,0x9a,0x0f,0x4d } },
        { FLAGS_NONE, { 203, 203, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"transparent_clock.png", { 0x7b,0x33,0x31,0xb4,0x3d,0x90,0xe2,0x72,0xff,0x73,0x5c,0x4a,0xb9,0xd4,0x05,0xeb } },

        { FLAGS_NONE, { 564, 749, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Bad5.bmp", { 0x1c,0x12,0x1a,0xe8,0x80,0x9a,0x17,0xa2,0x06,0x22,0xd0,0x31,0x2e,0x30,0x52,0x4b } },
        { FLAGS_NONE, { 640, 480, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"COUGAR.BMP", { 0xb8,0x6e,0x2f,0x2a,0x92,0x2c,0xb0,0x21,0x6e,0x07,0xc1,0x61,0x54,0x2e,0x3f,0x22 } },
        { FLAGS_GDI, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rgb32table888td.bmp", { 0x7e,0xfe,0x6a,0x59,0xbc,0xdd,0xb4,0xd4,0x35,0x56,0x60,0xab,0x71,0x3b,0x39,0xb9 } },

        #ifdef _M_X64
        { FLAGS_MQR_ORIENT | ALTMD5(8), { 1024, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Dock.jpg", { 0x38,0x1e,0xd2,0x54,0x50,0xa3,0x7a,0xa9,0x06,0x48,0xce,0x78,0x91,0x77,0x3d,0xce } },
        { ALTMD5(10), { 640, 480, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"image.127839287370267572.jpg", { 0xde,0xff,0x04,0x06,0x82,0xdc,0x05,0x76,0x66,0x56,0xd0,0x8d,0xfd,0xc3,0x1b,0xaa } },
        { FLAGS_MQR_ORIENT | ALTMD5(11), { 500, 500, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"progressivehuffman.jpg", { 0x5e,0x89,0x13,0x94,0xe5,0x4b,0x58,0xa1,0x03,0xc0,0x13,0xe4,0x32,0xea,0x42,0x8c } },
        #else
        { FLAGS_MQR_ORIENT | ALTMD5(8), { 1024, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Dock.jpg", { 0xeb,0x05,0x4e,0x02,0x81,0x22,0xfd,0x3a,0x35,0xe7,0xe2,0x80,0x54,0xfd,0x85,0xf5 } },
        { ALTMD5(10), { 640, 480, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"image.127839287370267572.jpg", { 0xa6,0xfb,0x0c,0xae,0x75,0xeb,0xca,0xce,0xed,0x6d,0x88,0x6b,0x2c,0x62,0xb7,0xc4 } },
        { FLAGS_MQR_ORIENT | ALTMD5(11), { 500, 500, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"progressivehuffman.jpg", { 0x57,0x72,0x29,0x57,0x51,0x49,0xf3,0xdc,0xa1,0x4d,0x3b,0x23,0x42,0x49,0x2a,0xf8 } },
        #endif

        { FLAGS_MQR_ORIENT, { 73, 43, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"flower-minisblack-06.tif", { 0xa7,0x55,0x69,0xf5,0xde,0xbc,0x71,0xfd,0x20,0x03,0xe5,0x73,0x9f,0x45,0x2e,0x2e } },
        { FLAGS_NONE, { 760, 399, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"H3_06.TIF", { 0xd8,0x53,0x57,0x40,0x34,0x2e,0xf6,0x61,0x61,0xe1,0x08,0x38,0xb5,0x84,0x47,0x11 } },

        { FLAGS_NONE, { 1920, 1200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"abstract-test-pattern.jpg", { 0xe7,0xf0,0xda,0x83,0x80,0x05,0x5d,0x66,0x86,0xb7,0xce,0x35,0xce,0x2d,0x3c,0x61 } },
        { FLAGS_NONE, { 256, 224, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"deltae_base.png", { 0x02,0x8b,0xf7,0x6e,0xe7,0x05,0xe9,0x3a,0xc5,0xb1,0xe1,0x0b,0xd5,0xd6,0xc7,0x56 } },
        { FLAGS_NONE, { 768, 576, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Grayscale_Staircase.png", { 0xd4,0xff,0x0b,0x98,0x34,0x7b,0xe7,0x05,0x7f,0xab,0x4b,0xa0,0xe0,0xd7,0xe3,0x8a } },

        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Omega-SwanNebula-M17.png", { 0x3b,0x21,0x9b,0x2a,0x4a,0xcc,0x2e,0x4f,0x0a,0x31,0x19,0xdf,0x27,0xd0,0x4e,0xab } },

        // WIC test suite
        { FLAGS_NONE, { 550, 481, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Desert 128bpp BGRA.tif",{ 0xfc,0x2a,0xff,0x8e,0xf5,0x16,0x2f,0x3c,0x62,0x6b,0x35,0x4f,0xf8,0x78,0xd3,0xc7 } },
        { FLAGS_NONE, { 550, 481, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Desert 128bpp RGBA - converted to pRGBA.tif",{ 0xe8,0xdc,0x45,0xab,0x67,0x3a,0x13,0x96,0x1f,0x3f,0x0b,0xb6,0xdc,0x39,0xf9,0xe2 } },
        // TODO - GUID_WICPixelFormat32bppPBGRA (tiff), GUID_WICPixelFormat64bppPRGBA (tiff)

        // CMYK
        #ifndef NO_CMYK
        #ifdef _M_X64
        { FLAGS_MQR_ORIENT | ALTMD5(9),{ 162, 80, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Glee_Newscorp_rev.jpg",{ 0x2b,0xf0,0xb9,0xf7,0xa6,0x68,0x68,0x71,0x22,0x5f,0xce,0x84,0x04,0x0d,0x9f,0xab } },
        #else
        { FLAGS_MQR_ORIENT | ALTMD5(9),{ 162, 80, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Glee_Newscorp_rev.jpg",{ 0x7f,0x64,0x3e,0xe9,0x27,0xd8,0x4f,0x6b,0x59,0x76,0x28,0xdf,0xc5,0x03,0x6b,0x0d } },
        #endif
        { FLAGS_NONE, { 300, 227, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bppCMYKNoAlphaLittleEndian.tif", { 0xd4,0xea,0xf0,0x56,0xde,0xd7,0xdb,0x26,0x26,0xa7,0xcc,0x12,0xc7,0xa3,0x7c,0x11 } },
        { FLAGS_NONE, { 300, 227, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"40bppCMYKAlphaLittleEndian.tif", { 0x2e,0x64,0xeb,0x1c,0xa9,0x07,0x9a,0xe5,0xe1,0xbd,0x42,0xd5,0xf9,0x74,0x58,0x24 } },
        { FLAGS_NONE, { 300, 227, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"64bppCMYKNoAlphaLittleEndian.tif", { 0x9f,0xf4,0x7f,0x22,0x21,0x32,0x96,0xea,0xd6,0xf7,0xfa,0x69,0xa0,0xb1,0x3c,0xe5 } },
        { FLAGS_NONE, { 300, 227, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"80bppCMYKAlphaLittleEndian.tif", { 0x97,0xb2,0x26,0x20,0xfb,0x97,0xfb,0x06,0xa0,0xa4,0xba,0xdc,0x2b,0xe8,0x6b,0x35 } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Flowers_32bpp_CMYK.tif", { 0x1c,0x8d,0xa7,0x87,0xd2,0x0e,0x34,0x57,0x67,0x76,0xe4,0x68,0xe2,0x5a,0x34,0x6d } },
        { FLAGS_NONE, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Window_64bpp_CMYK.tif", { 0x04,0x13,0x8f,0xfe,0x5c,0xed,0xc2,0x66,0xa0,0x7c,0xc7,0x06,0x9d,0xc2,0x94,0xd3 } },
        { FLAGS_NONE, { 9, 7, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"TransCMYK.TIF", { 0x05,0x90,0xff,0x4b,0x17,0x05,0xc1,0xfa,0x90,0xd6,0x65,0x3a,0x71,0xb9,0x0c,0x17 } },
        #endif

        // sRGB test cases
        { FLAGS_NONE, { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D  }, MEDIA_PATH L"testpattern.png", { 0x1e,0x85,0xd6,0xef,0xe1,0xa9,0x24,0x58,0xb4,0x00,0x6d,0x12,0xf8,0x1c,0xf6,0x44 } },
        { FLAGS_NONE, { 64, 24, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p99sr.png", {0x32,0xd0,0xe2,0x92,0x5d,0x00,0x43,0xbb,0xaa,0xbd,0xa2,0x52,0xc4,0xcf,0x9f,0xdd} },
        { FLAGS_NONE, { 976, 800, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"chip_lagrange_o3_400_sigm_6p5x50.png", {0x24,0x02,0xa9,0x6b,0x21,0x17,0x30,0xbc,0x65,0x8c,0x3e,0x1d,0x29,0x8d,0x52,0x73} },
        { FLAGS_MQR_ORIENT, { 512, 683, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"cup_small.jpg", {0x2f,0x7c,0x2d,0xa6,0x8e,0x25,0x10,0x26,0xcc,0x05,0xb6,0x70,0x63,0xd2,0x44,0x7b} },
        { FLAGS_NONE, { 800, 600, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"livingrobot-rear.tiff", {0xf0,0xa0,0x56,0xc3,0xda,0x17,0x4e,0x8a,0x79,0x44,0xa3,0xa1,0x5f,0x96,0x84,0xc2} },
        { FLAGS_NONE, { 2048, 2048, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex4.png", {0x37,0x6c,0x85,0xd5,0x20,0x4e,0xab,0xbd,0xf1,0x98,0xa3,0x07,0xde,0x7b,0x2d,0xb1} },
        { FLAGS_NONE, { 21600, 10800, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.png", {0x26,0x00,0xa1,0x94,0x33,0xe7,0x36,0x46,0x03,0xb6,0xe0,0xf8,0x1c,0xfb,0x56,0x6c} },
        { FLAGS_NONE, { 16384, 8192, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuseTexture.png", {0x95,0xdc,0xb3,0xa7,0xd0,0xf8,0x63,0x04,0x75,0x1b,0x8f,0x62,0x1f,0xb1,0x44,0x12} },
        { FLAGS_NONE, { 8192, 4096, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"callisto.png", {0x92,0xf7,0x93,0xd3,0x34,0x12,0xef,0x5b,0x54,0x01,0x1a,0xb6,0x7c,0xdb,0x6b,0xa8} },

        // Additional TIF test cases
        { FLAGS_NONE,{ 2048, 1024, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SnowPano_4k_Ref.TIF",{ 0x9a,0xa3,0x6c,0xd6,0x07,0x93,0x8d,0x67,0x5b,0x54,0xb9,0xcd,0xda,0x13,0x56,0xf5 } }, // 48bppRGB

        #if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) || defined(_WIN7_PLATFORM_UPDATE)
        { FLAGS_WIC2, { 1024, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"ramps_vdm_rel.TIF", { 0xea,0x2c,0xd3,0x74,0xef,0x08,0xbf,0x65,0xfa,0x7a,0x49,0x62,0x2a,0xd3,0x5f,0x82 } },
        { FLAGS_WIC2, { 768, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"96bpp_RGB_FP.TIF", { 0x3d,0x05,0x35,0x60,0x63,0x81,0xcf,0x86,0x09,0x5f,0x8f,0xcc,0xda,0x58,0xd9,0xfe } },
        #endif

        // orientation test cases
        { FLAGS_MQR_ORIENT, { 640, 480, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"JpegWithOrientation.JPG", { 0xe0,0x0b,0x5f,0xe6,0x16,0xfc,0x58,0xf2,0x32,0xae,0x22,0xd7,0xd2,0xe7,0x68,0x6b } },

        #ifdef _M_X64
        // Very large images
        { FLAGS_NONE,{ 16384, 16384, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k.png",{ 0x59,0x27,0x14,0xf9,0x22,0x6e,0x09,0xc2,0x6b,0x43,0xff,0xc3,0x98,0x4d,0x37,0x2c } },
        #endif
    };

    //-------------------------------------------------------------------------------------

    struct AltMD5
    {
        uint8_t md5[16];
    };

    const AltMD5 g_AltMD5[] =
	{
		{ 0xc2,0x58,0x17,0x29,0x6e,0x5a,0x10,0xd9,0x1a,0x5c,0x67,0x27,0x8a,0x0b,0x18,0x9b }, // ALTMD5(1)
		{ 0x63,0xd1,0xc2,0x82,0x50,0x6e,0x38,0xc5,0x67,0x58,0xd2,0x72,0x47,0xd1,0x2b,0xc6 }, // ALTMD5(2)
		{ 0xfa,0x93,0x8c,0xb9,0x39,0x99,0x57,0xd6,0x0f,0xa0,0x53,0xdd,0x06,0x53,0xe7,0xe4 }, // ALTMD5(3)
		{ 0x8e,0xdd,0x51,0xcc,0x85,0xda,0x0b,0x77,0x1c,0xda,0xd6,0x85,0xf7,0xaa,0x5a,0xc0 }, // ALTMD5(4)
		{ 0x5d,0x8b,0xb6,0x4a,0x28,0x5d,0x86,0x6f,0x73,0x51,0x72,0x7b,0xc0,0xa9,0xd8,0x33 }, // ALTMD5(5)
		{ 0x91,0xa5,0x65,0xe7,0x9d,0x66,0xcc,0xc9,0x20,0x86,0x91,0x3f,0x60,0xac,0x2a,0xf5 }, // ALTMD5(6)
		{ 0x41,0x58,0x80,0xbe,0x82,0x4d,0x69,0x14,0x78,0xeb,0xac,0x05,0xde,0x82,0xd1,0x6f }, // ALTMD5(7)
		{ 0x71,0x31,0x34,0x10,0x14,0xa6,0x7a,0x44,0xc4,0x4c,0xc2,0x52,0x9a,0xcb,0xad,0x08 }, // ALTMD5(8)
		{ 0x7d,0x06,0xdf,0x39,0x2c,0xa1,0xad,0x92,0x16,0x53,0xc1,0x45,0xa4,0x51,0x0e,0xeb }, // ALTMD5(9)
		{ 0x87,0xc1,0x51,0xe4,0x3e,0x42,0x46,0x51,0x88,0x5b,0xf1,0x6c,0xc6,0x20,0x66,0x1e }, // ALTMD5(10)
		{ 0xe1,0x8b,0x77,0x55,0x0e,0xcb,0xf2,0xc5,0x3f,0xa4,0xd0,0xe6,0xe6,0x91,0x70,0xe4 }, // ALTMD5(11)
		{ 0xf0,0x52,0xc1,0x16,0x5f,0xa2,0xca,0xdc,0x83,0x52,0x67,0x76,0xf9,0xb8,0x8e,0xe0 }, // ALTMD5(12)
#if defined(_XBOX_ONE) && defined(_TITLE)
		{ 0x0f,0x2c,0x13,0x44,0x7b,0x03,0x05,0x3b,0x61,0xd0,0xd3,0x2a,0x8b,0x4a,0x7f,0x84 }, // ALTMD5(13)
#else
		{ 0x3f,0x00,0xb9,0x27,0xba,0xfa,0x86,0x86,0xbc,0xbf,0x9f,0x82,0xe2,0xc2,0x17,0xe7 }, // ALTMD5(13)
#endif
		{ 0x04,0xe1,0x9f,0x86,0x2e,0x96,0x95,0xf9,0x86,0x62,0xa8,0x4e,0xc3,0x6d,0xfd,0x58 }, // ALTMD5(14)
		{ 0xf3,0x6b,0x14,0xb3,0xd8,0xf2,0xc5,0x26,0x25,0xcd,0x29,0xb5,0x96,0x9f,0x8a,0xbf }, // ALTMD5(15)
		{ 0x52,0x1b,0x68,0x2e,0x94,0x14,0x6c,0x69,0x22,0xc2,0x30,0xf4,0x68,0xea,0xc9,0xaa }, // ALTMD5(16)
		{ 0x2d,0x7b,0xcc,0x6e,0x00,0x3c,0x39,0xc0,0x7a,0x65,0x0c,0x04,0xc2,0x45,0x1f,0xf9 }, // ALTMD5(17)
		{ 0xf3,0x6b,0x14,0xb3,0xd8,0xf2,0xc5,0x26,0x25,0xcd,0x29,0xb5,0x96,0x9f,0x8a,0xbf }, // ALTMD5(18)
		{ 0x1c,0xdc,0x9f,0xbf,0xd2,0x0a,0x18,0xd4,0x90,0xb0,0x8b,0x19,0x45,0xf8,0x9d,0x8c }, // ALTMD5(19)
		{ 0xf3,0x6b,0x14,0xb3,0xd8,0xf2,0xc5,0x26,0x25,0xcd,0x29,0xb5,0x96,0x9f,0x8a,0xbf }, // ALTMD5(20)
		{ 0x98,0xbc,0xf8,0xff,0x0e,0x38,0x97,0x5a,0x58,0x4d,0x87,0x1d,0x8d,0x62,0xe6,0xf4 }, // ALTMD5(21)
		{ 0x2d,0x7b,0xcc,0x6e,0x00,0x3c,0x39,0xc0,0x7a,0x65,0x0c,0x04,0xc2,0x45,0x1f,0xf9 }, // ALTMD5(22)
		{ 0xf3,0x6b,0x14,0xb3,0xd8,0xf2,0xc5,0x26,0x25,0xcd,0x29,0xb5,0x96,0x9f,0x8a,0xbf }, // ALTMD5(23)
	};

    //-------------------------------------------------------------------------------------

    struct SaveMedia
    {
        WICCodecs tcodec;
        const wchar_t *ext;
        const wchar_t *source;
    };

    const SaveMedia g_SaveMedia[] =
    {
        // target-codec | target-extension | source-filename 
        { WIC_CODEC_BMP, L".bmp", MEDIA_PATH L"test8888.dds" },
        { WIC_CODEC_JPEG, L".jpeg", MEDIA_PATH L"test8888.dds" },
        { WIC_CODEC_PNG, L".png", MEDIA_PATH L"test8888.dds" },
        { WIC_CODEC_TIFF, L".tiff", MEDIA_PATH L"test8888.dds" },
        { WIC_CODEC_BMP, L".bmp", MEDIA_PATH L"windowslogo_R5G6B5.dds" },
        { WIC_CODEC_JPEG, L".jpeg", MEDIA_PATH L"windowslogo_R5G6B5.dds" },
        { WIC_CODEC_TIFF, L".tiff", MEDIA_PATH L"windowslogo_R5G6B5.dds" },
        { WIC_CODEC_BMP, L".bmp", MEDIA_PATH L"windowslogo_rgb565.dds" },
        { WIC_CODEC_JPEG, L".jpeg", MEDIA_PATH L"windowslogo_rgb565.dds" },
        { WIC_CODEC_TIFF, L".tiff", MEDIA_PATH L"windowslogo_rgb565.dds" },
        { WIC_CODEC_BMP, L".bmp", MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { WIC_CODEC_JPEG, L".jpeg", MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { WIC_CODEC_PNG, L".png", MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { WIC_CODEC_TIFF, L".tiff", MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { WIC_CODEC_JPEG, L".jpeg", MEDIA_PATH L"windowslogo_L8.dds" },
        { WIC_CODEC_PNG, L".png", MEDIA_PATH L"windowslogo_L8.dds" },
        { WIC_CODEC_TIFF, L".tiff", MEDIA_PATH L"windowslogo_L8.dds" },
        { WIC_CODEC_JPEG, L".jpeg", MEDIA_PATH L"Maui-BlackWhite.dds" },
        { WIC_CODEC_PNG, L".png", MEDIA_PATH L"Maui-BlackWhite.dds" },
        { WIC_CODEC_TIFF, L".tiff", MEDIA_PATH L"Maui-BlackWhite.dds" },
        { WIC_CODEC_BMP, L".bmp", MEDIA_PATH L"testpattern.png" },
        { WIC_CODEC_JPEG, L".jpeg", MEDIA_PATH L"testpattern.png" },
        { WIC_CODEC_PNG, L".png", MEDIA_PATH L"testpattern.png" },
        { WIC_CODEC_TIFF, L".tiff", MEDIA_PATH L"testpattern.png" },

        #ifndef NO_WMP
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"test8888.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_R5G6B5.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_rgb565.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_L8.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_L16.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_r16f.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_r32f.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_rgba16.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_rgba16f.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_rgba32f.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"test555.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"windowslogo_A1R5G5B5.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"earth_A2R10G10B10.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"alphaedge.dds" },
        { WIC_CODEC_WMP, L".wdp", MEDIA_PATH L"testpattern.png" },
        #endif
    };
}

//-------------------------------------------------------------------------------------
extern HRESULT LoadBlobFromFile( _In_z_ const wchar_t* szFile, Blob& blob );
extern size_t DetermineFileSize( _In_z_ const wchar_t* szFile );
extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest );
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// GetWICFactory/SetWICFactory
bool WICTest::Test00()
{
    bool systemSupportsWIC2 = false;

    {
        ComPtr<IWICImagingFactory> wic;
        HRESULT hr = CoCreateInstance( CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
                                       IID_PPV_ARGS(wic.GetAddressOf()));
        if (SUCCEEDED(hr))
        {
            systemSupportsWIC2 = true;

            SetWICFactory(wic.Get());

            bool iswic2 = false;
            auto pWIC = GetWICFactory(iswic2);
            if (pWIC != wic.Get() || !iswic2)
            {
                printe("SetWICFactory failed (WIC2)\n");
                return false;
            }
        }
        else
        {
            hr = CoCreateInstance( CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(wic.GetAddressOf()));
            if (SUCCEEDED(hr))
            {
                SetWICFactory(wic.Get());

                bool iswic2 = false;
                auto pWIC = GetWICFactory(iswic2);
                if (pWIC != wic.Get() || iswic2)
                {
                    printe("SetWICFactory failed (WIC1)\n");
                    return false;
                }
            }
        }

        SetWICFactory(nullptr);
    }

    {
        bool iswic2 = false;
        auto pWIC = GetWICFactory(iswic2);

        if (!pWIC)
        {
            printe("GetWICFactory failed test\n");
            return false;
        }

        if (iswic2 != systemSupportsWIC2)
        {
            printe("GetWICFactory failed with wrong WIC type\n");
            return false;
        }
    }

    return true;
}


//-------------------------------------------------------------------------------------
// GetMetadataFromWICMemory
bool WICTest::Test01()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && !iswic2 )
            continue;

        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
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
            hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, metadata,
                                           [&](IWICMetadataQueryReader* reader)
                                           {
                                               if ( !reader )
                                               {
                                                   success = false;
                                                   printe( "Failed metadata query reader from memory:\n%ls\n", szPath );
                                               }
#if (!defined(_XBOX_ONE) || !defined(_TITLE)) && !defined(_GAMING_XBOX)
                                               else if ( g_TestMedia[index].options & FLAGS_MQR_ORIENT )
                                               {
                                                   PROPVARIANT value;
                                                   PropVariantInit( &value );

                                                   bool orient = false;
                                                   if ( SUCCEEDED(reader->GetMetadataByName( L"System.Photo.Orientation", &value ) )
                                                        && value.vt == VT_UI2 )
                                                   {
                                                       orient = true;
                                                   }

                                                   if ( !orient )
                                                   {
                                                       success = false;
                                                       printe( "Failed metadata query read of orientation from memory:\n%ls\n", szPath );
                                                   }
                                                   
                                                   PropVariantClear( &value );
                                               }
#endif
                                           } );

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed getting data from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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
                bool pass = true;

                switch( metadata.format )
                {
                case DXGI_FORMAT_B8G8R8A8_UNORM:
                case DXGI_FORMAT_B8G8R8X8_UNORM:
                    {
                        TexMetadata metadata2;
                        hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_FORCE_RGB, metadata2 );
                        if ( FAILED(hr) || metadata2.format == DXGI_FORMAT_B8G8R8A8_UNORM || metadata2.format == DXGI_FORMAT_B8G8R8X8_UNORM )
                        {
                            pass = false;
                            success = false;
                            printe( "Metadata error with WIC_FLAGS_FORCE_RGB:\n%ls\n", szPath );
                            printmeta( &metadata2 );
                        }
                    }
                    break;

                case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                    {
                        TexMetadata metadata2;
                        hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_IGNORE_SRGB, metadata2 );
                        if ( FAILED(hr)
                             || ( metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM 
                                  && metadata2.format != DXGI_FORMAT_B8G8R8A8_UNORM &&  metadata2.format !=  DXGI_FORMAT_B8G8R8X8_UNORM ) )
                        {
                            pass = false;
                            success = false;
                            printe( "Metadata error with WIC_FLAGS_IGNORE_SRGB:\n%ls\n", szPath );
                            printmeta( &metadata2 );
                        }
                    }
                    break;

                case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
                    {
                        TexMetadata metadata2;
                        hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_NO_X2_BIAS, metadata2 );
                        if ( FAILED(hr) || metadata2.format == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM )
                        {
                            pass = false;
                            success = false;
                            printe( "Metadata error with WIC_FLAGS_NO_X2_BIAS:\n%ls\n", szPath );
                            printmeta( &metadata2 );
                        }
                    }
                    break;

                case DXGI_FORMAT_B5G5R5A1_UNORM:
                case DXGI_FORMAT_B5G6R5_UNORM:
                case DXGI_FORMAT_B4G4R4A4_UNORM:
                    {
                        TexMetadata metadata2;
                        hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_NO_16BPP, metadata2 );
                        if ( FAILED(hr)
                            || metadata2.format == DXGI_FORMAT_B5G5R5A1_UNORM || metadata2.format == DXGI_FORMAT_B5G6R5_UNORM
                            || metadata2.format == DXGI_FORMAT_B4G4R4A4_UNORM
                            )
                        {
                            pass = false;
                            success = false;
                            printe( "Metadata error with WIC_FLAGS_NO_16BPP:\n%ls\n", szPath );
                            printmeta( &metadata2 );
                        }
                    }
                    break;

                case DXGI_FORMAT_R1_UNORM:
                    {
                        TexMetadata metadata2;
                        hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALL_FRAMES, metadata2 );
                        if ( FAILED(hr) || metadata2.format != DXGI_FORMAT_R8_UNORM )
                        {
                            pass = false;
                            success = false;
                            printe( "Metadata error without WIC_FLAGS_ALLOW_MONO:\n%ls\n", szPath );
                            printmeta( &metadata2 );
                        }
                    }
                    break;
                }

                if ( metadata.arraySize > 1 )
                {
                    TexMetadata metadata2;
                    hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_NONE, metadata2 );

                    if ( FAILED(hr) || metadata2.arraySize != 1 )
                    {
                        pass = false;
                        success = false;
                        printe( "Metadata error without WIC_FLAGS_ALL_FRAMES:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                    }
                }

                if ( pass )
                    ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// GetMetadataFromWICFile
bool WICTest::Test02()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && !iswic2 )
            continue;

        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        HRESULT hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, metadata,
                                             [&](IWICMetadataQueryReader* reader)
                                             {
                                                if ( !reader )
                                                {
                                                    success = false;
                                                    printe( "Failed metadata query reader from file:\n%ls\n", szPath );
                                                }
#if (!defined(_XBOX_ONE) || !defined(_TITLE)) && !defined(_GAMING_XBOX)
                                                else if ( g_TestMedia[index].options & FLAGS_MQR_ORIENT )
                                                {
                                                    PROPVARIANT value;
                                                    PropVariantInit( &value );

                                                    bool orient = false;
                                                    if ( SUCCEEDED(reader->GetMetadataByName( L"System.Photo.Orientation", &value ) )
                                                         && value.vt == VT_UI2 )
                                                    {
                                                        orient = true;
                                                    }

                                                    if ( !orient )
                                                    {
                                                        success = false;
                                                        printe( "Failed metadata query read of orientation from file:\n%ls\n", szPath );
                                                    }
                                                   
                                                    PropVariantClear( &value );
                                                }
#endif
                                             } );

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
            bool pass = true;

            switch( metadata.format )
            {
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8X8_UNORM:
                {
                    TexMetadata metadata2;
                    hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_FORCE_RGB, metadata2 );
                    if ( FAILED(hr) || metadata2.format == DXGI_FORMAT_B8G8R8A8_UNORM || metadata2.format == DXGI_FORMAT_B8G8R8X8_UNORM )
                    {
                        pass = false;
                        success = false;
                        printe( "Metadata error with WIC_FLAGS_FORCE_RGB:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                    }
                }
                break;

            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                {
                    TexMetadata metadata2;
                    hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_IGNORE_SRGB, metadata2 );
                    if ( FAILED(hr)
                            || ( metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM
                                 && metadata2.format != DXGI_FORMAT_B8G8R8A8_UNORM &&  metadata2.format !=  DXGI_FORMAT_B8G8R8X8_UNORM ) )
                    {
                        pass = false;
                        success = false;
                        printe( "Metadata error with WIC_FLAGS_IGNORE_SRGB:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                    }
                }
                break;

            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
                {
                    TexMetadata metadata2;
                    hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_NO_X2_BIAS, metadata2 );
                    if ( FAILED(hr) || metadata2.format == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM )
                    {
                        pass = false;
                        success = false;
                        printe( "Metadata error with WIC_FLAGS_NO_X2_BIAS:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                    }
                }
                break;

            case DXGI_FORMAT_B5G5R5A1_UNORM:
            case DXGI_FORMAT_B5G6R5_UNORM:
            case DXGI_FORMAT_B4G4R4A4_UNORM:
                {
                    TexMetadata metadata2;
                    hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_NO_16BPP, metadata2 );
                    if ( FAILED(hr)
                        || metadata2.format == DXGI_FORMAT_B5G5R5A1_UNORM || metadata2.format == DXGI_FORMAT_B5G6R5_UNORM
                        || metadata2.format == DXGI_FORMAT_B4G4R4A4_UNORM
                        )
                    {
                        pass = false;
                        success = false;
                        printe( "Metadata error with WIC_FLAGS_NO_16BPP:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                    }
                }
                break;

            case DXGI_FORMAT_R1_UNORM:
                {
                    TexMetadata metadata2;
                    hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_ALL_FRAMES, metadata2 );
                    if ( FAILED(hr) || metadata2.format != DXGI_FORMAT_R8_UNORM )
                    {
                        pass = false;
                        success = false;
                        printe( "Metadata error without WIC_FLAGS_ALLOW_MONO:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                    }
                }
                break;
            }

            if ( metadata.arraySize > 1 )
            {
                TexMetadata metadata2;
                hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_NONE, metadata2 );

                if ( FAILED(hr) || metadata2.arraySize != 1 )
                {
                    pass = false;
                    success = false;
                    printe( "Metadata error without WIC_FLAGS_ALL_FRAMES:\n%ls\n", szPath );
                    printmeta( &metadata2 );
                }
            }

            if ( pass )
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromWICMemory
bool WICTest::Test03()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && !iswic2 )
            continue;

        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
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
            hr = LoadFromWICMemory(blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, &metadata, image);

            if (hr == E_NOTIMPL && (g_TestMedia[index].options & FLAGS_GDI))
            {
                continue;
            }

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed loading wic from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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
                bool pass = true;

                const uint8_t* expected2 = nullptr;
                if ( g_TestMedia[index].options & FLAGS_ALTMD5_MASK )
                {
                    // Some MD5s are different on WIC2
                    expected2 = g_AltMD5[ ((g_TestMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5;
                }

                uint8_t digest[16];
                hr = MD5Checksum( image, digest );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                }
                else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0
                          && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                          && (index != 60))
                {
                    success = false;
                    pass = false;
                    printe( "Failed comparing MD5 checksum:\n%ls\n", szPath );
                    printdigest( "computed", digest );
                    printdigest( "expected", g_TestMedia[index].md5 );
                    if ( expected2 )
                    {
                        printdigest( "expected2", expected2 );
                    }
                }

                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );

                if ( pass )
                    ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromWICFile
bool WICTest::Test04()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && !iswic2 )
            continue;

        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromWICFile( szPath, WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, &metadata, image );

        if (hr == E_NOTIMPL && (g_TestMedia[index].options & FLAGS_GDI))
        {
            continue;
        }

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading wic from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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
            bool pass = true;

            const uint8_t* expected2 = nullptr;
            if ( g_TestMedia[index].options & FLAGS_ALTMD5_MASK )
            {
                // Some MD5s are different on WIC2
                expected2 = g_AltMD5[ ((g_TestMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5;
            }

            uint8_t digest[16];
            hr = MD5Checksum( image, digest );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0
                      && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                      && (index != 60) )
            {
                success = false;
                pass = false;
                printe( "Failed comparing MD5 checksum:\n%ls\n", szPath );
                printdigest( "computed", digest );
                printdigest( "expected", g_TestMedia[index].md5 );
                if ( expected2 )
                {
                    printdigest( "expected2", expected2 );
                }
            }

            // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );

            if ( pass )
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveWICToMemory
bool WICTest::Test05()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_SaveMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_SaveMedia[index].source, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr;

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT );

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );
        }
        else
        {
            hr = LoadFromWICFile( szPath, WIC_FLAGS_NONE, &metadata, image );
        }

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading image from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else
        {
            bool pass = true;

            Blob blob;
            hr = SaveToWICMemory( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( g_SaveMedia[index].tcodec ), blob );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed writing wic (%ls) to memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath );
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO, &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed reading back written wic (%ls) from memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata.arraySize != metadata2.arraySize
                          || metadata.dimension != metadata2.dimension )
                { // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    pass = false;
                    printe( "Metadata error in wic (%ls) memory readback:\n%ls\n", g_SaveMedia[index].ext, szPath );
                    printmeta( &metadata2 );
                    printmetachk( &metadata );
                }

                size_t original = blob.GetBufferSize();

                // Providing a pixel format target 
                hr = SaveToWICMemory( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( g_SaveMedia[index].tcodec ), blob, &GUID_WICPixelFormat24bppBGR );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing wic 24bpp (%ls) to memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;
                    hr = LoadFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO, &metadata2, image2 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written wic 24bpp (%ls) from memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata.dimension != metadata2.dimension
                              || (metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM && metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) )
                    { // miplevel is going to be 1 for WIC images
                        success = false;
                        pass = false;
                        printe( "Metadata error in wic 24bpp (%ls) memory readback:\n%ls\n", g_SaveMedia[index].ext, szPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                }

                // Custom encoder properties
                if ( g_SaveMedia[index].tcodec == WIC_CODEC_TIFF )
                {
                    hr = SaveToWICMemory( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( g_SaveMedia[index].tcodec ), blob, nullptr,
                                            [&](IPropertyBag2* props)
                                            {
                                                PROPBAG2 options[2] = {};
                                                options[0].pstrName = const_cast<LPOLESTR>(L"CompressionQuality");
                                                options[1].pstrName = const_cast<LPOLESTR>(L"TiffCompressionMethod");

                                                VARIANT varValues[2];
                                                varValues[0].vt = VT_R4;
                                                varValues[0].fltVal = 0.25f;

                                                varValues[1].vt = VT_UI1;
                                                varValues[1].bVal = WICTiffCompressionNone;

                                                (void)props->Write( 2, options, varValues ); 
                                            });
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed writing wic with custom props (%ls) to memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath );
                    }
                    else
                    {
                        TexMetadata metadata2;
                        ScratchImage image2;
                        hr = LoadFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO, &metadata2, image2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed reading back written wic custom props (%ls) from memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( metadata.width != metadata2.width
                                    || metadata.height != metadata2.height
                                    || metadata.arraySize != metadata2.arraySize
                                    || metadata.dimension != metadata2.dimension )
                        { // miplevel is going to be 1 for WIC images, format changes
                            success = false;
                            pass = false;
                            printe( "Metadata error in wic custom props (%ls) memory readback:\n%ls\n", g_SaveMedia[index].ext, szPath );
                            printmeta( &metadata2 );
                            printmetachk( &metadata );
                        }
                        else if ( blob.GetBufferSize() <= original )
                        { // We are forcing uncompressed in the custom props, so new image should be strictly larger than original
                            success = false;
                            pass = false;
                            printe( "Failed wic custom props (%ls) from memory sizes unchanged (%zu ... %zu):\n%ls\n", g_SaveMedia[index].ext, blob.GetBufferSize(), original, szPath );
                        }
                    }
                }

                // WIC_FLAGS_FORCE_SRGB / WIC_FLAGS_FORCE_LINEAR
                if (g_SaveMedia[index].tcodec == WIC_CODEC_PNG
                    || g_SaveMedia[index].tcodec == WIC_CODEC_JPEG
                    || g_SaveMedia[index].tcodec == WIC_CODEC_TIFF)
                {
                    switch (metadata.format)
                    {
                    case DXGI_FORMAT_R8G8B8A8_UNORM:
                    case DXGI_FORMAT_B8G8R8A8_UNORM:
                    case DXGI_FORMAT_B8G8R8X8_UNORM:
                    {
                        hr = SaveToWICMemory(*image.GetImage(0, 0, 0), WIC_FLAGS_FORCE_SRGB, GetWICCodec(g_SaveMedia[index].tcodec), blob);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed writing wic with force srgb (%ls) to memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath);
                        }
                        else
                        {
                            TexMetadata metadata2;
                            ScratchImage image2;
                            hr = LoadFromWICMemory(blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO, &metadata2, image2);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed reading back written force srgb (%ls) from memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath);
                            }
                            else if (!IsSRGB(metadata2.format))
                            {
                                success = false;
                                pass = false;
                                printe("Metadata error in force srgb (%ls) memory readback:\n%ls\n", g_SaveMedia[index].ext, szPath);
                                printmeta(&metadata2);
                                printmetachk(&metadata);
                            }
                        }
                    }
                    break;

                    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                    {
                        hr = SaveToWICMemory(*image.GetImage(0, 0, 0), WIC_FLAGS_FORCE_LINEAR, GetWICCodec(g_SaveMedia[index].tcodec), blob);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed writing wic with force linear (%ls) to memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath);
                        }
                        else
                        {
                            TexMetadata metadata2;
                            ScratchImage image2;
                            hr = LoadFromWICMemory(blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO, &metadata2, image2);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed reading back written force linear (%ls) from memory (HRESULT %08X):\n%ls\n", g_SaveMedia[index].ext, static_cast<unsigned int>(hr), szPath);
                            }
                            else if (IsSRGB(metadata2.format))
                            {
                                success = false;
                                pass = false;
                                printe("Metadata error in force linear (%ls) memory readback:\n%ls\n", g_SaveMedia[index].ext, szPath);
                                printmeta(&metadata2);
                                printmetachk(&metadata);
                            }
                        }
                    }
                    break;
                    }
                }
            }

            if ( pass )
                ++npass;
        }

        ++ncount;
    }

    // Multiframe
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"testvol8888.dds", szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );
        ++ncount;

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading DDS from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else
        {
            Blob blob;
            hr = SaveToWICMemory( image.GetImages(), image.GetImageCount(), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_TIFF ), blob );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed writing mf wic .tiff to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALL_FRAMES, &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed reading back written mf wic .tiff to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata2.arraySize != 4
                          || metadata2.dimension != TEX_DIMENSION_TEXTURE2D	)
                {   // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    printe( "Metadata error in wic .tiff memory readback:\n%ls\n", szPath );
                    printmeta( &metadata2 );
                    printmetachk( &metadata );
                }
                else
                {
                    ++npass;
                }
            }
        }
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveWICToFile
bool WICTest::Test06()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_SaveMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_SaveMedia[index].source, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"wic", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        wchar_t szDestPath[MAX_PATH] = {};
        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, g_SaveMedia[index].ext );

        wchar_t szDestPath2[MAX_PATH] = {};
        wchar_t tname[_MAX_FNAME] = {};
        wcscpy_s( tname, fname );
        wcscat_s( tname, L"_tf" );
        _wmakepath_s( szDestPath2, MAX_PATH, nullptr, tempDir, tname, g_SaveMedia[index].ext );

        wchar_t szDestPath3[MAX_PATH] = {};
        wcscpy_s( tname, fname );
        wcscat_s( tname, L"_props" );
        _wmakepath_s( szDestPath3, MAX_PATH, nullptr, tempDir, tname, g_SaveMedia[index].ext );

        wchar_t szDestPath4[MAX_PATH] = {};
        wcscpy_s(tname, fname);
        wcscat_s(tname, L"_srgb");
        _wmakepath_s(szDestPath4, MAX_PATH, nullptr, tempDir, tname, g_SaveMedia[index].ext);

        wchar_t szDestPath5[MAX_PATH] = {};
        wcscpy_s(tname, fname);
        wcscat_s(tname, L"_linear");
        _wmakepath_s(szDestPath5, MAX_PATH, nullptr, tempDir, tname, g_SaveMedia[index].ext);

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr;

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );
        }
        else
        {
            hr = LoadFromWICFile( szPath, WIC_FLAGS_NONE, &metadata, image );
        }

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading image from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else
        {
            bool pass = true;

            hr = SaveToWICFile( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( g_SaveMedia[index].tcodec ), szDestPath );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed writing wic to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromWICFile( szDestPath, WIC_FLAGS_ALLOW_MONO, &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed reading back written wic (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata.arraySize != metadata2.arraySize
                          || metadata2.mipLevels != 1
                          || metadata.dimension != metadata2.dimension )
                {   // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    pass = false;
                    printe( "Metadata error in wic readback:\n%ls\n", szDestPath );
                    printmeta( &metadata2 );
                    printmetachk( &metadata );
                }
            }

            size_t original = DetermineFileSize( szDestPath );

            // Providing a pixel format target 
            hr = SaveToWICFile( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( g_SaveMedia[index].tcodec ), szDestPath2, &GUID_WICPixelFormat24bppBGR );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed writing wic 24bpp to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2 );
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromWICFile( szDestPath2, WIC_FLAGS_ALLOW_MONO, &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed reading back written wic 24bpp (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2 );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata.arraySize != metadata2.arraySize
                          || metadata2.mipLevels != 1
                          || metadata.dimension != metadata2.dimension
                          || (metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM && metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) )
                {   // miplevel is going to be 1 for WIC images
                    success = false;
                    pass = false;
                    printe( "Metadata error in wic 24bpp readback:\n%ls\n", szDestPath2 );
                    printmeta( &metadata2 );
                    printmetachk( &metadata );
                }
            }

            // Custom encoder properties
            if ( g_SaveMedia[index].tcodec == WIC_CODEC_TIFF )
            {
                hr = SaveToWICFile( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( g_SaveMedia[index].tcodec ), szDestPath3, nullptr,
                                        [&](IPropertyBag2* props)
                                        {
                                            PROPBAG2 options[2] = {};
                                            options[0].pstrName = const_cast<LPOLESTR>(L"CompressionQuality");
                                            options[1].pstrName = const_cast<LPOLESTR>(L"TiffCompressionMethod");

                                            VARIANT varValues[2];
                                            varValues[0].vt = VT_R4;
                                            varValues[0].fltVal = 0.25f;

                                            varValues[1].vt = VT_UI1;
                                            varValues[1].bVal = WICTiffCompressionNone;

                                            (void)props->Write( 2, options, varValues ); 
                                        });
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing wic with custom props to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath3 );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;
                    hr = LoadFromWICFile( szDestPath3, WIC_FLAGS_ALLOW_MONO, &metadata2, image2 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written wic custom props (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath3 );
                    }
                    else if ( metadata.width != metadata2.width
                                || metadata.height != metadata2.height
                                || metadata.arraySize != metadata2.arraySize
                                || metadata2.mipLevels != 1
                                || metadata.dimension != metadata2.dimension )
                    {   // miplevel is going to be 1 for WIC images, format changes
                        success = false;
                        pass = false;
                        printe( "Metadata error in wic custom props readback:\n%ls\n", szDestPath3 );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else
                    {
                        size_t uncompressed = DetermineFileSize( szDestPath3 );
                            
                        if ( uncompressed <= original )
                        { // We are forcing uncompressed in the custom props, so new image should be strictly larger than original
                            success = false;
                            pass = false;
                            printe( "Failed wic custom props sizes unchanged (%zu ... %zu):\n%ls\n", uncompressed, original, szDestPath3 );
                        }
                    }
                }
            }

            // WIC_FLAGS_FORCE_SRGB / WIC_FLAGS_FORCE_LINEAR
            if (g_SaveMedia[index].tcodec == WIC_CODEC_PNG
                || g_SaveMedia[index].tcodec == WIC_CODEC_JPEG
                || g_SaveMedia[index].tcodec == WIC_CODEC_TIFF)
            {
                switch (metadata.format)
                {
                case DXGI_FORMAT_R8G8B8A8_UNORM:
                case DXGI_FORMAT_B8G8R8A8_UNORM:
                case DXGI_FORMAT_B8G8R8X8_UNORM:
                {
                    hr = SaveToWICFile(*image.GetImage(0, 0, 0), WIC_FLAGS_FORCE_SRGB, GetWICCodec(g_SaveMedia[index].tcodec), szDestPath4);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed writing wic with force srgb to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath4);
                    }
                    else
                    {
                        TexMetadata metadata2;
                        ScratchImage image2;
                        hr = LoadFromWICFile(szDestPath4, WIC_FLAGS_ALLOW_MONO, &metadata2, image2);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed reading back written force srgb (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath4);
                        }
                        else if (!IsSRGB(metadata2.format))
                        {
                            success = false;
                            pass = false;
                            printe("Metadata error in force srgb readback:\n%ls\n", szDestPath4);
                            printmeta(&metadata2);
                            printmetachk(&metadata);
                        }
                    }
                }
                break;

                case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                {
                    hr = SaveToWICFile(*image.GetImage(0, 0, 0), WIC_FLAGS_FORCE_LINEAR, GetWICCodec(g_SaveMedia[index].tcodec), szDestPath5);
                    if (FAILED(hr))
                    {
                        success = false;
                        pass = false;
                        printe("Failed writing wic with force linear to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath5);
                    }
                    else
                    {
                        TexMetadata metadata2;
                        ScratchImage image2;
                        hr = LoadFromWICFile(szDestPath5, WIC_FLAGS_ALLOW_MONO, &metadata2, image2);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed reading back written force linear (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath5);
                        }
                        else if (IsSRGB(metadata2.format))
                        {
                            success = false;
                            pass = false;
                            printe("Metadata error in force linear readback:\n%ls\n", szDestPath5);
                            printmeta(&metadata2);
                            printmetachk(&metadata);
                        }
                    }
                }
                break;
                }
            }

            if ( pass )
                ++npass;
        }

        ++ncount;
    }

    // Multiframe
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"testvol8888.dds", szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        wchar_t szDestPath[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"wic\\multiframe.tif", szDestPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );
        ++ncount;

        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading DDS from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else
        {
            hr = SaveToWICFile( image.GetImages(), image.GetImageCount(), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_TIFF ), szDestPath );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed writing mf wic to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromWICFile( szDestPath, WIC_FLAGS_ALL_FRAMES, &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed reading back written mf wic to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata2.arraySize != 4
                          || metadata2.mipLevels != 1
                          || metadata2.dimension != TEX_DIMENSION_TEXTURE2D	)
                {   // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    printe( "Metadata error in wic readback:\n%ls\n", szDestPath );
                    printmeta( &metadata2 );
                    printmetachk( &metadata );
                }
                else
                {
                    ++npass;
                }
            }
        }
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// transcode
bool WICTest::Test07()
{
    static const GUID s_encoders[] =
    {
        GUID_ContainerFormatBmp,
        GUID_ContainerFormatPng,
        GUID_ContainerFormatJpeg,
        GUID_ContainerFormatTiff,
        GUID_ContainerFormatGif,
#ifndef NO_WMP
        GUID_ContainerFormatWmp
#endif
    };

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
        printe("ERROR: FindFirstFileEx FAILED (%u)\n", GetLastError());
        return false;
    }

    size_t ncount = 0;
    size_t npass = 0;

    for (;;)
    {
        if (!(findData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
        {
            // These files are not supported by the normal readers
            if (_wcsicmp(findData.cFileName, L"texture0.dds") == 0
                || _wcsicmp(findData.cFileName, L"texture3.dds") == 0
                || _wcsicmp(findData.cFileName, L"extended_dxt3.bmp") == 0)
            {
                if (!FindNextFile(hFile.get(), &findData))
                    break;

                continue;
            }

            wcscpy_s(szPath, szMediaPath);
            wcscat_s(szPath, findData.cFileName);

#if defined(_DEBUG) && defined(VERBOSE)
            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");
#endif

            wchar_t fname[_MAX_FNAME];
            wchar_t ext[_MAX_EXT];
            _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

            if (*fname == 0 || *ext == 0)
            {
                if (!FindNextFile(hFile.get(), &findData))
                    break;

                continue;
            }

#ifdef NO_WMP
            if (_wcsicmp(ext, L".wdp") == 0 || _wcsicmp(ext, L".jxr") == 0)
            {
                if (!FindNextFile(hFile.get(), &findData))
                    break;

                continue;
            }
#endif

#ifdef NO_CMYK
            if (wcsstr(fname, L"CMYK") != nullptr)
            {
                if (!FindNextFile(hFile.get(), &findData))
                    break;

                continue;
            }
#endif

            ScratchImage image;
            TexMetadata metadata;
            HRESULT hr;

            if (_wcsicmp(ext, L".dds") == 0)
            {
                hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, image);
            }
            else if (_wcsicmp(ext, L".hdr") == 0)
            {
                hr = LoadFromHDRFile(szPath, &metadata, image);
            }
            else if (_wcsicmp(ext, L".tga") == 0)
            {
                hr = LoadFromTGAFile(szPath, &metadata, image);
            }
            else
            {
                hr = LoadFromWICFile(szPath, WIC_FLAGS_NONE, &metadata, image);
            }

            if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)
                || hr == E_NOTIMPL
                || hr == WINCODEC_ERR_COMPONENTNOTFOUND
                || hr == E_OUTOFMEMORY
                || hr == E_UNEXPECTED
                || metadata.width >= 8192)
            {
                if (!FindNextFile(hFile.get(), &findData))
                    break;

                continue;
            }

            ++ncount;

            if (!(ncount % 100))
            {
                print(".");
            }

            if (FAILED(hr))
            {
                success = false;
                printe("Failed loading image from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else
            {
                bool pass = true;

                for (int j = 0; j < _countof(s_encoders); j++)
                {
                    Blob blob;
                    hr = SaveToWICMemory(image.GetImages(), image.GetImageCount(), WIC_FLAGS_NONE, s_encoders[j], blob);
                    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != E_UNEXPECTED)
                    {
                        success = false;
                        pass = false;
                        printe("Failed writing with encoder #%d (HRESULT %08X):\n%ls\n", j, static_cast<unsigned int>(hr), szPath);
                    }
                }

                if (pass)
                    ++npass;
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

    print(" %zu images tested, %zu images passed ", ncount, npass);

    return success;
}


//-------------------------------------------------------------------------------------
// Fuzz
bool WICTest::Test08()
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
        printe("ERROR: FindFirstFileEx FAILED (%u)\n", GetLastError());
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

#if defined(_DEBUG) && defined(VERBOSE)
            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");
#endif

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
                        (void)LoadFromWICMemory(blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_NONE, &metadata, image);
                    }
                }
            }

            // file
            {
                TexMetadata metadata;
                ScratchImage image;
                (void)LoadFromWICFile(szPath, WIC_FLAGS_NONE, &metadata, image);
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
