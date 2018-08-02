//-------------------------------------------------------------------------------------
// mipmaps.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

#pragma warning(disable:6262) // test code doesn't need conservative stack size

enum
{
    FLAGS_NONE              = 0x0,
    FLAGS_SEPALPHA          = 0x1,
    FLAGS_SKIP_POINTNOWIC   = 0x2,
    FLAGS_SKIP_SRGB         = 0x4,
    FLAGS_SKIP_TOPTEST      = 0x8,
    FLAGS_ALTMD5_MASK       = 0xff0,
    FLAGS_ALTMD5_MASKB      = 0xff000,
    FLAGS_ALTMD5_MASKC      = 0xff00000,
};

#define ALTMD5(n) (n << 4)
#define ALTMD5B(n) (n << 12)
#define ALTMD5C(n) (n << 20)

struct MipMapMedia
{
    DWORD options;
    TexMetadata metadata;
    const wchar_t *fname;
    uint8_t md5[16];
    uint8_t md5_point[16];
    uint8_t md5_linear[16];
#ifdef _M_X64
    uint8_t md5_x86[16];
    uint8_t md5_cubic[16];
#else
    uint8_t md5_cubic[16];
    uint8_t md5_x64[16];
#endif
    uint8_t md5_tri[16];
    uint8_t md5_c[16];
    uint8_t md5_sepalpha[16];
};

