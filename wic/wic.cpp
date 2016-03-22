//-------------------------------------------------------------------------------------
// wic.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"

#include "directxtexp.h"

#include <wrl.h>

DEFINE_GUID(GUID_WICPixelFormat24bppBGR, 0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0c);

using namespace DirectX;
using Microsoft::WRL::ComPtr;

enum
{
    FLAGS_NONE          = 0x0,
    FLAGS_WIC2          = 0x1, // Requires WIC factory 2 to function
    FLAGS_ALTMD5_MASK   = 0xf0,
    FLAGS_MQR_ORIENT    = 0x100,
};

#define ALTMD5(n) (n << 4)

struct TestMedia
{
    DWORD options;
    TexMetadata metadata;
    const wchar_t *fname;
    uint8_t md5[16];
};

static const TestMedia g_TestMedia[] = 
{
// options | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename

// Classic image processing sample files
#ifdef _M_AMD64
{ ALTMD5(1), { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"fishingboat.jpg", { 0xef,0xea,0x23,0xe7,0x85,0xd2,0xd9,0x10,0x55,0x1d,0xa8,0x14,0xd4,0xaf,0x53,0xca } },
{ ALTMD5(2), { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg", { 0xef,0x7a,0x90,0x3e,0xa3,0x25,0x3d,0xf9,0x65,0x37,0x77,0x5a,0x74,0xe4,0x53,0x1b } },
#else
{ ALTMD5(1), { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"fishingboat.jpg", { 0x25,0x17,0x5a,0x16,0xf7,0xf8,0x3d,0x40,0x9c,0xdb,0x82,0x24,0x8c,0xf2,0x4e,0x83 } },
{ ALTMD5(2), { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg", { 0x35,0x8b,0x3f,0xdb,0x55,0x97,0x32,0xbc,0xa2,0x15,0x86,0x78,0xf7,0x18,0xf3,0x4b } },
#endif

{ FLAGS_MQR_ORIENT, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"baboon.tiff", { 0x5b,0x60,0x5c,0xb3,0x59,0x6e,0xb1,0x05,0xba,0x49,0xb9,0xe1,0x0f,0xfe,0x97,0x5f } },
{ FLAGS_MQR_ORIENT, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"f16.tiff", { 0xcf,0x54,0xa2,0xb6,0x06,0xf0,0x25,0x68,0x03,0x02,0xe2,0xb6,0xba,0x44,0xdf,0x3f } },
{ FLAGS_MQR_ORIENT, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"peppers.tiff", { 0x71,0xe3,0xf1,0x0f,0xfe,0xb1,0x4d,0x00,0x45,0xa5,0xfb,0x87,0x88,0x7d,0x36,0x24 } },
{ FLAGS_NONE, { 1024, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pentagon.tiff", { 0xb9,0x45,0x3d,0xa2,0x13,0xdc,0xe0,0x64,0xe2,0x6c,0x5c,0xb4,0x08,0xe2,0x1e,0xb8 } },

// PNG Test Suite sample files
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G01.PNG", { 0x73,0x14,0x2a,0xf2,0xff,0xbb,0x6b,0x70,0xa0,0x9f,0xa0,0x8d,0x99,0xc5,0xb8,0x88 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G02.PNG", { 0x0e,0xd9,0x17,0x65,0xa5,0xc7,0xe9,0xd7,0x99,0xa7,0xa9,0xbb,0xc5,0x8e,0x26,0xa6 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G04.PNG", { 0xb8,0xfe,0x64,0xf7,0xcd,0x3b,0x78,0x38,0xaf,0x3d,0xae,0xcf,0x0d,0xd1,0x12,0x9e } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G08.PNG", { 0x2e,0x13,0xa2,0x50,0xfc,0xed,0x46,0x97,0xf4,0x85,0x41,0x17,0x3d,0x46,0xdc,0x76 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN0G16.PNG", { 0x88,0x0f,0x06,0x0c,0x52,0x6e,0x17,0xab,0xd6,0x1e,0xd7,0x40,0x8e,0x75,0xa1,0x71 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C08.PNG", { 0x51,0x2c,0x38,0x74,0xe3,0x00,0x61,0xe6,0x23,0x73,0x9e,0x2f,0x9a,0xdc,0x4e,0xba } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN2C16.PNG", { 0xbc,0xdf,0x95,0x2b,0x45,0x9c,0x5f,0x7d,0xe9,0x57,0x6e,0x10,0xac,0xbe,0x98,0xb2 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P01.PNG", { 0x1b,0xa5,0x9f,0x52,0x7f,0xf2,0xcf,0xdc,0x68,0xbb,0x0c,0x34,0x87,0x86,0x2e,0x91 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P02.PNG", { 0x05,0x28,0xe9,0xac,0x36,0x52,0x52,0xa8,0xc0,0xe2,0xd9,0xce,0xd8,0xa2,0xcc,0x6b } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P04.PNG", { 0xa3,0x39,0x59,0x3b,0x0d,0x82,0x10,0x3e,0x30,0xed,0x7b,0x00,0xaf,0xd6,0x88,0x16 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN3P08.PNG", { 0xd3,0x6b,0xdb,0xef,0xc1,0x26,0xef,0x50,0xbd,0x57,0xd5,0x1e,0xb3,0x8f,0x2a,0xc4 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A08.PNG", { 0xe2,0x21,0x2e,0xc5,0xfa,0x02,0x6a,0x41,0x82,0x61,0x36,0xe9,0x83,0xbf,0x92,0xb2 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN4A16.PNG", { 0x2a,0xd0,0x3a,0x20,0xbe,0x76,0xd8,0xb6,0x5b,0x05,0x31,0x92,0x3b,0x92,0x73,0x76 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A08.PNG", { 0x04,0x6e,0xa7,0x73,0x99,0xc2,0x25,0x9f,0xee,0x17,0xf9,0x7e,0x44,0xdc,0x6a,0x00 } },
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"BASN6A16.PNG", { 0xd5,0x7e,0xc1,0x98,0x62,0x87,0xa5,0x42,0x8d,0xd8,0x40,0x5b,0xd5,0x95,0x27,0x32 } },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"AlphaBall.png", { 0x25,0x35,0x0c,0x71,0xea,0x44,0x04,0xfb,0xe2,0xae,0x2a,0xfd,0x8b,0x5a,0xea,0xcc } },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"AlphaEdge.png", { 0x02,0x8c,0x1b,0x06,0x61,0x06,0x12,0x7e,0x36,0x6f,0xda,0x0b,0x61,0x19,0x45,0xf2 } },
{ FLAGS_NONE, { 480, 360, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"black817-480x360-3_5.png", { 0x36,0x3d,0x9e,0x53,0x7f,0xf6,0x1c,0xba,0x66,0x1d,0xee,0x10,0xb7,0x9a,0x19,0x85 } },
{ FLAGS_NONE, { 393, 501, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"globe-scene-fish-bowl-pngcrush.png", { 0x26,0xd3,0x7c,0x42,0x37,0x6d,0x72,0x5b,0xf9,0x86,0xeb,0x51,0x05,0xaf,0x65,0x91 } },
{ FLAGS_NONE, { 440, 330, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"imgcomp-440x330.png", { 0x89,0xa8,0x9d,0xb7,0xa6,0x6d,0x06,0x29,0xb3,0xf5,0x2f,0x46,0xb0,0x76,0x90,0x3c } },

// Windows BMP files
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.bmp", { 0x94,0xb1,0xfe,0x8e,0x54,0x3f,0xa6,0x29,0x2b,0xdd,0x84,0x93,0x9b,0x9a,0x00,0x23 } },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.bmp", { 0x38,0x97,0x4f,0x56,0x61,0x72,0x7c,0x11,0xae,0xf7,0x82,0xe0,0x5a,0x9b,0x56,0xd3 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-8bppGray.bmp", { 0x7a,0x20,0xc7,0x73,0xa7,0xbb,0x95,0x27,0x44,0x7f,0x93,0x95,0x25,0x14,0x20,0x1e } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR555.bmp", { 0x6c,0x0a,0xda,0x95,0x0b,0x3f,0x61,0x12,0xc0,0xb7,0xaf,0x6c,0xeb,0x52,0x35,0x30 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR565.bmp", { 0xfc,0x8f,0xcb,0x01,0x11,0xc9,0xd6,0xa2,0x11,0x2c,0xba,0xe0,0x8a,0x3b,0x91,0x3f } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-24bppBGR.bmp", { 0x3c,0x7e,0xf9,0x77,0xa5,0xef,0xc7,0xba,0x0d,0x15,0x0e,0x66,0x09,0x51,0xad,0x1f } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM /*not a V5 header*/, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGRA.bmp", { 0xc6,0x4f,0x63,0xd7,0xc4,0x67,0x1f,0x10,0xc3,0x62,0x08,0x35,0x49,0xae,0x39,0x2b } },

#ifndef NO_WMP
// WDP HD Photo (aka JPEG XR) files
{ FLAGS_NONE, { 690, 690, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"jpeg-xr.wdp", { 0x28,0xae,0x74,0x0a,0xa3,0x28,0x86,0x4d,0x7f,0x32,0x88,0xf2,0xf1,0xe2,0x81,0x39 } },
{ ALTMD5(3), { 64, 64, 1, 1, 1, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGR101010_64x64.wdp", { 0x8a,0x07,0x17,0x06,0x64,0xa0,0x17,0xee,0x46,0x78,0x44,0x07,0xe9,0x4e,0x9f,0x6e } },
{ ALTMD5(4), { 64, 64, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGRA_64x64.wdp", { 0xae,0xb0,0x08,0x8c,0x05,0x79,0x7d,0xf4,0x53,0xec,0xa8,0xd4,0x01,0xe1,0xc4,0xa6 } },
{ ALTMD5(5), { 64, 64, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR565_64x64.wdp", { 0x5b,0x2c,0xab,0x18,0x5f,0x03,0xeb,0x0e,0xee,0xe8,0x36,0x02,0xa9,0x0c,0x28,0xdc } },
{ ALTMD5(6), { 64, 64, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR555_64x64.wdp", { 0x98,0x31,0x18,0x3e,0xc1,0x5f,0xb6,0xc6,0xdf,0x1d,0xc9,0xee,0xb3,0x4b,0x14,0x96 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-128bppRGBAFixedPoint.wdp", { 0x06,0xe4,0xfd,0x3d,0xa1,0xc0,0xda,0x37,0xa4,0x03,0x60,0x62,0x8e,0x98,0xe4,0x9a } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-128bppRGBAFloat.wdp", { 0x3b,0xe9,0x52,0x8e,0xbe,0xa6,0x7a,0x9e,0x7c,0x26,0x7d,0x19,0x00,0xeb,0x33,0x21 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-128bppRGBFloat.wdp", { 0x3b,0xe9,0x52,0x8e,0xbe,0xa6,0x7a,0x9e,0x7c,0x26,0x7d,0x19,0x00,0xeb,0x33,0x21 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM , TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR555.wdp", { 0x6c,0x0a,0xda,0x95,0x0b,0x3f,0x61,0x12,0xc0,0xb7,0xaf,0x6c,0xeb,0x52,0x35,0x30 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM , TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppBGR565.wdp", { 0xfc,0x8f,0xcb,0x01,0x11,0xc9,0xd6,0xa2,0x11,0x2c,0xba,0xe0,0x8a,0x3b,0x91,0x3f } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppGray.wdp", { 0x35,0x9d,0xaa,0xc8,0x30,0xa8,0xf7,0x1a,0xea,0xed,0xbe,0x03,0x91,0x2f,0x73,0x55 } },
{ ALTMD5(7), { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppGrayFixedPoint.wdp", { 0xe7,0x4b,0x92,0x44,0xca,0xf2,0x04,0xe3,0xca,0x55,0xc2,0x93,0xe6,0x66,0x50,0x6b } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-16bppGrayHalf.wdp", { 0x29,0x3d,0x5d,0x6d,0xe2,0x83,0x11,0xb5,0x00,0xa9,0x68,0x47,0xb4,0x21,0x56,0x4e } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-24bppBGR.wdp", { 0x3c,0x7e,0xf9,0x77,0xa5,0xef,0xc7,0xba,0x0d,0x15,0x0e,0x66,0x09,0x51,0xad,0x1f } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-24bppRGB.wdp", { 0x3c,0x7e,0xf9,0x77,0xa5,0xef,0xc7,0xba,0x0d,0x15,0x0e,0x66,0x09,0x51,0xad,0x1f } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGRA.wdp", { 0xc6,0x4f,0x63,0xd7,0xc4,0x67,0x1f,0x10,0xc3,0x62,0x08,0x35,0x49,0xae,0x39,0x2b } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppGrayFixedPoint.wdp", { 0x5d,0x70,0x75,0x4e,0x95,0x11,0xd1,0xa8,0xcc,0x93,0x68,0xaa,0xce,0x1c,0xf6,0xc0 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppGrayFloat.wdp", { 0x22,0x2c,0x24,0x4b,0xa8,0xb4,0xd3,0xa7,0x16,0xba,0x59,0xf2,0xe7,0x00,0x11,0xe0 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppRGBE.wdp", { 0xd9,0xd3,0xda,0x62,0x27,0xe9,0x8c,0xaa,0xea,0x22,0xd7,0x42,0xa8,0x69,0x2d,0x3b } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-48bppRGB.wdp", { 0x11,0xf0,0xa6,0x35,0x59,0xf8,0x41,0x87,0xa6,0xb8,0xda,0x9e,0xe4,0xc4,0xb3,0x83 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-48bppRGBFixedPoint.wdp", { 0x77,0x34,0xb5,0x7b,0xbe,0xbd,0x4f,0x8f,0x10,0xec,0xbc,0x18,0xf0,0xc2,0x63,0x2a } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-48bppRGBHalf.wdp", { 0xc5,0x10,0x2d,0x1e,0x8f,0xc0,0x34,0x78,0xdd,0xe3,0x45,0x73,0xde,0xca,0xa6,0xf4 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-64bppRGBA.wdp", { 0x11,0xf0,0xa6,0x35,0x59,0xf8,0x41,0x87,0xa6,0xb8,0xda,0x9e,0xe4,0xc4,0xb3,0x83 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-64bppRGBAFixedPoint.wdp", { 0x77,0x34,0xb5,0x7b,0xbe,0xbd,0x4f,0x8f,0x10,0xec,0xbc,0x18,0xf0,0xc2,0x63,0x2a } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-64bppRGBAHalf.wdp", { 0xc5,0x10,0x2d,0x1e,0x8f,0xc0,0x34,0x78,0xdd,0xe3,0x45,0x73,0xde,0xca,0xa6,0xf4 } },
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-8bppGray.wdp", { 0x49,0x82,0x54,0x1a,0xbb,0x71,0x9c,0xe8,0xa1,0x64,0x05,0x8b,0xde,0x0a,0xf3,0x53 } },
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) || defined(_WIN7_PLATFORM_UPDATE)
{ FLAGS_WIC2, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-96bppRGBFixedPoint.wdp", { 0x9f,0xb9,0x2a,0xa3,0xe9,0x27,0x55,0x18,0xc6,0x6a,0xd6,0x39,0xb3,0xe6,0x6e,0xa2 } },
#else
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-96bppRGBFixedPoint.wdp", { 0x06,0xe4,0xfd,0x3d,0xa1,0xc0,0xda,0x37,0xa4,0x03,0x60,0x62,0x8e,0x98,0xe4,0x9a } },
#endif
{ FLAGS_NONE, { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-BlackWhite.wdp", { 0xfe,0x45,0xf9,0xc1,0xcb,0xad,0x46,0xbe,0x53,0xb4,0x4a,0xa1,0x05,0x08,0x69,0x0b } },
#endif

// Multi-frame example files
{ FLAGS_NONE, { 500, 350, 1, 15, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Animated.gif", {0xa7,0xdb,0x2e,0x1e,0x35,0x50,0xb0,0xb1,0x4c,0xe7,0x37,0x2e,0xce,0x9e,0x9d,0x95} },
{ FLAGS_MQR_ORIENT | ALTMD5(13), { 512, 512, 1, 4, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"MultiFrame.tif", {0xfb,0x22,0x3e,0x83,0x13,0x99,0x17,0x64,0x5c,0x76,0xde,0x16,0xf7,0xbd,0xff,0x58} },
{ FLAGS_NONE, { 500, 500, 1, 24, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SpinCube.gif", {0x3b,0x32,0x85,0x22,0xc5,0xb9,0x48,0x4b,0xa6,0x93,0xba,0x9d,0x30,0xd4,0x6a,0x0e} },

// Direct2D Test Images
{ FLAGS_NONE, { 300, 227, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bppRGBAI.png", { 0x8e,0x69,0x69,0xa1,0x04,0x0b,0xf7,0x90,0xbf,0x19,0x31,0xbd,0x0e,0xa1,0x9c,0x4f } },
{ FLAGS_NONE, { 300, 227, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"32bppRGBAN.png", { 0x8e,0x69,0x69,0xa1,0x04,0x0b,0xf7,0x90,0xbf,0x19,0x31,0xbd,0x0e,0xa1,0x9c,0x4f } },
{ FLAGS_NONE, { 4096, 4096, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"PNG-interlaced-compression9-24bit-4096x4096.png", { 0x77,0x4e,0x80,0xbd,0x40,0xed,0xc8,0x04,0x9d,0x1d,0x38,0xb1,0xf5,0x9a,0x0f,0x4d } },
{ FLAGS_NONE, { 203, 203, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"transparent_clock.png", { 0x7b,0x33,0x31,0xb4,0x3d,0x90,0xe2,0x72,0xff,0x73,0x5c,0x4a,0xb9,0xd4,0x05,0xeb } },

{ FLAGS_NONE, { 564, 749, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Bad5.bmp", { 0x1c,0x12,0x1a,0xe8,0x80,0x9a,0x17,0xa2,0x06,0x22,0xd0,0x31,0x2e,0x30,0x52,0x4b } },
{ FLAGS_NONE, { 640, 480, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"COUGAR.BMP", { 0xb8,0x6e,0x2f,0x2a,0x92,0x2c,0xb0,0x21,0x6e,0x07,0xc1,0x61,0x54,0x2e,0x3f,0x22 } },
{ FLAGS_NONE, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rgb32table888td.bmp", { 0x7e,0xfe,0x6a,0x59,0xbc,0xdd,0xb4,0xd4,0x35,0x56,0x60,0xab,0x71,0x3b,0x39,0xb9 } },

#ifdef _M_AMD64
{ FLAGS_MQR_ORIENT | ALTMD5(8), { 1024, 768, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Dock.jpg", { 0x38,0x1e,0xd2,0x54,0x50,0xa3,0x7a,0xa9,0x06,0x48,0xce,0x78,0x91,0x77,0x3d,0xce } },
{ FLAGS_MQR_ORIENT | ALTMD5(9), { 162, 80, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Glee_Newscorp_rev.jpg", { 0x2b,0xf0,0xb9,0xf7,0xa6,0x68,0x68,0x71,0x22,0x5f,0xce,0x84,0x04,0x0d,0x9f,0xab } },
{ ALTMD5(10), { 640, 480, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"image.127839287370267572.jpg", { 0xde,0xff,0x04,0x06,0x82,0xdc,0x05,0x76,0x66,0x56,0xd0,0x8d,0xfd,0xc3,0x1b,0xaa } },
{ FLAGS_MQR_ORIENT | ALTMD5(11), { 500, 500, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"progressivehuffman.jpg", { 0x5e,0x89,0x13,0x94,0xe5,0x4b,0x58,0xa1,0x03,0xc0,0x13,0xe4,0x32,0xea,0x42,0x8c } },
{ ALTMD5(12), { 512, 384, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"quad-jpeg.tif", { 0x4b,0x50,0xcb,0x88,0x6e,0xbf,0x85,0xe4,0xd0,0xd9,0x02,0x1b,0x9f,0x6b,0xd4,0xe0 } },
#else
{ FLAGS_MQR_ORIENT | ALTMD5(8), { 1024, 768, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Dock.jpg", { 0xeb,0x05,0x4e,0x02,0x81,0x22,0xfd,0x3a,0x35,0xe7,0xe2,0x80,0x54,0xfd,0x85,0xf5 } },
{ FLAGS_MQR_ORIENT | ALTMD5(9), { 162, 80, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Glee_Newscorp_rev.jpg", { 0x7f,0x64,0x3e,0xe9,0x27,0xd8,0x4f,0x6b,0x59,0x76,0x28,0xdf,0xc5,0x03,0x6b,0x0d } },
{ ALTMD5(10), { 640, 480, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"image.127839287370267572.jpg", { 0xa6,0xfb,0x0c,0xae,0x75,0xeb,0xca,0xce,0xed,0x6d,0x88,0x6b,0x2c,0x62,0xb7,0xc4 } },
{ FLAGS_MQR_ORIENT | ALTMD5(11), { 500, 500, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"progressivehuffman.jpg", { 0x57,0x72,0x29,0x57,0x51,0x49,0xf3,0xdc,0xa1,0x4d,0x3b,0x23,0x42,0x49,0x2a,0xf8 } },
{ ALTMD5(12), { 512, 384, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"quad-jpeg.tif", { 0x53,0x52,0xa4,0x0d,0xe0,0x5a,0xe9,0x0d,0x83,0x6c,0xbd,0x27,0xd1,0xa6,0x3e,0x1e } },
#endif

{ FLAGS_MQR_ORIENT, { 73, 43, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"flower-minisblack-06.tif", { 0xa7,0x55,0x69,0xf5,0xde,0xbc,0x71,0xfd,0x20,0x03,0xe5,0x73,0x9f,0x45,0x2e,0x2e } },
{ FLAGS_NONE, { 760, 399, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"H3_06.TIF", { 0xd8,0x53,0x57,0x40,0x34,0x2e,0xf6,0x61,0x61,0xe1,0x08,0x38,0xb5,0x84,0x47,0x11 } },

{ FLAGS_NONE, { 1920, 1200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"abstract-test-pattern.jpg", { 0xe7,0xf0,0xda,0x83,0x80,0x05,0x5d,0x66,0x86,0xb7,0xce,0x35,0xce,0x2d,0x3c,0x61 } },
{ FLAGS_NONE, { 256, 224, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"deltae_base.png", { 0x02,0x8b,0xf7,0x6e,0xe7,0x05,0xe9,0x3a,0xc5,0xb1,0xe1,0x0b,0xd5,0xd6,0xc7,0x56 } },
{ FLAGS_NONE, { 768, 576, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Grayscale_Staircase.png", { 0xd4,0xff,0x0b,0x98,0x34,0x7b,0xe7,0x05,0x7f,0xab,0x4b,0xa0,0xe0,0xd7,0xe3,0x8a } },

{ FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Omega-SwanNebula-M17.png", { 0x3b,0x21,0x9b,0x2a,0x4a,0xcc,0x2e,0x4f,0x0a,0x31,0x19,0xdf,0x27,0xd0,0x4e,0xab } },

// sRGB test cases
{ FLAGS_NONE, { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D  }, MEDIA_PATH L"testpattern.png", { 0x1e,0x85,0xd6,0xef,0xe1,0xa9,0x24,0x58,0xb4,0x00,0x6d,0x12,0xf8,0x1c,0xf6,0x44 } },
{ FLAGS_NONE, { 64, 24, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p99sr.png", {0x32,0xd0,0xe2,0x92,0x5d,0x00,0x43,0xbb,0xaa,0xbd,0xa2,0x52,0xc4,0xcf,0x9f,0xdd} },
{ FLAGS_NONE, { 976, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"chip_lagrange_o3_400_sigm_6p5x50.png", {0x24,0x02,0xa9,0x6b,0x21,0x17,0x30,0xbc,0x65,0x8c,0x3e,0x1d,0x29,0x8d,0x52,0x73} },
{ FLAGS_MQR_ORIENT, { 512, 683, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"cup_small.jpg", {0x2f,0x7c,0x2d,0xa6,0x8e,0x25,0x10,0x26,0xcc,0x05,0xb6,0x70,0x63,0xd2,0x44,0x7b} },
{ FLAGS_NONE, { 800, 600, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"livingrobot-rear.tiff", {0xf0,0xa0,0x56,0xc3,0xda,0x17,0x4e,0x8a,0x79,0x44,0xa3,0xa1,0x5f,0x96,0x84,0xc2} },
{ FLAGS_NONE, { 2048, 2048, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex4.png", {0x37,0x6c,0x85,0xd5,0x20,0x4e,0xab,0xbd,0xf1,0x98,0xa3,0x07,0xde,0x7b,0x2d,0xb1} },
{ FLAGS_NONE, { 21600, 10800, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.png", {0x26,0x00,0xa1,0x94,0x33,0xe7,0x36,0x46,0x03,0xb6,0xe0,0xf8,0x1c,0xfb,0x56,0x6c} },
{ FLAGS_NONE, { 16384, 8192, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuseTexture.png", {0x95,0xdc,0xb3,0xa7,0xd0,0xf8,0x63,0x04,0x75,0x1b,0x8f,0x62,0x1f,0xb1,0x44,0x12} },
{ FLAGS_NONE, { 8192, 4096, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"callisto.png", {0x92,0xf7,0x93,0xd3,0x34,0x12,0xef,0x5b,0x54,0x01,0x1a,0xb6,0x7c,0xdb,0x6b,0xa8} },

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) || defined(_WIN7_PLATFORM_UPDATE)
{ FLAGS_WIC2, { 1024, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"ramps_vdm_rel.TIF", { 0xea,0x2c,0xd3,0x74,0xef,0x08,0xbf,0x65,0xfa,0x7a,0x49,0x62,0x2a,0xd3,0x5f,0x82 } },
{ FLAGS_WIC2, { 768, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"96bpp_RGB_FP.TIF", { 0x3d,0x05,0x35,0x60,0x63,0x81,0xcf,0x86,0x09,0x5f,0x8f,0xcc,0xda,0x58,0xd9,0xfe } },
#endif

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
// TODO - test images for GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppPRGBAHalf
#endif

// orientation test cases
{ FLAGS_MQR_ORIENT, { 640, 480, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"JpegWithOrientation.JPG", { 0xe0,0x0b,0x5f,0xe6,0x16,0xfc,0x58,0xf2,0x32,0xae,0x22,0xd7,0xd2,0xe7,0x68,0x6b } },
};

//-------------------------------------------------------------------------------------

struct AltMD5
{
    uint8_t md5[16];
};

static const AltMD5 g_AltMD5[] =
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
    { 0x3f,0x00,0xb9,0x27,0xba,0xfa,0x86,0x86,0xbc,0xbf,0x9f,0x82,0xe2,0xc2,0x17,0xe7 }, // ALTMD5(13)
};

//-------------------------------------------------------------------------------------

struct SaveMedia
{
    WICCodecs tcodec;
    const wchar_t *ext;
    const wchar_t *source;
};

static const SaveMedia g_SaveMedia[] = 
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

// TODO - DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM

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

//-------------------------------------------------------------------------------------
extern HRESULT LoadBlobFromFile( _In_z_ LPCWSTR szFile, Blob& blob );
extern size_t DetermineFileSize( _In_z_ LPCWSTR szFile );
extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest );
extern HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image );


//-------------------------------------------------------------------------------------
// GetWICFactory/SetWICFactory
bool Test00()
{
    bool systemSupportsWIC2 = false;

    {
        ComPtr<IWICImagingFactory> wic;
        HRESULT hr = CoCreateInstance( CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
                                       __uuidof(IWICImagingFactory2), reinterpret_cast<LPVOID*>( wic.GetAddressOf() ) );
        if (SUCCEEDED(hr))
        {
            systemSupportsWIC2 = true;

            SetWICFactory(wic.Get());

            bool iswic2 = false;
            IWICImagingFactory* pWIC = GetWICFactory(iswic2);
            if (pWIC != wic.Get() || !iswic2)
            {
                printe("SetWICFactory failed (WIC2)\n");
                return false;
            }
        }
        else
        {
            hr = CoCreateInstance( CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER,
                                   __uuidof(IWICImagingFactory2), reinterpret_cast<LPVOID*>( wic.GetAddressOf() ) );
            if (SUCCEEDED(hr))
            {
                SetWICFactory(wic.Get());

                bool iswic2 = false;
                IWICImagingFactory* pWIC = GetWICFactory(iswic2);
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
        IWICImagingFactory* pWIC = GetWICFactory(iswic2);

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
bool Test01()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && iswic2 )
            continue;

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
            hr = GetMetadataFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, metadata,
                                           [&](IWICMetadataQueryReader* reader)
                                           {
                                               if ( !reader )
                                               {
                                                   success = false;
                                                   printe( "Failed metadata query reader from memory:\n%S\n", szPath );
                                               }
#if !defined(_XBOX_ONE) || !defined(_TITLE)
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
                                                       printe( "Failed metadata query read of orientation from memory:\n%S\n", szPath );
                                                   }
                                                   
                                                   PropVariantClear( &value );
                                               }
#endif
                                           } );

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed getting data from memory (HRESULT %08X):\n%S\n", hr, szPath );
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
                            printe( "Metadata error with WIC_FLAGS_FORCE_RGB:\n%S\n", szPath );
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
                            printe( "Metadata error with WIC_FLAGS_IGNORE_SRGB:\n%S\n", szPath );
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
                            printe( "Metadata error with WIC_FLAGS_NO_X2_BIAS:\n%S\n", szPath );
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
                            printe( "Metadata error with WIC_FLAGS_NO_16BPP:\n%S\n", szPath );
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
                            printe( "Metadata error without WIC_FLAGS_ALLOW_MONO:\n%S\n", szPath );
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
                        printe( "Metadata error without WIC_FLAGS_ALL_FRAMES:\n%S\n", szPath );
                        printmeta( &metadata2 );
                    }
                }

                if ( pass )
                    ++npass;
            }
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// GetMetadataFromWICFile
bool Test02()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && iswic2 )
            continue;

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
        HRESULT hr = GetMetadataFromWICFile( szPath, WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, metadata,
                                             [&](IWICMetadataQueryReader* reader)
                                             {
                                                if ( !reader )
                                                {
                                                    success = false;
                                                    printe( "Failed metadata query reader from file:\n%S\n", szPath );
                                                }
#if !defined(_XBOX_ONE) || !defined(_TITLE)
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
                                                        printe( "Failed metadata query read of orientation from file:\n%S\n", szPath );
                                                    }
                                                   
                                                    PropVariantClear( &value );
                                                }
#endif
                                             } );

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
                        printe( "Metadata error with WIC_FLAGS_FORCE_RGB:\n%S\n", szPath );
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
                        printe( "Metadata error with WIC_FLAGS_IGNORE_SRGB:\n%S\n", szPath );
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
                        printe( "Metadata error with WIC_FLAGS_NO_X2_BIAS:\n%S\n", szPath );
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
                        printe( "Metadata error with WIC_FLAGS_NO_16BPP:\n%S\n", szPath );
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
                        printe( "Metadata error without WIC_FLAGS_ALLOW_MONO:\n%S\n", szPath );
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
                    printe( "Metadata error without WIC_FLAGS_ALL_FRAMES:\n%S\n", szPath );
                    printmeta( &metadata2 );
                }
            }

            if ( pass )
                ++npass;
        }

        ++ncount;
    }

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromWICMemory
bool Test03()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && iswic2 )
            continue;

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
            hr = LoadFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, &metadata, image );

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed loading wic from memory (HRESULT %08X):\n%S\n", hr, szPath );
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
                    printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%S\n", hr, szPath );
                }
                else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0
                          && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                          && (index != 60))
                {
                    success = false;
                    pass = false;
                    printe( "Failed comparing MD5 checksum:\n%S\n", szPath );
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

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromWICFile
bool Test04()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    bool iswic2;
    (void)GetWICFactory(iswic2);

    for( size_t index=0; index < _countof(g_TestMedia); ++index )
    {
        if ( (g_TestMedia[index].options & FLAGS_WIC2) && iswic2 )
            continue;

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
        HRESULT hr = LoadFromWICFile( szPath, WIC_FLAGS_ALLOW_MONO | WIC_FLAGS_ALL_FRAMES, &metadata, image );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading wic from (HRESULT %08X):\n%S\n", hr, szPath );
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
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%S\n", hr, szPath );
            }
            else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0
                      && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                      && (index != 60) )
            {
                success = false;
                pass = false;
                printe( "Failed comparing MD5 checksum:\n%S\n", szPath );
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

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveWICToMemory
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

        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr;

        WCHAR ext[_MAX_EXT];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, NULL, NULL, ext, _MAX_EXT );

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
            printe( "Failed loading DDS from (HRESULT %08X):\n%S\n", hr, szPath );
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
                printe( "Failed writing wic (%S) to memory (HRESULT %08X):\n%S\n", g_SaveMedia[index].ext, hr, szPath );
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
                    printe( "Failed reading back written wic (%S) from memory (HRESULT %08X):\n%S\n", g_SaveMedia[index].ext, hr, szPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata.arraySize != metadata2.arraySize
                          || metadata.dimension != metadata2.dimension )
                { // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    pass = false;
                    printe( "Metadata error in wic (%S) memory readback:\n%S\n", g_SaveMedia[index].ext, szPath );
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
                    printe( "Failed writing wic 24bpp (%S) to memory (HRESULT %08X):\n%S\n", g_SaveMedia[index].ext, hr, szPath );
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
                        printe( "Failed reading back written wic 24bpp (%S) from memory (HRESULT %08X):\n%S\n", g_SaveMedia[index].ext, hr, szPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata.dimension != metadata2.dimension
                              || (metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM && metadata2.format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) )
                    { // miplevel is going to be 1 for WIC images
                        success = false;
                        pass = false;
                        printe( "Metadata error in wic 24bpp (%S) memory readback:\n%S\n", g_SaveMedia[index].ext, szPath );
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
                                                PROPBAG2 options[2] = { 0, 0 };
                                                options[0].pstrName = L"CompressionQuality";
                                                options[1].pstrName = L"TiffCompressionMethod";

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
                        printe( "Failed writing wic with custom props (%S) to memory (HRESULT %08X):\n%S\n", g_SaveMedia[index].ext, hr, szPath );
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
                            printe( "Failed reading back written wic custom props (%S) from memory (HRESULT %08X):\n%S\n", g_SaveMedia[index].ext, hr, szPath );
                        }
                        else if ( metadata.width != metadata2.width
                                    || metadata.height != metadata2.height
                                    || metadata.arraySize != metadata2.arraySize
                                    || metadata.dimension != metadata2.dimension )
                        { // miplevel is going to be 1 for WIC images, format changes
                            success = false;
                            pass = false;
                            printe( "Metadata error in wic custom props (%S) memory readback:\n%S\n", g_SaveMedia[index].ext, szPath );
                            printmeta( &metadata2 );
                            printmetachk( &metadata );
                        }
                        else if ( blob.GetBufferSize() <= original )
                        { // We are forcing uncompressed in the custom props, so new image should be strictly larger than original
                            success = false;
                            pass = false;
                            printe( "Failed wic custom props (%S) from memory sizes unchanged (%Iu ... %Iu):\n%S\n", g_SaveMedia[index].ext, blob.GetBufferSize(), original, szPath );
                        }
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
        WCHAR szPath[MAX_PATH];
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
            printe( "Failed loading DDS from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else
        {
            Blob blob;
            hr = SaveToWICMemory( image.GetImages(), image.GetImageCount(), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_TIFF ), blob );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed writing mf wic .tiff to memory (HRESULT %08X):\n%S\n", hr, szPath );
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromWICMemory( blob.GetBufferPointer(), blob.GetBufferSize(), WIC_FLAGS_ALL_FRAMES, &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed reading back written mf wic .tiff to memory (HRESULT %08X):\n%S\n", hr, szPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata2.arraySize != 4
                          || metadata2.dimension != TEX_DIMENSION_TEXTURE2D	)
                {   // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    printe( "Metadata error in wic .tiff memory readback:\n%S\n", szPath );
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

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveWICToFile
bool Test06()
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

        // Form dest path
        WCHAR ext[_MAX_EXT];
        WCHAR fname[_MAX_FNAME];
        _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        WCHAR tempDir[MAX_PATH];
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"wic", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, NULL );

        WCHAR szDestPath[MAX_PATH];
        _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, fname, g_SaveMedia[index].ext );

        WCHAR szDestPath2[MAX_PATH];
        WCHAR tname[_MAX_FNAME] = { 0 };
        wcscpy_s( tname, fname );
        wcscat_s( tname, L"_tf" );
        _wmakepath_s( szDestPath2, MAX_PATH, NULL, tempDir, tname, g_SaveMedia[index].ext );

        WCHAR szDestPath3[MAX_PATH];
        wcscpy_s( tname, fname );
        wcscat_s( tname, L"_props" );
        _wmakepath_s( szDestPath3, MAX_PATH, NULL, tempDir, tname, g_SaveMedia[index].ext );

#ifdef DEBUG
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
            printe( "Failed loading DDS from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else
        {
            bool pass = true;

            hr = SaveToWICFile( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( g_SaveMedia[index].tcodec ), szDestPath );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed writing wic to (HRESULT %08X):\n%S\n", hr, szDestPath );
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
                    printe( "Failed reading back written wic (HRESULT %08X):\n%S\n", hr, szDestPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata.arraySize != metadata2.arraySize
                          || metadata2.mipLevels != 1
                          || metadata.dimension != metadata2.dimension )
                {   // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    pass = false;
                    printe( "Metadata error in wic readback:\n%S\n", szDestPath );
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
                printe( "Failed writing wic 24bpp to (HRESULT %08X):\n%S\n", hr, szDestPath2 );
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
                    printe( "Failed reading back written wic 24bpp (HRESULT %08X):\n%S\n", hr, szDestPath2 );
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
                    printe( "Metadata error in wic 24bpp readback:\n%S\n", szDestPath2 );
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
                                            PROPBAG2 options[2] = { 0, 0 };
                                            options[0].pstrName = L"CompressionQuality";
                                            options[1].pstrName = L"TiffCompressionMethod";

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
                    printe( "Failed writing wic with custom props to (HRESULT %08X):\n%S\n", hr, szDestPath3 );
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
                        printe( "Failed reading back written wic custom props (HRESULT %08X):\n%S\n", hr, szDestPath3 );
                    }
                    else if ( metadata.width != metadata2.width
                                || metadata.height != metadata2.height
                                || metadata.arraySize != metadata2.arraySize
                                || metadata2.mipLevels != 1
                                || metadata.dimension != metadata2.dimension )
                    {   // miplevel is going to be 1 for WIC images, format changes
                        success = false;
                        pass = false;
                        printe( "Metadata error in wic cusomt props readback:\n%S\n", szDestPath3 );
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
                            printe( "Failed wic custom props sizes unchanged (%Iu ... %Iu):\n%S\n", uncompressed, original, szDestPath3 );
                        }
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
        WCHAR szPath[MAX_PATH];
        DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"testvol8888.dds", szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        WCHAR szDestPath[MAX_PATH];
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
            printe( "Failed loading DDS from (HRESULT %08X):\n%S\n", hr, szPath );
        }
        else
        {
            hr = SaveToWICFile( image.GetImages(), image.GetImageCount(), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_TIFF ), szDestPath );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed writing mf wic to (HRESULT %08X):\n%S\n", hr, szDestPath );
            }
            else
            {
                TexMetadata metadata2;
                ScratchImage image2;
                hr = LoadFromWICFile( szDestPath, WIC_FLAGS_ALL_FRAMES, &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed reading back written mf wic to (HRESULT %08X):\n%S\n", hr, szDestPath );
                }
                else if ( metadata.width != metadata2.width
                          || metadata.height != metadata2.height
                          || metadata2.arraySize != 4
                          || metadata2.mipLevels != 1
                          || metadata2.dimension != TEX_DIMENSION_TEXTURE2D	)
                {   // Formats can vary for readback, and miplevel is going to be 1 for WIC images
                    success = false;
                    printe( "Metadata error in wic readback:\n%S\n", szDestPath );
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

    print("%Iu images tested, %Iu images passed ", ncount, npass );

    return success;
}