static const MipMapMedia g_MipMapMedia[] = 
{
// <source> test-options | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
{ FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds",
    {0xa5,0xad,0x60,0x2a,0x3b,0xe9,0x67,0x0f,0xa0,0x11,0xf0,0x4d,0xee,0x5e,0xb0,0xee},{0x3f,0xe1,0xe1,0x30,0xbf,0x9b,0x53,0x10,0x6a,0xed,0xad,0x55,0xf7,0x98,0xb8,0x4b},{0x48,0x5d,0x4a,0xd1,0xe0,0x06,0x97,0x54,0x32,0xb5,0xdb,0x9c,0xda,0xd0,0x73,0x3b},
    {0x79,0xb8,0xb5,0x40,0xd9,0xfd,0x5e,0x5e,0x4e,0x7d,0x92,0xbc,0x5b,0x38,0xfa,0xdf},{0x7f,0x4f,0xe0,0x64,0x39,0x57,0x75,0x9f,0xf5,0x9f,0x8c,0xe3,0x31,0x7e,0x0e,0x77},{0x9f,0xd4,0x11,0x1a,0x50,0x5d,0xcd,0xcb,0xb7,0x15,0x1d,0x1e,0xab,0xf9,0x72,0xfc},
    {0},{0} },
{ FLAGS_NONE, { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds",
    {0xa5,0xad,0x60,0x2a,0x3b,0xe9,0x67,0x0f,0xa0,0x11,0xf0,0x4d,0xee,0x5e,0xb0,0xee},{0x3f,0xe1,0xe1,0x30,0xbf,0x9b,0x53,0x10,0x6a,0xed,0xad,0x55,0xf7,0x98,0xb8,0x4b},{0x48,0x5d,0x4a,0xd1,0xe0,0x06,0x97,0x54,0x32,0xb5,0xdb,0x9c,0xda,0xd0,0x73,0x3b},
    {0x79,0xb8,0xb5,0x40,0xd9,0xfd,0x5e,0x5e,0x4e,0x7d,0x92,0xbc,0x5b,0x38,0xfa,0xdf},{0x7f,0x4f,0xe0,0x64,0x39,0x57,0x75,0x9f,0xf5,0x9f,0x8c,0xe3,0x31,0x7e,0x0e,0x77},{0x9f,0xd4,0x11,0x1a,0x50,0x5d,0xcd,0xcb,0xb7,0x15,0x1d,0x1e,0xab,0xf9,0x72,0xfc},
    {0xa5,0xad,0x60,0x2a,0x3b,0xe9,0x67,0x0f,0xa0,0x11,0xf0,0x4d,0xee,0x5e,0xb0,0xee},{0} },
{ FLAGS_NONE, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds",
    {0xa5,0xad,0x60,0x2a,0x3b,0xe9,0x67,0x0f,0xa0,0x11,0xf0,0x4d,0xee,0x5e,0xb0,0xee},{0x3f,0xe1,0xe1,0x30,0xbf,0x9b,0x53,0x10,0x6a,0xed,0xad,0x55,0xf7,0x98,0xb8,0x4b},{0x48,0x5d,0x4a,0xd1,0xe0,0x06,0x97,0x54,0x32,0xb5,0xdb,0x9c,0xda,0xd0,0x73,0x3b},
    {0x79,0xb8,0xb5,0x40,0xd9,0xfd,0x5e,0x5e,0x4e,0x7d,0x92,0xbc,0x5b,0x38,0xfa,0xdf},{0x7f,0x4f,0xe0,0x64,0x39,0x57,0x75,0x9f,0xf5,0x9f,0x8c,0xe3,0x31,0x7e,0x0e,0x77},{0x9f,0xd4,0x11,0x1a,0x50,0x5d,0xcd,0xcb,0xb7,0x15,0x1d,0x1e,0xab,0xf9,0x72,0xfc},
    {0x63,0x0e,0x6c,0xc1,0xa0,0xea,0x5c,0xfc,0xd5,0xa6,0x1e,0xef,0x45,0xc2,0x94,0x9d},{0} },
{ FLAGS_NONE, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds",
    {0xa5,0xad,0x60,0x2a,0x3b,0xe9,0x67,0x0f,0xa0,0x11,0xf0,0x4d,0xee,0x5e,0xb0,0xee},{0x3f,0xe1,0xe1,0x30,0xbf,0x9b,0x53,0x10,0x6a,0xed,0xad,0x55,0xf7,0x98,0xb8,0x4b},{0x48,0x5d,0x4a,0xd1,0xe0,0x06,0x97,0x54,0x32,0xb5,0xdb,0x9c,0xda,0xd0,0x73,0x3b},
    {0x79,0xb8,0xb5,0x40,0xd9,0xfd,0x5e,0x5e,0x4e,0x7d,0x92,0xbc,0x5b,0x38,0xfa,0xdf},{0x7f,0x4f,0xe0,0x64,0x39,0x57,0x75,0x9f,0xf5,0x9f,0x8c,0xe3,0x31,0x7e,0x0e,0x77},{0x9f,0xd4,0x11,0x1a,0x50,0x5d,0xcd,0xcb,0xb7,0x15,0x1d,0x1e,0xab,0xf9,0x72,0xfc},
    {0x63,0x0e,0x6c,0xc1,0xa0,0xea,0x5c,0xfc,0xd5,0xa6,0x1e,0xef,0x45,0xc2,0x94,0x9d},{0} },
{ FLAGS_NONE, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds",
    {0xad,0xfa,0x7b,0x12,0x74,0x4d,0x31,0x53,0x0f,0x30,0xbb,0xe5,0x8b,0x12,0xd1,0x16},{0xb1,0x88,0x30,0xbb,0x98,0x20,0xb2,0x2d,0x38,0xbd,0x12,0x0b,0x10,0xdd,0x4c,0x26},{0xed,0x8a,0x1e,0x3e,0xfb,0x9a,0x21,0x94,0xf5,0x61,0xea,0xa6,0x6f,0x6a,0xee,0xb4},
    {0xf5,0x7b,0x63,0x20,0x13,0x0a,0xa6,0x98,0x7b,0x3f,0xa7,0xb5,0xbf,0x96,0xa8,0xae},{0xf5,0x7b,0x63,0x20,0x13,0x0a,0xa6,0x98,0x7b,0x3f,0xa7,0xb5,0xbf,0x96,0xa8,0xae},{0xdd,0xaf,0xd4,0x66,0xda,0x67,0x54,0x8d,0x1f,0xa5,0xdc,0x3c,0xdc,0x1a,0x38,0x76},
    {0xad,0xfa,0x7b,0x12,0x74,0x4d,0x31,0x53,0x0f,0x30,0xbb,0xe5,0x8b,0x12,0xd1,0x16},{0} },
{ FLAGS_NONE, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds",
    {0xad,0xfa,0x7b,0x12,0x74,0x4d,0x31,0x53,0x0f,0x30,0xbb,0xe5,0x8b,0x12,0xd1,0x16},{0xb1,0x88,0x30,0xbb,0x98,0x20,0xb2,0x2d,0x38,0xbd,0x12,0x0b,0x10,0xdd,0x4c,0x26},{0xed,0x8a,0x1e,0x3e,0xfb,0x9a,0x21,0x94,0xf5,0x61,0xea,0xa6,0x6f,0x6a,0xee,0xb4},
    {0xf5,0x7b,0x63,0x20,0x13,0x0a,0xa6,0x98,0x7b,0x3f,0xa7,0xb5,0xbf,0x96,0xa8,0xae},{0xf5,0x7b,0x63,0x20,0x13,0x0a,0xa6,0x98,0x7b,0x3f,0xa7,0xb5,0xbf,0x96,0xa8,0xae},{0xdd,0xaf,0xd4,0x66,0xda,0x67,0x54,0x8d,0x1f,0xa5,0xdc,0x3c,0xdc,0x1a,0x38,0x76},
    {0xad,0xfa,0x7b,0x12,0x74,0x4d,0x31,0x53,0x0f,0x30,0xbb,0xe5,0x8b,0x12,0xd1,0x16},{0} },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds",
    {0x91,0x3b,0x7e,0x61,0x55,0x48,0x3e,0x78,0x7d,0xfb,0x2b,0xc3,0xed,0xf1,0xb8,0x9e},{0xb9,0xa4,0xfe,0x9d,0x25,0x15,0xd7,0xb6,0x6a,0x86,0x32,0x94,0x53,0x01,0xa7,0x36},{0x5f,0x1d,0xf8,0x1a,0x4f,0x43,0xe4,0x69,0x9e,0x76,0x04,0xf2,0xa9,0xe1,0x2c,0x5b},
    {0xb7,0x6f,0xba,0xa2,0x80,0xfd,0x6d,0x49,0x6f,0xfc,0x26,0xe4,0x86,0xd7,0xe5,0x82},{0xdb,0x33,0x5d,0x8f,0x5e,0x38,0x52,0x0b,0x17,0x0d,0xe5,0xbf,0xa8,0x2e,0x00,0xdb},{0x9c,0x57,0x77,0xb9,0x09,0x0e,0x99,0x2c,0xe2,0x05,0x6f,0x22,0x85,0xb5,0xe4,0xd3},
    {0},{0} },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds",
    {0xb0,0xaf,0x72,0x5a,0xd4,0x2d,0x8a,0xc9,0x09,0x4b,0x97,0x3a,0xd3,0x2d,0x56,0x7c},{0x60,0xb4,0xe5,0x25,0xed,0x5c,0xbd,0xb8,0xeb,0x91,0x30,0xb6,0x73,0x9d,0x8d,0x11},{0x18,0xe7,0xb0,0xb6,0x9c,0xfe,0xbf,0x49,0x3e,0x12,0x68,0x7e,0xc7,0x25,0x5c,0x89},
    {0x6c,0xc3,0x3c,0xac,0x83,0x5f,0x5e,0x29,0x8f,0xfd,0x9c,0x7f,0x86,0x5f,0x29,0x59},{0x51,0xef,0xb5,0x9d,0x65,0xe9,0x15,0xc5,0x43,0xe2,0xe3,0xe9,0x27,0x23,0xd0,0x5e},{0x31,0xdc,0xa2,0x63,0x5b,0xdd,0xeb,0xf2,0x1b,0x25,0xbb,0x53,0x27,0x44,0xf4,0x85},
    {0},{0} },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds",
    {0x62,0x81,0x2a,0xb7,0x78,0x4b,0x65,0xcf,0x99,0x02,0x0e,0xe4,0xb2,0x53,0xc0,0xaa},{0x32,0x44,0x97,0xc6,0xee,0x15,0x35,0xdf,0x2d,0x9d,0x05,0xed,0x3c,0xf0,0x79,0xd7},{0x77,0x08,0x4a,0x9c,0x69,0xa4,0xe3,0x09,0x5e,0xb0,0x0a,0x9e,0xfb,0x52,0x0f,0x3d},
    {0xef,0x9d,0x48,0x8e,0x27,0xfa,0x83,0xe0,0xcf,0x84,0xba,0x80,0x9b,0xa0,0x15,0x52},{0x2c,0x51,0xba,0x9e,0x1b,0xf0,0x6d,0x1b,0xcf,0x81,0x07,0xc3,0x61,0xec,0xc6,0x75},{0x27,0x0c,0xb2,0x46,0xe2,0x26,0x46,0x02,0xa2,0x65,0x37,0x05,0x61,0x67,0x27,0x5e},
    {0},{0} },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgb565.dds",
    {0x2b,0xb4,0x35,0x5f,0x72,0xca,0x52,0x82,0xd5,0xa5,0x51,0xd9,0xfb,0x53,0x97,0x63},{0x45,0x62,0x9f,0x32,0xa6,0x60,0xcb,0x30,0x96,0x61,0xf1,0x66,0x7c,0xf3,0xc5,0x22},{0xaa,0xba,0x9b,0x91,0x8d,0x72,0xbd,0x74,0xcf,0x26,0x82,0xdc,0xaf,0x23,0xde,0xff},
    {0x86,0xee,0xd0,0x05,0xd0,0xfd,0x73,0x86,0xa7,0x9c,0x88,0x17,0x9a,0x11,0x38,0x12},{0x5a,0x81,0x2d,0xab,0xa9,0xf0,0xcf,0xb6,0x3b,0x29,0x48,0xbf,0x3c,0xc9,0x53,0xfa},{0xa8,0x16,0x3d,0xc7,0x29,0xb5,0x31,0x4d,0xbf,0x14,0xec,0x92,0x7e,0x77,0x98,0xa9},
    {0},{0} },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8R8G8B8.dds",
    {0x43,0x03,0x0a,0x90,0x56,0xfb,0x4a,0xbb,0x55,0x23,0x6e,0xd0,0xb3,0x7f,0x76,0x69},{0xba,0x65,0x45,0xb8,0x5e,0xb6,0xc6,0x5b,0xb1,0x40,0xed,0xb0,0xe0,0x1e,0x56,0x16},{0x33,0x10,0xab,0xaf,0x7b,0xdb,0x42,0x06,0x27,0x1e,0x02,0x78,0x18,0xa4,0xca,0x75},
    {0xf8,0x0a,0xc0,0x27,0x04,0x3f,0x4e,0x4f,0x31,0x41,0x7d,0xca,0x60,0x23,0xb2,0x0d},{0xee,0x30,0xe8,0x93,0xa1,0xdc,0x93,0x73,0x12,0x72,0xfd,0x7f,0xb0,0x6e,0x7e,0xf4},{0x42,0x76,0x9e,0x7b,0xd0,0x24,0xf6,0x40,0x3a,0x39,0x50,0xfa,0x09,0xca,0x07,0x09},
    {0},{0} },
{ ALTMD5(1), { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_L8.dds",
    {0x54,0x57,0x84,0xbd,0x04,0x0b,0x20,0xf7,0x73,0x9e,0xee,0x6a,0xcc,0xe3,0xb6,0x36},{0x12,0xdd,0xa4,0xe3,0xc1,0xa0,0x80,0xaf,0xb1,0x4a,0xf1,0x87,0xb3,0x76,0x7b,0xfe},{0xdd,0x55,0x0a,0x50,0xf9,0x23,0x71,0xd2,0x70,0x73,0xf2,0xfd,0x0a,0x71,0x0f,0xce},
    {0x6c,0xdd,0x99,0xfa,0xdc,0xbc,0x1f,0xef,0xaa,0xc6,0xc0,0x53,0x97,0xe1,0x33,0xf4},{0x33,0x7f,0xc0,0x68,0xff,0xa8,0xdb,0xd4,0xa3,0x66,0xd6,0x13,0x35,0x69,0xec,0x1b},{0x91,0x31,0x2a,0xf0,0x3e,0xd6,0x58,0x8f,0x8a,0x32,0x24,0x6e,0x00,0x0c,0x6e,0x98},
    {0},{0} },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_r16f.dds",
    {0x38,0x05,0xa1,0x3e,0x4e,0xc8,0x26,0xfb,0x7d,0x55,0xaa,0xab,0x33,0x86,0x7a,0x5c},{0x52,0x23,0x56,0x78,0xfa,0xf7,0x50,0xd6,0x85,0xa4,0xc3,0x74,0xba,0x6e,0x83,0xfc},{0x10,0x9d,0x2b,0x27,0x37,0xce,0x5c,0x97,0x44,0x08,0xad,0x2f,0xa3,0xde,0xab,0xfd},
    {0x7a,0x39,0xa8,0xc9,0xcc,0x93,0xa8,0x19,0xb7,0x03,0xbf,0x2c,0x78,0x60,0xf0,0x2c},{0x7a,0x39,0xa8,0xc9,0xcc,0x93,0xa8,0x19,0xb7,0x03,0xbf,0x2c,0x78,0x60,0xf0,0x2c},{0xa1,0x1e,0xa0,0x45,0x39,0xa9,0xf7,0x26,0x57,0xf1,0xca,0x51,0x65,0x7c,0xdc,0x26},
    {0},{0} },
{ FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_r32f.dds",
    {0xf5,0x0f,0xe9,0x44,0xe8,0x6b,0x1f,0x51,0x52,0xf8,0xd5,0x8f,0x9f,0x94,0x65,0x30},{0xf3,0x8f,0x93,0x9c,0xfa,0x7e,0x67,0x51,0xbd,0x4f,0x61,0x1e,0x29,0x4d,0x14,0xdd},{0xbf,0xb5,0x0c,0xf0,0x81,0xc4,0xf4,0x08,0x7d,0xf7,0xd4,0x8e,0xbb,0x21,0x77,0xdc},
    {0xe8,0xc5,0xa8,0xdb,0x03,0x47,0x40,0x2d,0x3a,0x3c,0xdb,0xc6,0x5f,0x8b,0x13,0xf3},{0xe8,0xc5,0xa8,0xdb,0x03,0x47,0x40,0x2d,0x3a,0x3c,0xdb,0xc6,0x5f,0x8b,0x13,0xf3},{0x1f,0x6d,0xf0,0x54,0xe7,0x8c,0x55,0xe6,0xc6,0x05,0x3f,0x57,0x91,0x46,0xa0,0x22},
    {0},{0} },
{ FLAGS_SEPALPHA, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16f.dds",
    {0x36,0x67,0xb6,0x2e,0x69,0xae,0x49,0x02,0xc3,0xc5,0xb7,0x7e,0x00,0x9b,0x57,0x72},{0x0a,0x48,0x1a,0x96,0xef,0xe4,0xdc,0xf1,0x43,0x57,0x5f,0x13,0xa6,0x1c,0xe6,0x60},{0x32,0x43,0x55,0x82,0x6a,0xf5,0x30,0x96,0x63,0x6b,0x07,0xe6,0x85,0x91,0x6a,0xfc},
    {0xb8,0x76,0x6c,0x77,0xb1,0x0c,0x2d,0x43,0xed,0x9c,0x94,0x28,0x04,0xc8,0x51,0x57},{0xb8,0x76,0x6c,0x77,0xb1,0x0c,0x2d,0x43,0xed,0x9c,0x94,0x28,0x04,0xc8,0x51,0x57},{0x4a,0x6d,0x1d,0xd3,0x13,0xa2,0x99,0x25,0x7e,0x62,0x9a,0xdb,0xa6,0x94,0x83,0x3b},
    {0},{0x36,0x67,0xb6,0x2e,0x69,0xae,0x49,0x02,0xc3,0xc5,0xb7,0x7e,0x00,0x9b,0x57,0x72} },
{ ALTMD5(8) | FLAGS_SEPALPHA, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba32f.dds",
    {0xe3,0x50,0xe6,0xaa,0xb9,0xf7,0xe9,0x20,0x34,0x4e,0xd7,0x2e,0x11,0xcb,0x33,0x8e},{0x84,0x4f,0x12,0x7e,0x10,0x5a,0x61,0x7c,0xac,0x52,0x8b,0xee,0xdb,0xbd,0x9b,0x64},{0x47,0x1c,0x68,0xb3,0x58,0x43,0xc2,0x03,0xca,0xfb,0x31,0x75,0xf1,0xe9,0xc6,0xd6},
    {0x35,0x9c,0xd7,0x75,0x92,0xf4,0x2c,0x64,0xc2,0x41,0x2e,0x53,0x84,0xf7,0x39,0x10},{0x35,0x9c,0xd7,0x75,0x92,0xf4,0x2c,0x64,0xc2,0x41,0x2e,0x53,0x84,0xf7,0x39,0x10},{0xca,0xc0,0xf8,0x9a,0x21,0xbc,0x7b,0x7d,0x74,0x44,0xc1,0xf8,0xd0,0x01,0xd8,0xca},
    {0},{0x43,0x5c,0x6b,0x72,0x23,0x14,0x0b,0xd1,0x3e,0x87,0x91,0x55,0x11,0xdd,0xe9,0x71} },
{ ALTMD5(2), { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds",
    {0xcd,0x89,0xb3,0x80,0xce,0xd1,0x94,0xdf,0xa8,0xaf,0xfc,0x1b,0x2f,0x61,0x4e,0x42},{0x38,0xb3,0xd7,0xfb,0xcb,0x07,0xe1,0x59,0xbb,0x69,0x02,0xb9,0x70,0xb5,0xd3,0x51},{0xcd,0xb7,0xba,0xb8,0x34,0xde,0x92,0x95,0x26,0x69,0xd4,0xf2,0x75,0xf0,0x56,0x0e},
    {0x79,0x5f,0x31,0xf9,0xbf,0xd1,0xcb,0x84,0x4b,0x3a,0x2f,0x18,0xa6,0x21,0x7f,0xa4},{0x63,0xb9,0x47,0x1e,0x16,0xb3,0x2b,0x6f,0x03,0xa2,0xec,0x84,0xa6,0xfe,0xf0,0x59},{0xb2,0x18,0x6b,0x86,0xf4,0xf1,0x95,0xcc,0x97,0x97,0x89,0x76,0xea,0x76,0x6e,0xea},
    {0},{0} },
#ifdef _M_X64
{ ALTMD5(3), { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_L16.dds",
    {0xdb,0x61,0xa1,0x83,0x3d,0xb3,0xc6,0xc0,0xae,0x9d,0xd0,0x29,0xb6,0xda,0xf4,0xd6},{0xd3,0x50,0x90,0x6c,0xce,0xe3,0x5d,0xcc,0x27,0x76,0xb2,0x10,0x25,0xba,0x92,0x42},{0xa6,0xc0,0x03,0xde,0xdc,0xa2,0x18,0x67,0x49,0xa7,0xf5,0x34,0xe7,0x84,0x67,0x50},
    {0},{0xfb,0x85,0x7c,0xc2,0x51,0xdf,0x62,0x3a,0x74,0xe4,0x69,0xd9,0xca,0xe1,0xc5,0xd4},{0xc0,0xc2,0x02,0xe2,0xf7,0x86,0x75,0x81,0x10,0x17,0x57,0x2e,0xdf,0x53,0xae,0x61},
    {0},{0} },
{ ALTMD5(7), { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds",
    {0x3c,0x7a,0xf6,0x09,0xab,0x04,0x8d,0x6e,0x01,0x73,0x18,0xc1,0xed,0x48,0xdc,0xbd},{0xdd,0xd6,0xe9,0x5c,0xd1,0xb0,0xcb,0x9f,0x16,0x91,0xc7,0x81,0x67,0x52,0x38,0x2e},{0xef,0x24,0x6a,0x78,0x69,0x9f,0x6a,0x12,0xb8,0x80,0x30,0x28,0x04,0x68,0xf5,0x94},
    {0},{0x8f,0xd0,0xce,0x8d,0x71,0x6c,0x05,0x4c,0x13,0x77,0xa4,0x54,0x23,0x9f,0x51,0x19},{0x38,0x60,0xb3,0x2e,0x30,0x23,0xfa,0x8b,0xed,0xc4,0x39,0x25,0xbe,0x74,0x6b,0xe8},
    {0},{0} },
{ALTMD5(9) | FLAGS_SKIP_POINTNOWIC | FLAGS_SKIP_SRGB, { 1920, 1200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"abstract-test-pattern.jpg",
    {0xfd,0x30,0x62,0xc0,0x46,0x8e,0xd4,0xb2,0xfb,0xbf,0x88,0x41,0x02,0x0f,0x19,0x45},{0xa3,0xc0,0x6a,0xf0,0x66,0x97,0x18,0x99,0xb2,0x11,0x44,0x37,0xd9,0x9e,0xf4,0x3b},{0x99,0x04,0x43,0xe1,0xe7,0x76,0x23,0x25,0x69,0xb6,0x63,0xad,0x10,0xe9,0x80,0x05},
    {0xa5,0x2d,0xb3,0xff,0x24,0xaf,0x5e,0x3b,0x88,0x9b,0xbe,0x32,0xc6,0x01,0x1c,0xc4},{0x51,0x01,0xd6,0x19,0xf7,0x04,0xbd,0x6b,0x19,0xca,0x02,0x4b,0x94,0xad,0x65,0x46},{0x6b,0x95,0x0a,0x18,0xe9,0xea,0x3f,0x15,0xdc,0xfb,0x07,0xe4,0x0b,0xd7,0x4f,0x9b},
    {0},{0} },
{ FLAGS_SKIP_SRGB, { 256, 224, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"deltae_base.png",
    {0x3e,0x86,0x13,0x4e,0x01,0x4e,0x25,0xf5,0x5b,0xc1,0xd9,0xb1,0x9d,0x8c,0xad,0xb5},{0xd1,0xea,0x06,0xb3,0xe9,0x58,0xc2,0xb9,0x9b,0x5b,0xdc,0x31,0x4b,0xe0,0x6f,0xe8},{0x2c,0xfe,0xce,0x61,0x7c,0x72,0x30,0x17,0x76,0xea,0xcc,0x2f,0x90,0x01,0x0b,0xa6},
    {0x9f,0x30,0x55,0xeb,0x94,0x0b,0xe3,0xa9,0x01,0xbd,0x63,0xe7,0x01,0xf5,0x1a,0xd8},{0x5b,0x83,0xe5,0x4d,0x44,0x68,0x18,0xd8,0xf2,0x02,0xb7,0xbc,0x13,0x9e,0x5d,0xcf},{0x62,0x5c,0x2c,0x6a,0x89,0x74,0xe5,0x82,0xac,0x92,0xc5,0xc4,0x28,0xdf,0x17,0x7c},
    {0},{0} },
{ FLAGS_SKIP_SRGB, { 768, 576, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Grayscale_Staircase.png",
    {0x52,0x9f,0x21,0xe3,0x7c,0xc4,0x0f,0xe7,0xf3,0xc4,0x49,0x41,0xe7,0x52,0x57,0x08},{0xb5,0x23,0x11,0x72,0x51,0xba,0x79,0x3a,0x0c,0xc4,0x9c,0x9d,0x6d,0xa5,0x19,0x56},{0x8a,0x8a,0x36,0x72,0x4a,0xee,0xae,0x64,0xc2,0xeb,0x5c,0xcc,0xd4,0xc9,0x65,0x35},
    {0xa4,0xf7,0xa7,0x73,0x01,0x52,0xc8,0x7b,0x1b,0x01,0xb1,0x98,0x64,0x00,0xc6,0x9a},{0xf0,0xe9,0xa4,0x0e,0x3d,0xd3,0x15,0xe7,0x33,0x94,0x7c,0xa7,0xbd,0xa3,0x29,0x1e},{0xf0,0xdd,0xc3,0x5d,0xc7,0x5d,0x3e,0x30,0x88,0xb9,0x68,0xc7,0x7b,0xa0,0x65,0x17},
    {0},{0} },
{ FLAGS_SKIP_TOPTEST | ALTMD5(4) | ALTMD5B(10) | ALTMD5C(11), { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testpatternYUY2.dds",
    {0x9c,0x8b,0xa8,0x78,0xe5,0x6f,0x13,0x37,0xb1,0x01,0x67,0x34,0x8c,0x5b,0x40,0x02},{0x35,0x3f,0xc7,0x91,0xec,0x8e,0xbf,0x9d,0x21,0x15,0x82,0x1b,0x19,0x68,0xfa,0xac},{0xe8,0x6b,0xf4,0x36,0x3d,0x7c,0x3b,0x00,0xd3,0x09,0xbe,0x5e,0x8b,0xf0,0x7a,0x50},
    {0x1a,0xfb,0xbf,0x35,0xda,0x46,0xef,0xa4,0x34,0x94,0x58,0x5c,0x1d,0x41,0xc9,0x0b},{0x8d,0x79,0x59,0x08,0x26,0x11,0xeb,0x89,0x7a,0x96,0x7e,0xaa,0xcc,0xfc,0x39,0x7e},{0x92,0x01,0xe8,0x5c,0x37,0xa4,0x6d,0xeb,0xcd,0x9c,0x72,0x06,0x6b,0x14,0xd2,0x78},
    {0},{0} },
{ FLAGS_SKIP_TOPTEST | ALTMD5(5) | ALTMD5B(12) | ALTMD5C(13), { 200, 200, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaCubeYUY2.dds",
    {0xfc,0x05,0x3d,0xbb,0xb2,0x1d,0xb4,0x2a,0x47,0x7f,0x3b,0x2e,0x3e,0x08,0x90,0x1e},{0x5c,0xdc,0xf0,0x6d,0x13,0x23,0x1c,0xa1,0xbd,0xbc,0x58,0x88,0x23,0xbe,0x09,0xa9},{0xdd,0xbe,0x18,0x6b,0xa9,0x95,0x00,0x5a,0x1c,0x5b,0x2f,0x92,0x63,0xa5,0x19,0x42},
    {0x19,0xf2,0x05,0x88,0xe9,0x97,0xf3,0x07,0xf9,0x89,0xbb,0x3a,0x53,0x1c,0x03,0x45},{0x71,0x81,0xf8,0x8c,0x4d,0xf1,0x86,0xcb,0x37,0x9d,0xd8,0x98,0xa9,0xcf,0xe2,0x13},{0xc3,0x04,0xae,0xe4,0x84,0x02,0xe9,0x79,0x62,0x6f,0x6e,0xdb,0xf7,0x1e,0x65,0xa9},
    {0xb1,0x60,0x60,0x4b,0x1f,0xd0,0xc9,0x2b,0x09,0xd3,0xd7,0x7d,0xaf,0x11,0x4b,0xec},{0} },
#else
{ ALTMD5(3), { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_L16.dds",
    {0xdb,0x61,0xa1,0x83,0x3d,0xb3,0xc6,0xc0,0xae,0x9d,0xd0,0x29,0xb6,0xda,0xf4,0xd6},{0xd3,0x50,0x90,0x6c,0xce,0xe3,0x5d,0xcc,0x27,0x76,0xb2,0x10,0x25,0xba,0x92,0x42},{0xa6,0xc0,0x03,0xde,0xdc,0xa2,0x18,0x67,0x49,0xa7,0xf5,0x34,0xe7,0x84,0x67,0x50},
    {0xfb,0x85,0x7c,0xc2,0x51,0xdf,0x62,0x3a,0x74,0xe4,0x69,0xd9,0xca,0xe1,0xc5,0xd4},{0},{0xc0,0xc2,0x02,0xe2,0xf7,0x86,0x75,0x81,0x10,0x17,0x57,0x2e,0xdf,0x53,0xae,0x61},
    {0},{0} },
{ ALTMD5(7), { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds",
    {0x3c,0x7a,0xf6,0x09,0xab,0x04,0x8d,0x6e,0x01,0x73,0x18,0xc1,0xed,0x48,0xdc,0xbd},{0xdd,0xd6,0xe9,0x5c,0xd1,0xb0,0xcb,0x9f,0x16,0x91,0xc7,0x81,0x67,0x52,0x38,0x2e},{0xef,0x24,0x6a,0x78,0x69,0x9f,0x6a,0x12,0xb8,0x80,0x30,0x28,0x04,0x68,0xf5,0x94},
    {0x8f,0xd0,0xce,0x8d,0x71,0x6c,0x05,0x4c,0x13,0x77,0xa4,0x54,0x23,0x9f,0x51,0x19},{0},{0x38,0x60,0xb3,0x2e,0x30,0x23,0xfa,0x8b,0xed,0xc4,0x39,0x25,0xbe,0x74,0x6b,0xe8},
    {0},{0} },
{ ALTMD5(9) | FLAGS_SKIP_POINTNOWIC | FLAGS_SKIP_SRGB, { 1920, 1200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"abstract-test-pattern.jpg",
    {0xfd,0x30,0x62,0xc0,0x46,0x8e,0xd4,0xb2,0xfb,0xbf,0x88,0x41,0x02,0x0f,0x19,0x45},{0xa3,0xc0,0x6a,0xf0,0x66,0x97,0x18,0x99,0xb2,0x11,0x44,0x37,0xd9,0x9e,0xf4,0x3b},{0x99,0x04,0x43,0xe1,0xe7,0x76,0x23,0x25,0x69,0xb6,0x63,0xad,0x10,0xe9,0x80,0x05},
    {0xa5,0x2d,0xb3,0xff,0x24,0xaf,0x5e,0x3b,0x88,0x9b,0xbe,0x32,0xc6,0x01,0x1c,0xc4},{0x51,0x01,0xd6,0x19,0xf7,0x04,0xbd,0x6b,0x19,0xca,0x02,0x4b,0x94,0xad,0x65,0x46},{0x6b,0x95,0x0a,0x18,0xe9,0xea,0x3f,0x15,0xdc,0xfb,0x07,0xe4,0x0b,0xd7,0x4f,0x9b},
    {0},{0} },
{ FLAGS_SKIP_SRGB, { 256, 224, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"deltae_base.png",
    {0x3e,0x86,0x13,0x4e,0x01,0x4e,0x25,0xf5,0x5b,0xc1,0xd9,0xb1,0x9d,0x8c,0xad,0xb5},{0xd1,0xea,0x06,0xb3,0xe9,0x58,0xc2,0xb9,0x9b,0x5b,0xdc,0x31,0x4b,0xe0,0x6f,0xe8},{0x2c,0xfe,0xce,0x61,0x7c,0x72,0x30,0x17,0x76,0xea,0xcc,0x2f,0x90,0x01,0x0b,0xa6},
    {0x9f,0x30,0x55,0xeb,0x94,0x0b,0xe3,0xa9,0x01,0xbd,0x63,0xe7,0x01,0xf5,0x1a,0xd8},{0x5b,0x83,0xe5,0x4d,0x44,0x68,0x18,0xd8,0xf2,0x02,0xb7,0xbc,0x13,0x9e,0x5d,0xcf},{0x62,0x5c,0x2c,0x6a,0x89,0x74,0xe5,0x82,0xac,0x92,0xc5,0xc4,0x28,0xdf,0x17,0x7c},
    {0},{0} },
{ FLAGS_SKIP_SRGB, { 768, 576, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Grayscale_Staircase.png",
    {0x52,0x9f,0x21,0xe3,0x7c,0xc4,0x0f,0xe7,0xf3,0xc4,0x49,0x41,0xe7,0x52,0x57,0x08},{0xb5,0x23,0x11,0x72,0x51,0xba,0x79,0x3a,0x0c,0xc4,0x9c,0x9d,0x6d,0xa5,0x19,0x56},{0x8a,0x8a,0x36,0x72,0x4a,0xee,0xae,0x64,0xc2,0xeb,0x5c,0xcc,0xd4,0xc9,0x65,0x35},
    {0xa4,0xf7,0xa7,0x73,0x01,0x52,0xc8,0x7b,0x1b,0x01,0xb1,0x98,0x64,0x00,0xc6,0x9a},{0xf0,0xe9,0xa4,0x0e,0x3d,0xd3,0x15,0xe7,0x33,0x94,0x7c,0xa7,0xbd,0xa3,0x29,0x1e},{0xf0,0xdd,0xc3,0x5d,0xc7,0x5d,0x3e,0x30,0x88,0xb9,0x68,0xc7,0x7b,0xa0,0x65,0x17},
    {0},{0} },
{ FLAGS_SKIP_TOPTEST | ALTMD5(4) | ALTMD5B(10) | ALTMD5C(11), { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testpatternYUY2.dds",
    {0x9c,0x8b,0xa8,0x78,0xe5,0x6f,0x13,0x37,0xb1,0x01,0x67,0x34,0x8c,0x5b,0x40,0x02},{0x35,0x3f,0xc7,0x91,0xec,0x8e,0xbf,0x9d,0x21,0x15,0x82,0x1b,0x19,0x68,0xfa,0xac},{0xe8,0x6b,0xf4,0x36,0x3d,0x7c,0x3b,0x00,0xd3,0x09,0xbe,0x5e,0x8b,0xf0,0x7a,0x50},
    {0x1a,0xfb,0xbf,0x35,0xda,0x46,0xef,0xa4,0x34,0x94,0x58,0x5c,0x1d,0x41,0xc9,0x0b},{0x8d,0x79,0x59,0x08,0x26,0x11,0xeb,0x89,0x7a,0x96,0x7e,0xaa,0xcc,0xfc,0x39,0x7e},{0x92,0x01,0xe8,0x5c,0x37,0xa4,0x6d,0xeb,0xcd,0x9c,0x72,0x06,0x6b,0x14,0xd2,0x78},
    {0},{0} },
{ FLAGS_SKIP_TOPTEST | ALTMD5(5) | ALTMD5B(12) | ALTMD5C(13), { 200, 200, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaCubeYUY2.dds",
    {0xfc,0x05,0x3d,0xbb,0xb2,0x1d,0xb4,0x2a,0x47,0x7f,0x3b,0x2e,0x3e,0x08,0x90,0x1e},{0x5c,0xdc,0xf0,0x6d,0x13,0x23,0x1c,0xa1,0xbd,0xbc,0x58,0x88,0x23,0xbe,0x09,0xa9},{0xdd,0xbe,0x18,0x6b,0xa9,0x95,0x00,0x5a,0x1c,0x5b,0x2f,0x92,0x63,0xa5,0x19,0x42},
    {0x19,0xf2,0x05,0x88,0xe9,0x97,0xf3,0x07,0xf9,0x89,0xbb,0x3a,0x53,0x1c,0x03,0x45},{0x71,0x81,0xf8,0x8c,0x4d,0xf1,0x86,0xcb,0x37,0x9d,0xd8,0x98,0xa9,0xcf,0xe2,0x13},{0xc3,0x04,0xae,0xe4,0x84,0x02,0xe9,0x79,0x62,0x6f,0x6e,0xdb,0xf7,0x1e,0x65,0xa9},
    {0xb1,0x60,0x60,0x4b,0x1f,0xd0,0xc9,0x2b,0x09,0xd3,0xd7,0x7d,0xaf,0x11,0x4b,0xec},{0} },
#endif

{ FLAGS_NONE, { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D  }, MEDIA_PATH L"testpattern.png",
    {0x7e,0xc4,0xf2,0x47,0x21,0xae,0x2e,0x5e,0x9f,0x84,0x27,0xc3,0x33,0xcc,0xe4,0x6f},{0xa6,0x5b,0xff,0x02,0xc3,0x66,0xbf,0xb8,0x57,0x18,0x4c,0x67,0xe8,0x1b,0x05,0x45},{0x7e,0xc4,0xf2,0x47,0x21,0xae,0x2e,0x5e,0x9f,0x84,0x27,0xc3,0x33,0xcc,0xe4,0x6f},
    {0x2b,0x31,0x6a,0x08,0x6c,0x2d,0x16,0x19,0x8a,0x7c,0x05,0x3f,0x91,0x3e,0x18,0x5d},{0x2b,0x31,0x6a,0x08,0x6c,0x2d,0x16,0x19,0x8a,0x7c,0x05,0x3f,0x91,0x3e,0x18,0x5d},{0x58,0x80,0xab,0xb4,0xf8,0xea,0xac,0xed,0x47,0xb5,0xf6,0xbd,0x4a,0xef,0xb5,0x99},
    {0},{0} },

{ FLAGS_SEPALPHA | FLAGS_SKIP_SRGB, { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds",
    {0x20,0xfb,0x52,0x0c,0x92,0x5b,0xb7,0x1c,0xfc,0x61,0xe8,0x27,0xf6,0xff,0x26,0x24},{0xa7,0x8e,0xd0,0x4c,0x28,0x86,0x93,0xe8,0xac,0xcc,0x43,0x6e,0x1b,0x49,0x36,0x7b},{0x36,0xad,0x30,0x2f,0xf2,0xed,0x43,0xd3,0xca,0xa6,0x6b,0xf7,0x81,0x8c,0xb8,0xc1},
    {0x94,0x6f,0xd7,0xa2,0x62,0x90,0x12,0x88,0x1d,0x6b,0x51,0x02,0xe7,0xaf,0x47,0xbb},{0x4c,0x24,0x0f,0xa4,0xcf,0x93,0x39,0x61,0x55,0x09,0x98,0x5f,0x16,0x7a,0xd1,0x20},{0xd4,0x05,0xfb,0xb4,0xd5,0x49,0x16,0xab,0x6b,0x7c,0xf1,0xe1,0xfe,0xf1,0x45,0xf2},
    {0x20,0xfb,0x52,0x0c,0x92,0x5b,0xb7,0x1c,0xfc,0x61,0xe8,0x27,0xf6,0xff,0x26,0x24},{0xb2,0x0b,0x5f,0x67,0xa8,0xd5,0xae,0x7d,0x4e,0xb7,0x11,0xdc,0x6b,0xac,0x40,0x7f} },
{ FLAGS_SKIP_SRGB, { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SplashScreen2.dds",
    {0x65,0x86,0xbb,0xb8,0x14,0xdd,0xd0,0xba,0x12,0x33,0x02,0xcd,0xc2,0x7f,0xd0,0x57},{0xf9,0xd3,0x35,0xae,0x28,0x61,0xfe,0x03,0x4e,0x7d,0xf3,0x53,0xbc,0xee,0xee,0x30},{0x4e,0x58,0x0e,0x8c,0xfd,0xee,0xcd,0x47,0x38,0xe4,0xd4,0x63,0x7c,0x5c,0x50,0x26},
    {0x68,0x27,0x36,0x8f,0x5e,0x26,0x3a,0x2f,0x20,0x8c,0x05,0xab,0xa0,0x64,0x9f,0xfc},{0x5d,0xbe,0xa6,0x1b,0xf2,0xd1,0x1b,0x38,0x7c,0x91,0x3c,0xf0,0x37,0x3f,0x04,0x4a},{0x63,0xe9,0x29,0x1f,0x59,0x31,0x50,0xb7,0xc0,0x0d,0xa9,0xfe,0x85,0x87,0x4d,0x0b},
    {0},{0} },
{ FLAGS_SEPALPHA | FLAGS_SKIP_SRGB, { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.dds",
    {0x5b,0x95,0xe5,0x19,0x22,0xe6,0x18,0x08,0xf4,0xd2,0xee,0xf2,0xe5,0x36,0x4a,0x22},{0x75,0x38,0x7f,0x9a,0xcc,0x57,0x66,0xfc,0x1d,0x35,0x26,0x37,0xa9,0x5d,0x81,0xb1},{0x45,0x48,0xfb,0xca,0xda,0xd4,0x49,0xe4,0x4a,0xc2,0x12,0xb8,0xae,0xeb,0x4f,0x1d},
    {0xcd,0xfc,0x45,0x42,0x21,0x99,0x52,0xe1,0x13,0x37,0xf7,0xfa,0xc6,0x84,0xf3,0x38},{0x08,0xaa,0x1e,0x81,0x31,0x2e,0xda,0xa0,0x00,0x5e,0x03,0x68,0x71,0x0d,0x09,0x30},{0x6f,0xfd,0x29,0x25,0x3f,0xb4,0x4e,0x66,0xc6,0xaf,0x03,0xc7,0x61,0xa4,0x2f,0xab},
    {0x5b,0x95,0xe5,0x19,0x22,0xe6,0x18,0x08,0xf4,0xd2,0xee,0xf2,0xe5,0x36,0x4a,0x22},{0x27,0xca,0x2b,0xd2,0x69,0x8f,0x95,0xa0,0xcd,0xb1,0x72,0xd1,0xe8,0xf2,0xdf,0xb6} },

#ifdef _M_X64
// Very large images
// TODO -
{ FLAGS_NONE, { 16384, 16384, 1, 1, 15, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k.dds" },
{ FLAGS_NONE, { 16384, 16384, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_SNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k_snorm.dds" },
#endif
};

static const MipMapMedia g_MipMapMedia3D[] = 
{

// <source> test-options | width height depth arraySize mipLevels miscFlags format dimension | filename
#ifdef _M_X64
{ FLAGS_NONE, { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds",
    {0xc7,0xd7,0xba,0xe7,0x25,0x67,0x27,0x3a,0x1f,0x8f,0xae,0x50,0xe5,0x6c,0x92,0x90},{0xc4,0xd7,0x16,0xfb,0x44,0xfc,0xe6,0xb0,0x83,0x41,0x3f,0x18,0x55,0x2f,0x45,0x67},{0x2e,0xff,0xed,0xfc,0x27,0x45,0xd1,0xfd,0xb4,0x36,0x7a,0xb5,0x1b,0x9d,0x6c,0x2d},
    {0},{0x12,0xe6,0xaa,0x09,0x26,0xa3,0x93,0x57,0x79,0xb2,0x35,0x10,0x19,0xc5,0xa8,0x8f},{0x17,0xca,0x6f,0x32,0xfa,0x49,0x39,0x1d,0xec,0x92,0xdb,0x66,0x3a,0x42,0xef,0xc3},
    {0xc7,0xd7,0xba,0xe7,0x25,0x67,0x27,0x3a,0x1f,0x8f,0xae,0x50,0xe5,0x6c,0x92,0x90},{0} },
{ FLAGS_NONE, { 200, 200, 3, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvolip.dds",
    {0x0b,0x45,0xc2,0xd1,0xcc,0xb0,0xb7,0x68,0x56,0x2a,0x6e,0x87,0x42,0x59,0x55,0x5c},{0xc7,0x2b,0xc6,0x94,0x0d,0xe7,0x1b,0xd3,0x20,0x2a,0x7f,0x09,0x2a,0x4a,0xec,0xa8},{0x45,0xe8,0x5b,0x6a,0x75,0x9e,0x1a,0xba,0xc0,0x7e,0x62,0xd9,0xff,0x75,0x54,0x72},
    {0},{0xd3,0x12,0xed,0x48,0x0a,0x56,0x25,0x96,0x62,0x53,0xec,0x4c,0x61,0xfe,0x31,0x61},{0x0b,0x45,0xc2,0xd1,0xcc,0xb0,0xb7,0x68,0x56,0x2a,0x6e,0x87,0x42,0x59,0x55,0x5c},
    {0x0b,0x45,0xc2,0xd1,0xcc,0xb0,0xb7,0x68,0x56,0x2a,0x6e,0x87,0x42,0x59,0x55,0x5c},{0} },
{ ALTMD5(6), { 200, 200, 4, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"lenaVolYUY2.dds",
    {0x86,0xb5,0x4b,0x9e,0x4d,0xa1,0xb5,0x37,0xaf,0xb8,0x5d,0x1f,0x09,0x23,0xbb,0x38},{0x1c,0x0c,0x42,0xa5,0x1d,0x7d,0xdc,0x06,0x0c,0x5e,0x57,0xd4,0x75,0xc4,0x24,0x73},{0xf4,0xae,0x72,0xf2,0xc7,0x85,0x45,0x7c,0xf8,0x86,0x0f,0xde,0xa4,0x7a,0x9d,0x45},
    {0},{0x1c,0xe3,0xd4,0xbd,0x62,0x11,0xe9,0x1c,0xbd,0xd7,0x8d,0xab,0x65,0x39,0x91,0xf5},{0x86,0xb5,0x4b,0x9e,0x4d,0xa1,0xb5,0x37,0xaf,0xb8,0x5d,0x1f,0x09,0x23,0xbb,0x38},
    {0x86,0xb5,0x4b,0x9e,0x4d,0xa1,0xb5,0x37,0xaf,0xb8,0x5d,0x1f,0x09,0x23,0xbb,0x38},{0} },
#else
{ FLAGS_NONE, { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds",
    {0xc7,0xd7,0xba,0xe7,0x25,0x67,0x27,0x3a,0x1f,0x8f,0xae,0x50,0xe5,0x6c,0x92,0x90},{0xc4,0xd7,0x16,0xfb,0x44,0xfc,0xe6,0xb0,0x83,0x41,0x3f,0x18,0x55,0x2f,0x45,0x67},{0x2e,0xff,0xed,0xfc,0x27,0x45,0xd1,0xfd,0xb4,0x36,0x7a,0xb5,0x1b,0x9d,0x6c,0x2d},
    {0x12,0xe6,0xaa,0x09,0x26,0xa3,0x93,0x57,0x79,0xb2,0x35,0x10,0x19,0xc5,0xa8,0x8f},{0},{0x17,0xca,0x6f,0x32,0xfa,0x49,0x39,0x1d,0xec,0x92,0xdb,0x66,0x3a,0x42,0xef,0xc3},
    {0xc7,0xd7,0xba,0xe7,0x25,0x67,0x27,0x3a,0x1f,0x8f,0xae,0x50,0xe5,0x6c,0x92,0x90},{0} },
{ FLAGS_NONE, { 200, 200, 3, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvolip.dds",
    {0x0b,0x45,0xc2,0xd1,0xcc,0xb0,0xb7,0x68,0x56,0x2a,0x6e,0x87,0x42,0x59,0x55,0x5c},{0xc7,0x2b,0xc6,0x94,0x0d,0xe7,0x1b,0xd3,0x20,0x2a,0x7f,0x09,0x2a,0x4a,0xec,0xa8},{0x45,0xe8,0x5b,0x6a,0x75,0x9e,0x1a,0xba,0xc0,0x7e,0x62,0xd9,0xff,0x75,0x54,0x72},
    {0xd3,0x12,0xed,0x48,0x0a,0x56,0x25,0x96,0x62,0x53,0xec,0x4c,0x61,0xfe,0x31,0x61},{0},{0x0b,0x45,0xc2,0xd1,0xcc,0xb0,0xb7,0x68,0x56,0x2a,0x6e,0x87,0x42,0x59,0x55,0x5c},
    {0x0b,0x45,0xc2,0xd1,0xcc,0xb0,0xb7,0x68,0x56,0x2a,0x6e,0x87,0x42,0x59,0x55,0x5c},{0} },
{ ALTMD5(6), { 200, 200, 4, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"lenaVolYUY2.dds",
    {0x86,0xb5,0x4b,0x9e,0x4d,0xa1,0xb5,0x37,0xaf,0xb8,0x5d,0x1f,0x09,0x23,0xbb,0x38},{0x1c,0x0c,0x42,0xa5,0x1d,0x7d,0xdc,0x06,0x0c,0x5e,0x57,0xd4,0x75,0xc4,0x24,0x73},0xf4,0xae,0x72,0xf2,0xc7,0x85,0x45,0x7c,0xf8,0x86,0x0f,0xde,0xa4,0x7a,0x9d,0x45,
    {0x1c,0xe3,0xd4,0xbd,0x62,0x11,0xe9,0x1c,0xbd,0xd7,0x8d,0xab,0x65,0x39,0x91,0xf5},{0},{0x86,0xb5,0x4b,0x9e,0x4d,0xa1,0xb5,0x37,0xaf,0xb8,0x5d,0x1f,0x09,0x23,0xbb,0x38},
    {0x86,0xb5,0x4b,0x9e,0x4d,0xa1,0xb5,0x37,0xaf,0xb8,0x5d,0x1f,0x09,0x23,0xbb,0x38},{0} },
#endif
};

//-------------------------------------------------------------------------------------

struct AltMD5
{
    uint8_t md5[16];
    uint8_t md5_point[16];
    uint8_t md5_linear[16];
    uint8_t md5_cubic[16];
    uint8_t md5_tri[16];
    uint8_t md5_c[16];
    uint8_t md5_sepalpha[16];
};

static const AltMD5 g_AltMD5[] =
{
    { {0},{0},{0},
      {0},{0x85,0x69,0x79,0x1e,0x13,0xe7,0x92,0x54,0x87,0xbe,0x3f,0x7e,0xa2,0x0f,0x8d,0x46},
      {0},{0} }, // ALTMD5(1)
    { {0},{0},{0},
      {0},{0x3f,0xeb,0x05,0x51,0xa3,0x86,0xc9,0xa5,0x73,0xf8,0x48,0x53,0x33,0x7e,0x1e,0xa6},
      {0},{0} }, // ALTMD5(2)
    { {0},{0},{0x0c,0xd9,0x1c,0x5e,0x7b,0xb7,0x16,0xc5,0xb4,0x17,0x18,0x5f,0xdd,0xeb,0xcc,0x97},
      {0},{0x5d,0x36,0x51,0xe7,0x56,0xb2,0x44,0x19,0xba,0xec,0x78,0xd9,0x4a,0xec,0xcf,0x05},
      {0},{0} }, // ALTMD5(3)
    { {0x1e,0x46,0x16,0x16,0xd9,0xb3,0x9b,0xb3,0x5b,0x45,0xca,0x28,0x56,0x5b,0x36,0x4b},{0},{0},
      {0},{0x78,0x51,0xba,0x7e,0x5c,0x81,0xee,0x68,0xda,0xca,0x18,0xe5,0x29,0xab,0x72,0xb5},
      {0},{0} }, // ALTMD5(4)
    { {0x37,0xe0,0x47,0x39,0x2a,0xef,0x29,0x7c,0xa0,0xc8,0x2c,0x21,0x15,0x7b,0x9d,0x95},{0},{0},
      {0},{0x07,0x4c,0x49,0x49,0xc1,0xbf,0x18,0xed,0x16,0x35,0x2d,0x68,0xe3,0x51,0xc9,0x09},
      {0x8b,0x81,0x6b,0x8a,0x59,0xbf,0x91,0x35,0x90,0x58,0x3c,0x24,0x06,0x07,0x5e,0xd6},{0} }, // ALTMD5(5)
    { {0xed,0xfa,0x37,0x45,0xec,0x8f,0xad,0x59,0xb9,0xdc,0x6e,0x6a,0xb4,0xb3,0x11,0xa7},{0},{0xe9,0x37,0x7c,0x71,0xb8,0x9e,0x0b,0x2d,0xd8,0x3f,0xec,0xd3,0x5f,0xbd,0xc0,0x52},
      {0xcb,0x69,0xca,0xa2,0xcb,0x90,0x3a,0xd6,0x18,0xaa,0x38,0x7f,0xd6,0xae,0x5d,0x5e},{0xed,0xfa,0x37,0x45,0xec,0x8f,0xad,0x59,0xb9,0xdc,0x6e,0x6a,0xb4,0xb3,0x11,0xa7},
      {0xed,0xfa,0x37,0x45,0xec,0x8f,0xad,0x59,0xb9,0xdc,0x6e,0x6a,0xb4,0xb3,0x11,0xa7},{0} }, // ALTMD5(6)
    { {0x2e,0x65,0x40,0xee,0x9d,0xff,0x58,0x56,0x5c,0x68,0x28,0x3f,0x95,0xe0,0x24,0xde},{0},{0x1d,0x7b,0x42,0x05,0x27,0xf0,0x44,0x10,0x93,0x75,0x44,0x68,0x93,0x8b,0xf8,0x13},
      {0x62,0xc0,0x3e,0xc2,0xa3,0xdc,0xb5,0xf4,0xe5,0x14,0xfa,0xb0,0x3d,0x9b,0x84,0xbe},{0xb1,0x15,0x7e,0xd1,0x6b,0xfe,0xba,0x6f,0x7c,0x6d,0xff,0xcf,0x29,0x91,0xfb,0x4e},
      {0},{0} }, // ALTMD5(7)
    { {0x54,0x22,0x7e,0xb1,0xc3,0xdc,0xe7,0x67,0x3e,0x8d,0x25,0x63,0x8c,0xf1,0x51,0xdf}, {0}, {0},
      {0}, {0},
      {0}, {0} }, // ALTMD5(8)
    { {0xd8,0xda,0xf2,0xda,0x90,0x4f,0x49,0x89,0x83,0x57,0xf0,0xbd,0x5c,0x39,0x94,0x97}, {0}, {0},
      {0}, {0},
      {0}, {0} }, // ALTMD5(9)
    { {0x6e,0xb7,0x6b,0x2d,0xed,0x64,0xf5,0x64,0x50,0x64,0xa7,0xdf,0xbe,0x38,0x18,0xb8}, {0}, {0},
      {0}, {0},
      {0}, {0} }, // ALTMD5(10)
    { {0x30,0x12,0x2f,0xc3,0x78,0xe6,0xbf,0xc2,0x0d,0x81,0x0c,0x13,0xb1,0x5b,0x45,0xa1}, {0}, {0},
      {0}, {0},
      {0}, {0} }, // ALTMD5(11)
    { {0x3e,0x99,0x9b,0x0f,0xd5,0xb3,0x77,0xcc,0xb2,0x27,0x87,0xcd,0xf3,0x4a,0x8e,0x2c}, {0}, {0},
      {0}, {0},
      {0xa9,0x8d,0xae,0x57,0x55,0xe6,0x21,0x45,0x68,0x50,0xab,0x59,0x47,0x2d,0x82,0x20}, {0} }, // ALTMD5(12)
    { {0x6c,0xc5,0x88,0xb5,0x58,0xf0,0x57,0x51,0xdc,0x91,0x84,0x8d,0xf9,0x64,0x74,0x20}, {0}, {0},
      {0}, {0},
      {0xe0,0x43,0x9a,0x39,0xa4,0x65,0xbb,0xc0,0xc9,0x0a,0x72,0xdc,0x82,0x2c,0x7d,0x2a}, {0} }, // ALTMD5(13)
};

//-------------------------------------------------------------------------------------

extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages=0 );
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
inline bool IsErrorTooSmall( float f, float threshold )
{
    return ( fabs(f) < threshold ) != 0;
}

inline bool IsErrorTooLarge( float f, float threshold )
{
    return ( fabs(f) > threshold ) != 0;
}

//-------------------------------------------------------------------------------------
// GenerateMipMaps
bool Test02()
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

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"mips", tempDir, MAX_PATH);
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
            printe( "Failed getting DDS data from (HRESULT %08X):\n%ls\n", hr, szPath );
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
                printe( "Failed computing MD5 checksum (HRESULT %08X):\n%ls\n", hr, szPath );
            }
            else
            {
                bool pass = true;

                //--- Simple mipmaps 1D/2D ------------------------------------------------
                ScratchImage mipChain;
                hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_DEFAULT, 0, mipChain, false );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Fant/Box generation (HRESULT %08X):\n%ls\n", hr, szPath );
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
                        printe( "Failed computing Fant/Box MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_TOPTEST) && memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Fant/Box top MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5;
                    }

                    const uint8_t* expected3 = nullptr;
                    if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKB )
                    {
                        expected3 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKB) >> 12) - 1 ].md5;
                    }

                    const uint8_t* expected4 = nullptr;
                    if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKC )
                    {
                        expected4 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKC) >> 20) - 1 ].md5;
                    }

                    hr = MD5Checksum( mipChain, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia[index].md5, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                              && (!expected3 || memcmp( digest, expected3, 16 ) != 0 )
                              && (!expected4 || memcmp( digest, expected4, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Fant/Box mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia[index].md5 );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                        if ( expected3 )
                        {
                            printdigest( "expected3", expected3 );
                        }
                        if ( expected4 )
                        {
                            printdigest( "expected4", expected4 );
                        }
                    }

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChain );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".wdp" );
                    SaveToWICFile( *mipChain.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                }

                if (FAILED(hr))
                    continue;

                // POINT
                ScratchImage mipChainPoint;
                hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_POINT, 0, mipChainPoint, false );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Point generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainPoint.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Point generation result is %zu mip levels:\n%ls\n", mipChainPoint.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainPoint, digest, 1 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_TOPTEST) && memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Point top MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data 
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_point;
                    }

                    hr = MD5Checksum( mipChainPoint, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia[index].md5_point, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Point mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia[index].md5_point );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_POINT" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainPoint );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *mipChainPoint.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                }

                // LINEAR
                ScratchImage mipChainLinear;
                hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_LINEAR, 0, mipChainLinear, false );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Linear generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainLinear.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Linear generation result is %zu mip levels:\n%ls\n", mipChainLinear.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainLinear, digest, 1 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_TOPTEST) && memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Linear top MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_linear;
                    }

                    hr = MD5Checksum( mipChainLinear, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia[index].md5_linear, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Linear mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia[index].md5_linear );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    float targMSE = 0.023f;
                    float mse = 0, mseV[4] = {};
                    const Image* img = mipChainPoint.GetImage(1,0,0);
                    hr = (img) ? ComputeMSE( *img, *mipChainLinear.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Linear image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Linear MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_LINEAR" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainLinear );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *mipChainLinear.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                }

                // CUBIC
                ScratchImage mipChainCubic;
                hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_CUBIC, 0, mipChainCubic, false );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Cubic generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainCubic.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Cubic generation result is %zu mip levels:\n%ls\n", mipChainCubic.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainCubic, digest, 1 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_TOPTEST) && memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Cubic top MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_cubic;
                    }

                    hr = MD5Checksum( mipChainCubic, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia[index].md5_cubic, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Cubic mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia[index].md5_cubic );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    float targMSE = 0.023f;
                    float mse = 0, mseV[4] = {};
                    const Image* img = mipChainPoint.GetImage(1,0,0);
                    hr = (img) ? ComputeMSE( *img, *mipChainCubic.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Cubic image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Cubic MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                    }

                    targMSE = ( BitsPerColor( metadata.format ) > 8 ) ? 0.03f : 0.01f;
                    img = mipChainLinear.GetImage(1,0,0);
                    hr = (img) ? ComputeMSE( *img, *mipChainCubic.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing Linear vs. Cubic image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing Linear vs. Cubic MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_CUBIC" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainCubic );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *mipChainCubic.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                }

                // TRIANGLE
                ScratchImage mipChainTriangle;
                hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_TRIANGLE, 0, mipChainTriangle, false );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Triangle generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainTriangle.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed mip-map Triangle generation result is %zu mip levels:\n%ls\n", mipChainTriangle.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainTriangle, digest, 1 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_TOPTEST) && memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Triangle top MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_tri;
                    }

                    hr = MD5Checksum( mipChainTriangle, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia[index].md5_tri, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Triangle mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia[index].md5_tri );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_TRIANGLE" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainTriangle );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *mipChainTriangle.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                }

                // SEPALPHA
                if ( g_MipMapMedia[index].options & FLAGS_SEPALPHA )
                {
                    ScratchImage mipChainSepAlpha;
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_SEPARATE_ALPHA, 0, mipChainSepAlpha, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map separate alpha generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( mipChainSepAlpha.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map separate alpha generation result is %zu mip levels:\n%ls\n", mipChainSepAlpha.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        // Verfy the top-level of mipchain matches our source data
                        uint8_t digest[16];
                        hr = MD5Checksum( mipChainSepAlpha, digest, 1 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_TOPTEST) && memcmp( digest, srcdigest, 16 ) != 0 )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed comparing separate alpha top MD5 checksum:\n%ls\n", szPath );
                            printdigest( "computed", digest );
                            printdigest( "expected", srcdigest );
                        }

                        // Verify the mip-chain image data
                        const uint8_t* expected2 = nullptr;
                        if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK )
                        {
                            expected2 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_sepalpha;
                        }

                        hr = MD5Checksum( mipChainSepAlpha, digest );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed computing MD5 checksum of mipchain separate alpha data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( memcmp( digest, g_MipMapMedia[index].md5_sepalpha, 16 ) != 0
                                  && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed comparing separate alpha mipchain MD5 checksum:\n%ls\n", szPath );
                            printdigest( "computed", digest );
                            printdigest( "expected", g_MipMapMedia[index].md5_sepalpha );
                            if ( expected2 )
                            {
                                printdigest( "expected2", expected2 );
                            }
                        }
                        
                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_SEPALPHA" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainSepAlpha );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *mipChainSepAlpha.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }
                }

                //--- WIC vs. non-WIC mipgen ----------------------------------------------
                ScratchImage nwMipChain;
                ScratchImage nwMipChainLinear;
                ScratchImage nwMipChainCubic;
                if ( BitsPerColor( metadata.format ) <= 8 && !IsSRGB(metadata.format) )
                {
                    // non-WIC is already used when color-depth is > 8 and for sRGB

                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_FORCE_NON_WIC, 0, nwMipChain, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( nwMipChain.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC generation result is %zu mip levels:\n%ls\n", nwMipChain.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        float targMSE = ( g_MipMapMedia[index].options & FLAGS_SEPALPHA ) ? 0.071f : 0.003f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = mipChain.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwMipChain.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_nowic" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwMipChain );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *nwMipChain.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }

                    // POINT
                    ScratchImage nwMipChainPoint;
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_FORCE_NON_WIC | TEX_FILTER_POINT, 0, nwMipChainPoint, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC Point generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( nwMipChainPoint.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC Point generation result is %zu mip levels:\n%ls\n", nwMipChainPoint.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_POINTNOWIC) )
                        {
                            float targMSE = ( g_MipMapMedia[index].options & FLAGS_SEPALPHA ) ? 0.11f : 0.047f;
                            float mse = 0, mseV[4] = {};
                            const Image* img = mipChainPoint.GetImage(1,0,0);
                            hr = (img) ? ComputeMSE( *img, *nwMipChainPoint.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                            if ( FAILED(hr) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC to WIC Point image data (HRESULT %08X):\n%ls\n", hr, szPath );
                            }
                            else if ( IsErrorTooLarge( mse, targMSE ) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC to WIC Point MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                            }
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_POINT_nowic" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwMipChainPoint );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *nwMipChainPoint.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }

                    // LINEAR
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_FORCE_NON_WIC | TEX_FILTER_LINEAR, 0, nwMipChainLinear, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC Linear generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( nwMipChainLinear.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC Linear generation result is %zu mip levels:\n%ls\n", nwMipChainLinear.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        float targMSE = ( g_MipMapMedia[index].options & FLAGS_SEPALPHA ) ? 0.072f : 0.003f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = mipChainLinear.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwMipChainLinear.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Linear image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Linear MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        targMSE = 0.022f;
                        img = nwMipChainPoint.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwMipChainLinear.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC Point vs. Linear image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC Point vs. Linear MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_LINEAR_nowic" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwMipChainLinear );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *nwMipChainLinear.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }

                    // CUBIC
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_FORCE_NON_WIC | TEX_FILTER_CUBIC, 0, nwMipChainCubic, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC Cubic generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( nwMipChainCubic.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map non-WIC Cubic generation result is %zu mip levels:\n%ls\n", nwMipChainCubic.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        float targMSE = ( g_MipMapMedia[index].options & FLAGS_SEPALPHA ) ? 0.09f : 0.06f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = mipChainCubic.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwMipChainCubic.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Cubic image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Cubic MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_POINTNOWIC) )
                        {
                            targMSE = 0.052f;
                            img = nwMipChainPoint.GetImage(1,0,0);
                            hr = (img) ? ComputeMSE( *img, *nwMipChainCubic.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                            if ( FAILED(hr) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC Point vs. Cubic image data (HRESULT %08X):\n%ls\n", hr, szPath );
                            }
                            else if ( IsErrorTooLarge( mse, targMSE ) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC Point vs. Cubic MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                            }
                        }

                        targMSE = (g_MipMapMedia[index].options & FLAGS_SKIP_POINTNOWIC) ? 0.053f : 0.03f;
                        img = nwMipChainLinear.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwMipChainCubic.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC Linear vs. Cubic image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC Linear vs. Cubic MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_CUBIC_nowic" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwMipChainCubic );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *nwMipChainCubic.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }
                }

                //--- sRGB correct filtering -----------------------------------------------
                DXGI_FORMAT srgb = MakeSRGB( metadata.format );
                if ( srgb != metadata.format && !(g_MipMapMedia[index].options & FLAGS_SKIP_SRGB) )
                {
                    ScratchImage srgbMipChain;
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_SRGB, 0, srgbMipChain, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( srgbMipChain.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB generation result is %zu mip levels:\n%ls\n", srgbMipChain.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        float targMSE = 0.0011f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = nwMipChain.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *srgbMipChain.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB to non-WIC image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooSmall( mse, 0.00005f ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB to WIC which appear near identical MSE = %f (%f %f %f %f):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB to WIC MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_srgb" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbMipChain );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *srgbMipChain.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }

                    // LINEAR
                    ScratchImage srgbMipChainLinear;
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_SRGB | TEX_FILTER_LINEAR, 0, srgbMipChainLinear, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB Linear generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( srgbMipChainLinear.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB Linear generation result is %zu mip levels:\n%ls\n", srgbMipChainLinear.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        float targMSE = 0.0011f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = nwMipChainLinear.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *srgbMipChainLinear.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Linear to non-WIC image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooSmall( mse, 0.00005f ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Linear to WIC which appear near identical MSE = %f (%f %f %f %f):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Linear to WIC MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_LINEAR_srgb" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbMipChainLinear );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *srgbMipChainLinear.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }

                    // CUBIC
                    ScratchImage srgbMipChainCubic;
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_SRGB | TEX_FILTER_CUBIC, 0, srgbMipChainCubic, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB Cubic generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( srgbMipChainCubic.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB Cubic generation result is %zu mip levels:\n%ls\n", srgbMipChainCubic.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        float targMSE = 0.0011f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = nwMipChainCubic.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *srgbMipChainCubic.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Cubic to non-WIC image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooSmall( mse, 0.00005f ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Cubic to non-WIC which appear near identical MSE = %f (%f %f %f %f):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Cubic to non-WIC WIC MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_CUBIC_srgb" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbMipChainCubic );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *srgbMipChainCubic.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }

                    // TRIANGLE
                    ScratchImage srgbMipChainTriangle;
                    hr = GenerateMipMaps( *srcimage.GetImage(0,0,0), TEX_FILTER_SRGB | TEX_FILTER_TRIANGLE, 0, srgbMipChainTriangle, false );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB Triangle generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( srgbMipChainTriangle.GetMetadata().mipLevels < 2 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map sRGB Triangle generation result is %zu mip levels:\n%ls\n", srgbMipChainTriangle.GetMetadata().mipLevels, szPath );
                    }
                    else
                    {
                        float targMSE = 0.0011f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = mipChainTriangle.GetImage(1,0,0);
                        hr = (img) ? ComputeMSE( *img, *srgbMipChainTriangle.GetImage(1,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Triangle to default data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( IsErrorTooSmall( mse, 0.00005f ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Triangle to default which appear near identical MSE = %f (%f %f %f %f):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Triangle to default MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_TRIANGLE_srgb" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbMipChainTriangle );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *srgbMipChainTriangle.GetImage(1,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath ); 
#endif
                    }
                }

                //--- non-WIC mipgen ------------------------------------------------------
// TODO - TEX_FILTER_MIRROR_U, TEX_FILTER_MIRROR_V
// TODO - TEX_FILTER_WRAP_U, TEX_FILTER_WRAP_V


                //--- Complex mipmaps 1D/2D -----------------------------------------------
                if ( srcimage.GetImageCount() > 1 && ( (metadata.width * metadata.height) < (16384 * 16384) ) )
                {
                    ScratchImage mipChainComplex;
                    hr = GenerateMipMaps( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChainComplex );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed mip-map complex Fant/Box generation (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else
                    {
                        const TexMetadata& metadata2 = mipChainComplex.GetMetadata();

                        if ( metadata2.width != check->width
                             || metadata2.height != check->height
                             || metadata2.mipLevels < 2
                             || metadata2.depth != check->depth
                             || metadata2.arraySize != check->arraySize
                             || metadata2.miscFlags != check->miscFlags
                             || metadata2.format != check->format
                             || metadata2.dimension != check->dimension )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed mip-map complex Fant/Box generation result is %zu mip levels:\n%ls\n", metadata2.mipLevels, szPath );
                            printmeta( &metadata2 );
                            printmetachk( check );
                        }
                        else
                        {
                            // Verfy the top-level of mipchain matches our source data
                            uint8_t digest[16];
                            hr = MD5Checksum( mipChainComplex, digest, 1 );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                            }
                            else if ( !(g_MipMapMedia[index].options & FLAGS_SKIP_TOPTEST) && memcmp( digest, srcdigest, 16 ) != 0 )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed comparing complex top MD5 checksum:\n%ls\n", szPath );
                                printdigest( "computed", digest );
                                printdigest( "expected", srcdigest );
                            }

                            // Verify the mip-chain image data
                            const uint8_t* expected2 = nullptr;
                            if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK )
                            {
                                expected2 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_c;
                            }

                            const uint8_t* expected3 = nullptr;
                            if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKB )
                            {
                                expected3 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKB) >> 12) - 1 ].md5_c;
                            }

                            const uint8_t* expected4 = nullptr;
                            if ( g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKC )
                            {
                                expected4 = g_AltMD5[ ((g_MipMapMedia[index].options & FLAGS_ALTMD5_MASKC) >> 20) - 1 ].md5_c;
                            }

                            hr = MD5Checksum( mipChainComplex, digest );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed computing MD5 checksum of complex mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                            }
                            else if ( memcmp( digest, g_MipMapMedia[index].md5_c, 16 ) != 0
                                      && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                                      && (!expected3 || memcmp( digest, expected3, 16 ) != 0 )
                                      && (!expected4 || memcmp( digest, expected4, 16 ) != 0 ) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed comparing complex mipchain MD5 checksum:\n%ls\n", szPath );
                                printdigest( "computed", digest );
                                printdigest( "expected", g_MipMapMedia[index].md5_c );
                                if ( expected2 )
                                {
                                    printdigest( "expected2", expected2 );
                                }
                                if ( expected3 )
                                {
                                    printdigest( "expected3", expected2 );
                                }
                                if ( expected4 )
                                {
                                    printdigest( "expected4", expected2 );
                                }
                            }

                            wchar_t tname[MAX_PATH] = {};
                            wcscpy_s( tname, fname );
                            wcscat_s( tname, L"_complex" );

                            wchar_t szDestPath[MAX_PATH] = {};
                            _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                            SaveToDDSFile( mipChainComplex.GetImages(), mipChainComplex.GetImageCount(), mipChainComplex.GetMetadata(), DDS_FLAGS_NONE, szDestPath );
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


//-------------------------------------------------------------------------------------
// GenerateMipMaps3D
bool Test03()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_MipMapMedia3D); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_MipMapMedia3D[index].fname, szPath, MAX_PATH);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"mips3D", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );

        const TexMetadata* check = &g_MipMapMedia3D[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting DDS data from (HRESULT %08X):\n%ls\n", hr, szPath );
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
            assert( metadata.IsVolumemap() );

            uint8_t srcdigest[16];
            hr = MD5Checksum( srcimage, srcdigest, metadata.depth );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing MD5 checksum (HRESULT %08X):\n%ls\n", hr, szPath );
            }
            else
            {
                bool pass = true;

                //--- Simple mipmaps volume -----------------------------------------------
                ScratchImage mipChain;
                hr = GenerateMipMaps3D( srcimage.GetImages(), metadata.depth, TEX_FILTER_DEFAULT, 0, mipChain );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Fant/Box generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChain.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Fant/Box generation result is %zu mip levels:\n%ls\n", mipChain.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChain, digest, metadata.depth );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data 
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5;
                    }

                    hr = MD5Checksum( mipChain, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia3D[index].md5, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia3D[index].md5 );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChain );
                }

                if (FAILED(hr))
                    continue;

                // POINT
                ScratchImage mipChainPoint;
                hr = GenerateMipMaps3D( srcimage.GetImages(), metadata.depth, TEX_FILTER_POINT, 0, mipChainPoint );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Point generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainPoint.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Point generation result is %zu mip levels:\n%ls\n", mipChainPoint.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainPoint, digest, metadata.depth );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Point MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_point;
                    }

                    hr = MD5Checksum( mipChainPoint, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia3D[index].md5_point, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Point mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia3D[index].md5_point );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }


                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_POINT" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainPoint );
                }

                // LINEAR
                ScratchImage mipChainLinear;
                hr = GenerateMipMaps3D( srcimage.GetImages(), metadata.depth, TEX_FILTER_LINEAR, 0, mipChainLinear );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Linear generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainLinear.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Linear generation result is %zu mip levels:\n%ls\n", mipChainLinear.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainLinear, digest, metadata.depth );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Linear MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_linear;
                    }

                    hr = MD5Checksum( mipChainLinear, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia3D[index].md5_linear, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Linear mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia3D[index].md5_linear );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }


                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_LINEAR" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainLinear );
                }

                // CUBIC
                ScratchImage mipChainCubic;
                hr = GenerateMipMaps3D( srcimage.GetImages(), metadata.depth, TEX_FILTER_CUBIC, 0, mipChainCubic );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Cubic generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainCubic.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Cubic generation result is %zu mip levels:\n%ls\n", mipChainCubic.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainCubic, digest, metadata.depth );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Cubic MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_cubic;
                    }

                    hr = MD5Checksum( mipChainCubic, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia3D[index].md5_cubic, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Cubic mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia3D[index].md5_cubic );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }


                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_CUBIC" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainCubic );
                }

                // TRIANGLE
                ScratchImage mipChainTriangle;
                hr = GenerateMipMaps3D( srcimage.GetImages(), metadata.depth, TEX_FILTER_TRIANGLE, 0, mipChainTriangle );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Triangle generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else if ( mipChainTriangle.GetMetadata().mipLevels < 2 )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map Triangle generation result is %zu mip levels:\n%ls\n", mipChainTriangle.GetMetadata().mipLevels, szPath );
                }
                else
                {
                    // Verfy the top-level of mipchain matches our source data
                    uint8_t digest[16];
                    hr = MD5Checksum( mipChainTriangle, digest, metadata.depth );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, srcdigest, 16 ) != 0 )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Triangle MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", srcdigest );
                    }

                    // Verify the mip-chain image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_tri;
                    }

                    hr = MD5Checksum( mipChainTriangle, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                    }
                    else if ( memcmp( digest, g_MipMapMedia3D[index].md5_tri, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing Triangle mipchain MD5 checksum:\n%ls\n", szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_MipMapMedia3D[index].md5_tri );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_TRIANGLE" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, mipChainTriangle );
                }

// TODO - TEX_FILTER_MIRROR_U, TEX_FILTER_MIRROR_V, TEX_FILTER_MIRROR_W
// TODO - TEX_FILTER_WRAP_U, TEX_FILTER_WRAP_V, TEX_FILTER_WRAP_W

                //--- Complex mipmaps volume ----------------------------------------------
                ScratchImage mipChainComplex;
                hr = GenerateMipMaps3D( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChainComplex );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed volume mip-map complex Fant/Box generation (HRESULT %08X):\n%ls\n", hr, szPath );
                }
                else
                {
                    const TexMetadata& metadata2 = mipChainComplex.GetMetadata();

                    if ( metadata2.width != check->width
                         || metadata2.height != check->height
                         || metadata2.mipLevels < 2
                         || metadata2.depth != check->depth
                         || metadata2.arraySize != check->arraySize
                         || metadata2.miscFlags != check->miscFlags
                         || metadata2.format != check->format
                         || metadata2.dimension != check->dimension )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed volume mip-map complex Fant/Box generation result is %zu mip levels:\n%ls\n", metadata2.mipLevels, szPath );
                        printmeta( &metadata2 );
                        printmetachk( check );
                    }
                    else
                    {
                        // Verfy the top-level of mipchain matches our source data
                        uint8_t digest[16];
                        hr = MD5Checksum( mipChainComplex, digest, metadata2.depth );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( memcmp( digest, srcdigest, 16 ) != 0 )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed comparing complex MD5 checksum:\n%ls\n", szPath );
                            printdigest( "computed", digest );
                            printdigest( "expected", srcdigest );
                        }

                        // Verify the mip-chain image data
                        const uint8_t* expected2 = nullptr;
                        if ( g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK )
                        {
                            expected2 = g_AltMD5[ ((g_MipMapMedia3D[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_c;
                        }

                        hr = MD5Checksum( mipChainComplex, digest );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed computing MD5 checksum of mipchain data (HRESULT %08X):\n%ls\n", hr, szPath );
                        }
                        else if ( memcmp( digest, g_MipMapMedia3D[index].md5_c, 16 ) != 0
                                  && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed comparing complex mipchain MD5 checksum:\n%ls\n", szPath );
                            printdigest( "computed", digest );
                            printdigest( "expected", g_MipMapMedia3D[index].md5_c );
                            if ( expected2 )
                            {
                                printdigest( "expected2", expected2 );
                            }
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname );
                        wcscat_s( tname, L"_complex" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveToDDSFile( mipChainComplex.GetImages(), mipChainComplex.GetImageCount(), mipChainComplex.GetMetadata(), DDS_FLAGS_NONE, szDestPath );
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
