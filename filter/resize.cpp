//-------------------------------------------------------------------------------------
// resize.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "filter.h"

#include "DirectXTex.h"

using namespace DirectX;

#pragma warning(disable:6262) // test code doesn't need conservative stack size

#define ALTMD5(n) (n << 4)
#define ALTMD5B(n) (n << 12)

namespace
{
    enum
    {
        FLAGS_NONE = 0x0,
        FLAGS_SEPALPHA = 0x1,
        FLAGS_SKIP_POINTNOWIC = 0x2,
        FLAGS_SKIP_SRGB = 0x4,
        FLAGS_NAMECONFLICT = 0x8,
        FLAGS_SKIP_EXHAUSTIVE = 0x10,
        FLAGS_ALTMD5_MASK = 0xff0,
        FLAGS_ALTMD5_MASKB = 0xff000,
    };

    struct ResizeMedia
    {
        DWORD options;
        UINT twidth;
        UINT theight;
        TexMetadata metadata;
        const wchar_t *fname;
        uint8_t md5[16];
        uint8_t md5_point[16];
        uint8_t md5_linear[16];
#if defined(_M_X64) || defined(_M_ARM64)
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

    const ResizeMedia g_ResizeMedia[] =
    {
        // test-options | target-width target-height | <source> width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { FLAGS_NONE, 16, 16, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds",
            {0x0e,0x0a,0x03,0x11,0xf2,0xac,0xf5,0xee,0xf3,0x07,0x47,0x44,0x17,0x17,0xca,0x84},{0xdd,0x08,0x9e,0x03,0x56,0x2f,0xe1,0x2a,0x3b,0xa7,0xed,0x53,0xde,0x31,0xb1,0x1b},{0xbc,0x9b,0xe4,0x80,0x82,0x04,0x2e,0xe2,0x68,0x10,0xaa,0x96,0x34,0x05,0xdd,0xe7},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #else
            {0x81,0x52,0xec,0x48,0x64,0x44,0xc9,0x09,0x5f,0xf1,0xa6,0x7a,0x58,0x67,0x00,0xef},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #endif
            {0}, {0} },

    #ifndef BUILD_BVT_ONLY
        { FLAGS_NONE, 100, 75, { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds",
            {0xd2,0xa7,0xed,0x29,0x83,0x91,0xb0,0xdd,0x78,0x93,0xfb,0xaf,0x57,0xd4,0x61,0xea},{0xd7,0x01,0x81,0x9c,0xf6,0x31,0xcc,0xda,0x85,0xbd,0x1d,0xf4,0x3d,0x72,0xba,0x22},{0x12,0x28,0x60,0xdb,0xc4,0xe5,0x98,0x68,0x55,0x65,0xb5,0xbb,0xf2,0x6a,0x86,0x7c},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x75,0x97,0x3a,0xce,0x0f,0x18,0x79,0x3f,0x75,0x3c,0x06,0x0e,0xb7,0x7d,0xf4,0x39},{0x2f,0x72,0xcb,0x4f,0x7a,0xb1,0x8f,0x30,0xbf,0x59,0xfa,0x46,0xc1,0xac,0x7a,0xf5},
        #else
            {0x98,0x28,0xfe,0xf8,0xa6,0x2e,0x9d,0x8e,0xc2,0xb5,0xc4,0x4a,0x6c,0x33,0x67,0x25},{0x75,0x97,0x3a,0xce,0x0f,0x18,0x79,0x3f,0x75,0x3c,0x06,0x0e,0xb7,0x7d,0xf4,0x39},{0x15,0xfa,0xf3,0x79,0x1f,0x3e,0x2a,0x5b,0x99,0xe9,0x37,0x0a,0x64,0xe5,0xcb,0xb4},
        #endif
            {0}, {0} },
        { FLAGS_NONE, 128, 128, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds",
            {0x23,0x2c,0x22,0x2d,0x88,0x00,0xa3,0xd3,0x95,0xc6,0xd6,0x95,0x66,0x16,0x47,0xef},{0x07,0xe5,0x99,0x20,0x12,0xc7,0xdf,0x72,0x31,0xb2,0xad,0x13,0x8e,0x91,0xb2,0x21},{0xca,0xa9,0xf5,0xc7,0x46,0x0e,0xf2,0x93,0xf7,0x2b,0xbf,0x2b,0xf6,0xfc,0x3d,0x97},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x87,0x97,0x95,0x8e,0xdf,0x16,0x46,0xae,0x7f,0x7c,0xc5,0x54,0xa1,0xf5,0xba,0x5d},{0x5f,0x48,0xc0,0x4d,0xbc,0x83,0x0f,0xf5,0x08,0x96,0xe2,0xb0,0xe6,0x7f,0xab,0x8a},
        #else
            {0xff,0x7c,0x93,0xb8,0x12,0xec,0x1e,0xd7,0x64,0xa9,0x97,0x7c,0x0b,0x27,0x42,0xf3},{0x87,0x97,0x95,0x8e,0xdf,0x16,0x46,0xae,0x7f,0x7c,0xc5,0x54,0xa1,0xf5,0xba,0x5d},{0x5f,0x48,0xc0,0x4d,0xbc,0x83,0x0f,0xf5,0x08,0x96,0xe2,0xb0,0xe6,0x7f,0xab,0x8a},
        #endif
            {0},{0} },
        { FLAGS_NONE, 16, 16, { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds",
            {0x0e,0x0a,0x03,0x11,0xf2,0xac,0xf5,0xee,0xf3,0x07,0x47,0x44,0x17,0x17,0xca,0x84},{0xdd,0x08,0x9e,0x03,0x56,0x2f,0xe1,0x2a,0x3b,0xa7,0xed,0x53,0xde,0x31,0xb1,0x1b},{0xbc,0x9b,0xe4,0x80,0x82,0x04,0x2e,0xe2,0x68,0x10,0xaa,0x96,0x34,0x05,0xdd,0xe7},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #else
            {0x81,0x52,0xec,0x48,0x64,0x44,0xc9,0x09,0x5f,0xf1,0xa6,0x7a,0x58,0x67,0x00,0xef},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #endif
            {0x0e,0x0a,0x03,0x11,0xf2,0xac,0xf5,0xee,0xf3,0x07,0x47,0x44,0x17,0x17,0xca,0x84}, {0} },
        { FLAGS_NONE, 16, 16, { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds",
            {0x0e,0x0a,0x03,0x11,0xf2,0xac,0xf5,0xee,0xf3,0x07,0x47,0x44,0x17,0x17,0xca,0x84},{0xdd,0x08,0x9e,0x03,0x56,0x2f,0xe1,0x2a,0x3b,0xa7,0xed,0x53,0xde,0x31,0xb1,0x1b},{0xbc,0x9b,0xe4,0x80,0x82,0x04,0x2e,0xe2,0x68,0x10,0xaa,0x96,0x34,0x05,0xdd,0xe7},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #else
            {0x81,0x52,0xec,0x48,0x64,0x44,0xc9,0x09,0x5f,0xf1,0xa6,0x7a,0x58,0x67,0x00,0xef},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #endif
            {0xfc,0xfb,0xc4,0xa4,0xe0,0x50,0x0e,0xd3,0x1b,0xec,0x81,0x6a,0xf3,0xcb,0x41,0xd7}, {0} },
        { FLAGS_NONE, 16, 16, { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds",
            {0x0e,0x0a,0x03,0x11,0xf2,0xac,0xf5,0xee,0xf3,0x07,0x47,0x44,0x17,0x17,0xca,0x84},{0xdd,0x08,0x9e,0x03,0x56,0x2f,0xe1,0x2a,0x3b,0xa7,0xed,0x53,0xde,0x31,0xb1,0x1b},{0xbc,0x9b,0xe4,0x80,0x82,0x04,0x2e,0xe2,0x68,0x10,0xaa,0x96,0x34,0x05,0xdd,0xe7},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #else
            {0x81,0x52,0xec,0x48,0x64,0x44,0xc9,0x09,0x5f,0xf1,0xa6,0x7a,0x58,0x67,0x00,0xef},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #endif
            {0xfc,0xfb,0xc4,0xa4,0xe0,0x50,0x0e,0xd3,0x1b,0xec,0x81,0x6a,0xf3,0xcb,0x41,0xd7}, {0} },
        { FLAGS_NONE, 16, 16, { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds",
            {0x0e,0x0a,0x03,0x11,0xf2,0xac,0xf5,0xee,0xf3,0x07,0x47,0x44,0x17,0x17,0xca,0x84},{0xdd,0x08,0x9e,0x03,0x56,0x2f,0xe1,0x2a,0x3b,0xa7,0xed,0x53,0xde,0x31,0xb1,0x1b},{0xbc,0x9b,0xe4,0x80,0x82,0x04,0x2e,0xe2,0x68,0x10,0xaa,0x96,0x34,0x05,0xdd,0xe7},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #else
            {0x81,0x52,0xec,0x48,0x64,0x44,0xc9,0x09,0x5f,0xf1,0xa6,0x7a,0x58,0x67,0x00,0xef},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #endif
            {0x1d,0xe5,0x3a,0xc3,0xe9,0x71,0x95,0xd3,0x77,0x95,0xf0,0x6b,0xa2,0x34,0x02,0xd1}, {0} },
        { FLAGS_NONE, 16, 16, { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds",
            {0x0e,0x0a,0x03,0x11,0xf2,0xac,0xf5,0xee,0xf3,0x07,0x47,0x44,0x17,0x17,0xca,0x84},{0xdd,0x08,0x9e,0x03,0x56,0x2f,0xe1,0x2a,0x3b,0xa7,0xed,0x53,0xde,0x31,0xb1,0x1b},{0xbc,0x9b,0xe4,0x80,0x82,0x04,0x2e,0xe2,0x68,0x10,0xaa,0x96,0x34,0x05,0xdd,0xe7},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #else
            {0x81,0x52,0xec,0x48,0x64,0x44,0xc9,0x09,0x5f,0xf1,0xa6,0x7a,0x58,0x67,0x00,0xef},{0x63,0x49,0x15,0xc1,0x19,0xdf,0x10,0x0c,0xcc,0x4b,0xbc,0x9c,0x0d,0x9a,0xff,0x10},{0x44,0xa6,0xf2,0x1e,0x69,0x9b,0xcb,0xb9,0xfc,0x86,0x4c,0x94,0x98,0x86,0x8c,0x97},
        #endif
            {0x1d,0xe5,0x3a,0xc3,0xe9,0x71,0x95,0xd3,0x77,0x95,0xf0,0x6b,0xa2,0x34,0x02,0xd1}, {0} },
        { FLAGS_NONE, 128, 128, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds",
            {0x38,0x76,0xa7,0xe8,0xf7,0xec,0x3a,0x54,0x2a,0xda,0x3d,0x69,0x9a,0xe4,0x99,0x4f},{0xa6,0xab,0x65,0x44,0x17,0xa6,0xd8,0x19,0xb6,0x4a,0xd2,0xde,0x1d,0xa4,0x3e,0xfe},{0x19,0xc7,0x19,0x18,0xc9,0x99,0xe3,0xae,0x7c,0x09,0xd7,0x88,0xff,0xb6,0x94,0x5b},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x25,0x5a,0x37,0x1c,0x26,0x1b,0x5f,0x50,0x58,0x9f,0xed,0x8e,0x4e,0xe6,0x8a,0xbe},{0x79,0x20,0x85,0x63,0xb7,0x3d,0x2e,0x98,0x51,0xc4,0x63,0x88,0x5c,0x27,0xbb,0x82},
        #else
            {0xa1,0xb7,0x5b,0xef,0xc8,0x14,0x76,0xf2,0x5b,0xc1,0x4c,0xc3,0xc3,0x5a,0xef,0x81},{0x25,0x5a,0x37,0x1c,0x26,0x1b,0x5f,0x50,0x58,0x9f,0xed,0x8e,0x4e,0xe6,0x8a,0xbe},{0xff,0xd9,0xe7,0xc2,0x5e,0xe6,0xa7,0x78,0x2b,0x10,0x7c,0x37,0xb9,0xc8,0x97,0x04},
        #endif
            {0}, {0} },
        { FLAGS_NONE, 128, 128, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds",
            {0x9f,0x6c,0xb7,0x04,0x99,0x8f,0x8c,0x3d,0x75,0x8c,0x6e,0x70,0xbc,0x15,0x4a,0xd3},{0xc8,0x17,0x7f,0xaa,0x25,0xac,0x56,0xd0,0x65,0xdb,0x8b,0xb9,0x70,0x97,0x68,0x4b},{0x67,0x64,0xb8,0xcf,0x80,0x65,0xf9,0xfe,0x2e,0x29,0xc3,0x7a,0x40,0xcf,0x36,0xc1},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0xc1,0xf6,0x8b,0x7e,0x55,0xfd,0x84,0x36,0x9e,0x92,0x94,0x95,0x2c,0xca,0x7f,0xbe},{0x32,0xb4,0x58,0xdb,0xe6,0x95,0x32,0x4d,0x01,0x15,0x7b,0x2f,0x69,0x05,0x2d,0xef},
        #else
            {0x6b,0x8d,0xe9,0x5a,0x93,0xa0,0x6d,0x3a,0x60,0x98,0xeb,0xa7,0x69,0xfc,0x01,0x9b},{0xc1,0xf6,0x8b,0x7e,0x55,0xfd,0x84,0x36,0x9e,0x92,0x94,0x95,0x2c,0xca,0x7f,0xbe},{0xc8,0xa5,0x04,0xfa,0xfc,0xb9,0xf5,0x32,0x06,0xf8,0xfe,0x2b,0x33,0x64,0x07,0x16},
        #endif
            {0}, {0} },
        { FLAGS_NONE, 128, 128, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds",
            {0x9f,0x1c,0x8d,0xa2,0x85,0xae,0xf7,0x26,0x17,0x17,0x35,0xda,0x4a,0x50,0x7f,0xfa},{0xdf,0x68,0xed,0x2d,0xd1,0x29,0x4b,0x16,0x76,0x25,0xe5,0x3d,0x35,0x3e,0x0b,0xe0},{0x52,0xac,0x9b,0xd0,0xa0,0x04,0x51,0x9a,0x01,0xbf,0xcc,0x0d,0xce,0xbb,0xfb,0xdd},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0xa4,0xfb,0x6b,0x37,0x37,0x7c,0xc7,0x56,0xb0,0x11,0xa3,0x7b,0x45,0x88,0x27,0x67},{0xe0,0xaf,0x3f,0xbb,0x84,0x83,0x6c,0x47,0x86,0x83,0x64,0xcc,0x22,0x9b,0x73,0x0d},
        #else
            {0xff,0x59,0x12,0xaf,0xba,0x8b,0x4a,0xc9,0xee,0x4d,0x80,0x15,0x8a,0x1d,0xf3,0x22},{0xa4,0xfb,0x6b,0x37,0x37,0x7c,0xc7,0x56,0xb0,0x11,0xa3,0x7b,0x45,0x88,0x27,0x67},{0x74,0x69,0xb3,0x95,0xc2,0x27,0xaa,0xd1,0x3d,0x45,0xbd,0x44,0xa5,0xc7,0xa8,0xc3},
        #endif
            {0}, {0} },
        { FLAGS_NONE, 128, 128, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgb565.dds",
            {0x89,0x7d,0x82,0x30,0xde,0x8e,0x9f,0xfb,0x60,0x16,0xda,0x19,0xd7,0xe5,0x20,0xb0},{0xc8,0xa0,0x9e,0x88,0x5f,0xf2,0xb9,0x3b,0xb5,0xb8,0x36,0xd7,0xab,0x4b,0xec,0x1d},{0x18,0x30,0x1f,0x5f,0x80,0x75,0x12,0x4d,0x4c,0x89,0x44,0xa4,0xf8,0x0c,0x20,0xf9},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x3d,0xea,0xc5,0x3f,0x43,0x50,0x7f,0x82,0x4f,0x7e,0x42,0xb2,0x03,0xb2,0x83,0x68},{0x10,0xca,0x5b,0x2b,0x3b,0xc6,0xb3,0xf0,0x03,0x51,0x17,0x5a,0x59,0xf6,0xe5,0x5e},
        #else
            {0xc9,0x05,0x07,0xf9,0x0e,0x04,0x9c,0x31,0xe9,0xed,0xda,0xb1,0x46,0x06,0x84,0x2e},{0x3d,0xea,0xc5,0x3f,0x43,0x50,0x7f,0x82,0x4f,0x7e,0x42,0xb2,0x03,0xb2,0x83,0x68},{0x01,0xaa,0x4b,0xfb,0xb6,0x3b,0xad,0xfa,0x79,0x9d,0x6d,0x87,0xcd,0x3f,0x71,0x66},
        #endif
            {0}, {0} },
        { FLAGS_NONE, 128, 128, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8R8G8B8.dds",
            {0x38,0xd9,0x95,0xa9,0x98,0x31,0x76,0x49,0x4a,0xaf,0xd3,0xa9,0xc5,0xe6,0x31,0xa2},{0x3a,0x58,0x43,0x0b,0x37,0xb7,0x80,0xde,0xbb,0x6a,0x18,0x4c,0x1d,0x2d,0x69,0x11},{0x3d,0x24,0x3b,0x4f,0x1b,0x4d,0x85,0x80,0x05,0x84,0x72,0x2b,0xfe,0x02,0x9e,0x62},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x40,0x04,0x46,0xce,0x64,0xef,0xc7,0x0c,0x72,0xe5,0x5a,0x4f,0xef,0x70,0xc6,0x22},{0x15,0xdf,0x56,0xce,0xeb,0x56,0xfe,0x32,0xb5,0x97,0x5d,0xb8,0xe1,0x16,0x7d,0x79},
        #else
            {0x58,0xd5,0xf6,0x6a,0xb6,0xcf,0x74,0x46,0x12,0xdc,0xaa,0x2d,0xf3,0xff,0xb6,0xfe},{0x40,0x04,0x46,0xce,0x64,0xef,0xc7,0x0c,0x72,0xe5,0x5a,0x4f,0xef,0x70,0xc6,0x22},{0xb6,0xe2,0xa4,0x9a,0x7a,0x5f,0x61,0x3e,0x09,0x2f,0xc5,0x15,0x69,0xbd,0x40,0x4e},
        #endif
            {0}, {0} },
        { ALTMD5(1), 64, 64, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_L8.dds",
            {0x1e,0xa9,0x38,0xab,0x37,0x76,0x6e,0x7e,0xd6,0x9f,0x48,0xf5,0x51,0xc8,0x0a,0x98},{0x7a,0xea,0xba,0xe2,0x7e,0x68,0x99,0xe1,0x2f,0xc4,0x52,0xa5,0x29,0xc7,0x30,0xfb},{0xad,0x68,0x35,0xf1,0xd2,0xee,0xf0,0xd1,0x8f,0x65,0xf8,0x49,0x41,0x73,0xd6,0xda},
            {0x86,0x5a,0x8f,0xd9,0x10,0x8f,0xcb,0xd2,0x80,0x66,0xd3,0xf5,0xc9,0xc9,0xec,0x43},{0x17,0xcd,0xe5,0x4b,0xce,0x38,0xf1,0x55,0x5a,0x6c,0x2a,0x65,0x15,0x39,0xb0,0x69},{0x08,0x78,0x6c,0xe4,0x45,0x0e,0xaf,0x03,0x42,0x58,0x62,0xdb,0xcc,0x25,0xee,0xe4},
            {0}, {0} },
        { ALTMD5B(13) | ALTMD5(9), 64, 64, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_r16f.dds",
            {0xc2,0x76,0x24,0xf4,0xaf,0x31,0x86,0x3f,0x0a,0xbb,0x54,0x9c,0x4f,0x33,0x72,0xb0},{0x08,0xce,0x46,0x5a,0xbf,0x73,0x33,0xd0,0xd0,0x3a,0xab,0xb3,0x47,0xfd,0x97,0x89},{0x8f,0x02,0x67,0xdb,0xb9,0xae,0xd1,0xcc,0x16,0x3a,0x62,0x11,0x38,0x05,0x17,0x33},
            {0x20,0xad,0x2d,0x4f,0xe5,0x84,0x43,0xd4,0x20,0x0c,0xa5,0x2c,0xab,0x1c,0x09,0x27},{0x20,0xad,0x2d,0x4f,0xe5,0x84,0x43,0xd4,0x20,0x0c,0xa5,0x2c,0xab,0x1c,0x09,0x27},{0x8c,0x58,0x5e,0x8e,0xb3,0x27,0x62,0x1e,0xd9,0x91,0x2d,0x75,0x45,0xfd,0x4b,0x53},
            {0}, {0} },
        { FLAGS_NONE, 64, 64, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_r32f.dds",
            {0x67,0xfb,0xdb,0xdc,0x12,0xab,0xb3,0x6e,0xdc,0xbb,0xaf,0xe6,0xff,0xbd,0x16,0xf6},{0xb0,0x64,0x6f,0xf1,0x96,0xaf,0x60,0xb5,0x47,0x69,0x52,0x1e,0xe8,0x39,0x39,0x6e},{0x67,0xfb,0xdb,0xdc,0x12,0xab,0xb3,0x6e,0xdc,0xbb,0xaf,0xe6,0xff,0xbd,0x16,0xf6},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x01,0xa4,0x0c,0x76,0x10,0x4a,0x0f,0x0d,0x79,0xa7,0xc9,0x60,0xc3,0x17,0xf0,0x2b},{0x97,0x57,0x40,0x42,0x5f,0xe1,0x89,0x76,0x48,0x59,0xff,0x02,0xba,0x25,0xa6,0x84},
        #else
            {0x01,0xa4,0x0c,0x76,0x10,0x4a,0x0f,0x0d,0x79,0xa7,0xc9,0x60,0xc3,0x17,0xf0,0x2b},{0x01,0xa4,0x0c,0x76,0x10,0x4a,0x0f,0x0d,0x79,0xa7,0xc9,0x60,0xc3,0x17,0xf0,0x2b},{0x5e,0xfd,0xae,0xa9,0xbd,0x27,0xd5,0x55,0x5f,0x6a,0xc6,0xc5,0xf3,0xe0,0xf1,0xec},
        #endif
            {0}, {0} },
        { FLAGS_NONE, 200, 100, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds",
            {0xec,0x0e,0x1a,0xc1,0x9f,0x59,0x2a,0xf2,0xa2,0xa9,0x37,0xf3,0xca,0x60,0xe3,0x85},{0x55,0xa2,0x80,0xfb,0x11,0x56,0x29,0x9d,0x09,0xea,0x72,0x72,0x10,0x57,0xc0,0x74},{0xec,0x0e,0x1a,0xc1,0x9f,0x59,0x2a,0xf2,0xa2,0xa9,0x37,0xf3,0xca,0x60,0xe3,0x85},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0xf3,0xa3,0xf7,0xb6,0xc8,0xc8,0x0f,0x80,0xd5,0xb8,0x17,0xd7,0xfe,0x0c,0x3b,0x16},{0xc2,0xfc,0x6c,0x07,0x89,0x18,0x35,0xda,0xd3,0x1e,0xea,0x12,0x0d,0x79,0x98,0xe3},
        #else
            {0xf3,0xa3,0xf7,0xb6,0xc8,0xc8,0x0f,0x80,0xd5,0xb8,0x17,0xd7,0xfe,0x0c,0x3b,0x16},{0xf3,0xa3,0xf7,0xb6,0xc8,0xc8,0x0f,0x80,0xd5,0xb8,0x17,0xd7,0xfe,0x0c,0x3b,0x16},{0x89,0xa8,0x8f,0xea,0x63,0x42,0x31,0xd7,0x86,0x1b,0x96,0x06,0xf2,0x09,0xd8,0xf1},
        #endif
            {0xec,0x0e,0x1a,0xc1,0x9f,0x59,0x2a,0xf2,0xa2,0xa9,0x37,0xf3,0xca,0x60,0xe3,0x85}, {0} },
        { FLAGS_NONE, 300, 200, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds",
            {0xb2,0xef,0x5b,0xd2,0xf1,0xf9,0xaf,0xb9,0xe0,0xe8,0xff,0x35,0x4b,0xa6,0xbd,0xe8},{0x47,0xd0,0x08,0xd7,0xaf,0xb9,0xff,0x9b,0x63,0x1c,0x65,0x38,0xc9,0x5b,0xc4,0x80},{0xb2,0xef,0x5b,0xd2,0xf1,0xf9,0xaf,0xb9,0xe0,0xe8,0xff,0x35,0x4b,0xa6,0xbd,0xe8},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x19,0x35,0x46,0x54,0x14,0x4c,0xb8,0x02,0x5f,0xbc,0x19,0xf2,0x93,0x89,0xee,0x45},{0x55,0xd7,0xed,0x07,0x42,0xd1,0xa5,0xd6,0x05,0x18,0x60,0x05,0x9e,0x73,0xb2,0x72},
        #else
            {0x19,0x35,0x46,0x54,0x14,0x4c,0xb8,0x02,0x5f,0xbc,0x19,0xf2,0x93,0x89,0xee,0x45},{0x19,0x35,0x46,0x54,0x14,0x4c,0xb8,0x02,0x5f,0xbc,0x19,0xf2,0x93,0x89,0xee,0x45},{0x52,0x07,0x22,0x07,0x8e,0xa0,0x1f,0xf6,0x29,0x72,0x68,0x57,0xab,0xdf,0x53,0xda},
        #endif
            {0xb2,0xef,0x5b,0xd2,0xf1,0xf9,0xaf,0xb9,0xe0,0xe8,0xff,0x35,0x4b,0xa6,0xbd,0xe8}, {0} },
        { ALTMD5(2), 64, 64, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_L16.dds",
            {0xa1,0x59,0xd1,0xd1,0xf6,0x61,0x85,0x02,0x9c,0x83,0x65,0xac,0x7d,0xb7,0xbd,0xa5},{0xfc,0x6a,0x1f,0xb0,0x8a,0x4f,0xaf,0xd3,0x68,0xa9,0x51,0x50,0xc9,0xee,0x24,0x47},{0xa1,0x59,0xd1,0xd1,0xf6,0x61,0x85,0x02,0x9c,0x83,0x65,0xac,0x7d,0xb7,0xbd,0xa5},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0xf2,0x9d,0xc0,0xc8,0x61,0x7b,0xcf,0x11,0x15,0x13,0x81,0xf8,0x39,0xc9,0x07,0x86},{0xed,0x7f,0x04,0x3c,0xd4,0x6c,0xa0,0x00,0xb9,0xc2,0x9b,0x4a,0x13,0x60,0xd8,0xee},
        #else
            {0xf2,0x9d,0xc0,0xc8,0x61,0x7b,0xcf,0x11,0x15,0x13,0x81,0xf8,0x39,0xc9,0x07,0x86},{0xf2,0x9d,0xc0,0xc8,0x61,0x7b,0xcf,0x11,0x15,0x13,0x81,0xf8,0x39,0xc9,0x07,0x86},{0x49,0x30,0x87,0x20,0x77,0x19,0x3b,0xc3,0x9d,0xbf,0x48,0x5c,0xa1,0xbd,0x27,0x55},
        #endif
            {0}, {0} },
        { FLAGS_NONE, 64, 64, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds",
            {0x41,0x19,0x74,0xe2,0x94,0xf9,0xa8,0xc8,0x7a,0x1c,0xb9,0x47,0x29,0x3e,0xc4,0x6e},{0x8c,0x2f,0x22,0x89,0xe5,0xb7,0xbb,0x51,0xa6,0x69,0x94,0x0f,0x54,0x45,0xa0,0x64},{0x41,0x19,0x74,0xe2,0x94,0xf9,0xa8,0xc8,0x7a,0x1c,0xb9,0x47,0x29,0x3e,0xc4,0x6e},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0xc8,0x1d,0x96,0x9e,0xd6,0xa7,0x46,0xb2,0x2e,0xa3,0xf2,0xc5,0x91,0xf2,0x64,0x8b},{0xd6,0x6d,0xd0,0x52,0x3b,0x09,0x4e,0xd5,0x91,0xb9,0xe1,0x80,0x51,0xde,0xc7,0x40},
        #else
            {0xc8,0x1d,0x96,0x9e,0xd6,0xa7,0x46,0xb2,0x2e,0xa3,0xf2,0xc5,0x91,0xf2,0x64,0x8b},{0xc8,0x1d,0x96,0x9e,0xd6,0xa7,0x46,0xb2,0x2e,0xa3,0xf2,0xc5,0x91,0xf2,0x64,0x8b},{0x33,0xbe,0xa5,0xab,0x5c,0x64,0x6e,0x01,0xb0,0x51,0x34,0x54,0x48,0x30,0x70,0x40},
        #endif
            {0}, {0} },
        { FLAGS_SKIP_SRGB, 1280, 720, { 256, 224, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"deltae_base.png",
            {0x71,0x3b,0x9a,0xbe,0x13,0xf2,0x15,0x57,0xd1,0xb3,0x31,0xed,0xa2,0x46,0x8e,0xb0},{0xfe,0x68,0x8c,0x50,0x20,0x76,0xdd,0xdd,0xcf,0x28,0xf0,0x30,0x3e,0x13,0x30,0xc4},{0x62,0x2c,0xba,0xd5,0x1c,0x78,0xe2,0x78,0xdf,0x88,0x60,0xf7,0x6b,0xfa,0xb1,0x0d},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x21,0xfd,0x88,0xc6,0x0a,0xf8,0xaa,0x80,0x32,0x41,0x25,0x3f,0xa2,0x4c,0x6e,0x98},{0x27,0xb4,0xdb,0x09,0xb7,0xdf,0x88,0x4c,0xf5,0x30,0x4e,0x03,0x0d,0xd3,0x57,0x6c},
        #else
            {0x54,0x8f,0x6f,0xa5,0xe6,0x82,0x85,0x13,0x5a,0x65,0x0d,0x6c,0x2a,0xba,0x27,0xbc},{0x21,0xfd,0x88,0xc6,0x0a,0xf8,0xaa,0x80,0x32,0x41,0x25,0x3f,0xa2,0x4c,0x6e,0x98},{0xb8,0x40,0x8e,0x1f,0xfc,0x3b,0x69,0x3b,0xb5,0x37,0x4f,0x51,0x31,0x85,0xd0,0x9f},
        #endif
            {0},{0} },
        { FLAGS_SKIP_SRGB, 1024, 768, { 768, 576, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Grayscale_Staircase.png",
            {0xb5,0x7a,0xa7,0xa2,0x09,0x39,0xd6,0x0a,0x2f,0x28,0xdf,0x14,0xfc,0x31,0x59,0x8e},{0x29,0x12,0xf3,0xba,0x50,0x86,0xb1,0xeb,0xbb,0x8f,0xdc,0x2f,0xea,0xdb,0xd5,0x03},{0x7f,0x59,0x51,0xe1,0x9f,0x1e,0x69,0x6a,0xf5,0x8d,0x6f,0xb6,0xd0,0xeb,0x9b,0xd3},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0xbb,0x6e,0x09,0xee,0xe0,0x51,0xee,0xc8,0x88,0xfc,0x23,0xa9,0x1e,0x01,0xf5,0xb3},{0xef,0xe3,0xf2,0xe7,0xb1,0xa6,0x33,0x20,0x09,0xeb,0x8b,0xf7,0x94,0xb7,0x8b,0xc7},
        #else
            {0xc5,0x31,0xd7,0x60,0x06,0x6f,0x01,0x1b,0x53,0xdb,0x05,0x7d,0xd7,0xf1,0x25,0xc4},{0xbb,0x6e,0x09,0xee,0xe0,0x51,0xee,0xc8,0x88,0xfc,0x23,0xa9,0x1e,0x01,0xf5,0xb3},{0x02,0xcb,0x2c,0x45,0x70,0x94,0x5a,0x03,0xb0,0xf3,0xa1,0x0b,0xa3,0x60,0x13,0x91},
        #endif
            {0},{0} },
        { FLAGS_NONE, 1920, 1200, { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D  }, MEDIA_PATH L"testpattern.png",
            {0x4c,0x0b,0xd1,0xc9,0x9f,0x25,0x7d,0x15,0x1e,0x0c,0xf1,0x8a,0x7d,0x8a,0x6a,0xa5},{0x83,0xa2,0x6d,0xd0,0xd4,0x2d,0x40,0xf6,0x49,0x8d,0x5e,0x11,0x47,0x89,0xb1,0x92},{0x4c,0x0b,0xd1,0xc9,0x9f,0x25,0x7d,0x15,0x1e,0x0c,0xf1,0x8a,0x7d,0x8a,0x6a,0xa5},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x25,0xe3,0xa3,0xb1,0x1b,0x86,0x8d,0x84,0x7c,0x86,0xf4,0x87,0x4e,0x11,0xf1,0x8d},{0xd9,0x45,0x21,0x5f,0x9c,0x4b,0x46,0x19,0x42,0xe3,0xe0,0x1e,0xef,0x00,0x3d,0x3b},
        #else
            {0x25,0xe3,0xa3,0xb1,0x1b,0x86,0x8d,0x84,0x7c,0x86,0xf4,0x87,0x4e,0x11,0xf1,0x8d},{0x25,0xe3,0xa3,0xb1,0x1b,0x86,0x8d,0x84,0x7c,0x86,0xf4,0x87,0x4e,0x11,0xf1,0x8d},{0x6b,0x9a,0x47,0xe4,0x33,0x1a,0xe5,0x12,0xb1,0xf9,0x30,0x98,0x3c,0xea,0x9d,0x3a},
        #endif
            {0},{0} },
        { FLAGS_SEPALPHA | FLAGS_SKIP_SRGB, 128, 64, { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds",
            {0x86,0xcf,0x6b,0x88,0x87,0xcf,0x05,0xb2,0x4c,0xd8,0xaa,0xa1,0xf4,0xc4,0xa2,0x93},{0x36,0xdb,0x71,0x84,0xd6,0x8c,0x6f,0xa8,0x92,0x91,0x1a,0x5c,0x61,0xc7,0xca,0xd0},{0xfb,0x07,0xe6,0xe0,0xc0,0xd1,0xa8,0x6d,0x75,0x98,0x99,0xa5,0x84,0x9c,0x52,0x65},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x30,0xaa,0xe4,0xb0,0xee,0x1b,0x44,0xdb,0xa1,0xa5,0x4e,0x19,0xbe,0x69,0x98,0x8c},{0xf0,0x73,0xa2,0x50,0xab,0x49,0xa7,0x15,0x52,0x42,0x17,0x71,0x33,0xcc,0x98,0x9d},
        #else
            {0xdb,0x97,0x35,0x38,0xee,0x44,0x61,0xe9,0x0e,0x21,0x01,0xb4,0xee,0x33,0xb1,0xc2},{0x30,0xaa,0xe4,0xb0,0xee,0x1b,0x44,0xdb,0xa1,0xa5,0x4e,0x19,0xbe,0x69,0x98,0x8c},{0x15,0x1d,0x93,0xad,0x85,0xe6,0x5c,0x0e,0x05,0xfe,0x11,0x50,0x89,0xee,0x23,0x5d},
        #endif
            {0x86,0xcf,0x6b,0x88,0x87,0xcf,0x05,0xb2,0x4c,0xd8,0xaa,0xa1,0xf4,0xc4,0xa2,0x93},{0x6a,0x0f,0x9a,0xbc,0xe1,0xea,0x2d,0x7a,0x7c,0xdb,0x69,0x16,0x87,0xe9,0x4d,0x39} },
        { FLAGS_SEPALPHA | ALTMD5B(14) | ALTMD5(10), 64, 64, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16f.dds",
            {0x6b,0x43,0xea,0xc6,0xa8,0x06,0xd1,0x02,0x0e,0xe9,0x66,0x8d,0x3a,0x6b,0xe8,0x16},{0xcf,0x1f,0xf7,0xb8,0x01,0xfd,0x47,0x5f,0x9c,0x47,0x63,0x33,0x7c,0x93,0x53,0xa2},{0x4d,0x17,0x0c,0xc9,0x21,0x08,0x6a,0xd4,0xae,0x73,0xda,0x30,0xcc,0x14,0xd5,0x4e},
            {0x5f,0xa9,0x49,0xd8,0x95,0x02,0xb5,0xac,0x84,0x00,0xd1,0x7c,0x61,0x77,0x1a,0x52},{0x5f,0xa9,0x49,0xd8,0x95,0x02,0xb5,0xac,0x84,0x00,0xd1,0x7c,0x61,0x77,0x1a,0x52},{0x5e,0x46,0xb1,0x5e,0x1e,0x08,0x6c,0x92,0xad,0x96,0x44,0x71,0x8e,0xbd,0x5b,0xfb},
            {0}, {0x6b,0x43,0xea,0xc6,0xa8,0x06,0xd1,0x02,0x0e,0xe9,0x66,0x8d,0x3a,0x6b,0xe8,0x16} },
        { FLAGS_SEPALPHA, 64, 64, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba32f.dds",
            {0x2a,0xe9,0x8a,0x24,0x59,0x2f,0x0a,0xdb,0xe1,0x10,0xc1,0x4f,0x52,0x33,0x53,0x6b},{0x72,0x62,0x9a,0x77,0x89,0xc4,0x42,0xc7,0xc4,0x89,0x7e,0xf2,0xee,0x92,0x4c,0xd1},{0x2a,0xe9,0x8a,0x24,0x59,0x2f,0x0a,0xdb,0xe1,0x10,0xc1,0x4f,0x52,0x33,0x53,0x6b},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x85,0x96,0xbd,0x8b,0xdf,0xef,0xae,0x6b,0x7b,0xc9,0xa3,0xcb,0x11,0xca,0xd1,0xb4},{0x22,0xb8,0x16,0x74,0xd6,0x33,0x04,0xd2,0x61,0xea,0x96,0xe5,0x3c,0x6c,0x0b,0xbb},
        #else
            {0},{0x85,0x96,0xbd,0x8b,0xdf,0xef,0xae,0x6b,0x7b,0xc9,0xa3,0xcb,0x11,0xca,0xd1,0xb4},{0xf4,0x27,0x03,0x2e,0x05,0x5e,0x2d,0x7f,0x3b,0xce,0x35,0x39,0x52,0x95,0xca,0xb5},
        #endif
            {0}, {0x2a,0xe9,0x8a,0x24,0x59,0x2f,0x0a,0xdb,0xe1,0x10,0xc1,0x4f,0x52,0x33,0x53,0x6b} },
        { FLAGS_NONE, 32, 32, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds",
            {0x0c,0xe9,0x75,0xd3,0x31,0x69,0x2c,0xa8,0x0b,0xd7,0x66,0xe1,0x48,0x2e,0x0b,0x9b},{0x42,0x3f,0x19,0x43,0x7b,0x0f,0x8b,0xef,0xb8,0x53,0x14,0xd6,0x58,0xcd,0x9a,0xa3},{0xf0,0x32,0xde,0xc6,0x37,0xc3,0x90,0x24,0x18,0x7c,0x78,0x37,0xfe,0x3f,0xd2,0xc2},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x91,0xac,0x57,0x03,0x0a,0x02,0xa0,0x0e,0xb4,0x72,0xf5,0x25,0x59,0xdd,0x29,0x48},{0xf3,0x3a,0x75,0x86,0x7e,0x95,0x84,0x28,0x1b,0x9d,0x4b,0xa5,0xca,0x42,0x84,0x94},
        #else
            {0x4f,0x4f,0x01,0x64,0x58,0xd8,0xda,0x72,0x8b,0xb8,0x20,0x3e,0x87,0x4f,0x48,0x8a},{0x91,0xac,0x57,0x03,0x0a,0x02,0xa0,0x0e,0xb4,0x72,0xf5,0x25,0x59,0xdd,0x29,0x48},{0xf3,0x3a,0x75,0x86,0x7e,0x95,0x84,0x28,0x1b,0x9d,0x4b,0xa5,0xca,0x42,0x84,0x94},
        #endif
            {0}, {0} },
        { FLAGS_SKIP_SRGB, 400, 400, { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SplashScreen2.dds",
            {0xee,0xd2,0x39,0x12,0xe2,0x92,0x76,0x78,0xe1,0x29,0xfc,0xe3,0x49,0x7f,0x48,0x04},{0xb3,0xea,0x5a,0xa3,0x88,0x3a,0x1a,0xce,0xc0,0x79,0x3a,0x07,0xea,0xbf,0x1f,0x9a},{0xba,0x1e,0x6c,0x5c,0x9d,0x72,0x3d,0xa3,0x77,0x62,0x68,0x51,0x52,0xea,0x7f,0xa6},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0x53,0xdb,0x6b,0xf6,0x02,0x6f,0x40,0xab,0x42,0x5d,0x96,0xe8,0x14,0xc9,0x22,0x4f},{0x0f,0x92,0x3a,0xc9,0xba,0xd3,0xca,0x38,0x7f,0xb4,0xca,0xa5,0x7f,0xfa,0x21,0xf4},
        #else
            {0},{0x53,0xdb,0x6b,0xf6,0x02,0x6f,0x40,0xab,0x42,0x5d,0x96,0xe8,0x14,0xc9,0x22,0x4f},{0x5c,0xde,0x54,0xcc,0xc5,0xaf,0x22,0x2b,0x03,0x64,0xf3,0xfe,0x85,0xdd,0x43,0xfc},
        #endif
            {0},{0} },
        { FLAGS_SEPALPHA | FLAGS_SKIP_POINTNOWIC | FLAGS_SKIP_SRGB, 256, 128, { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.dds",
            {0xe6,0x82,0x54,0x75,0xe9,0x54,0x39,0xdb,0xcf,0xab,0x0b,0x00,0x53,0x48,0x86,0x8c},{0x59,0xf5,0x9c,0xfd,0x67,0xa9,0x8f,0x47,0x9e,0xf6,0x48,0xff,0x97,0x48,0xa7,0x8c},{0x2f,0xb1,0x1b,0x03,0x6c,0x06,0x70,0xdf,0x92,0x87,0x10,0xb3,0x92,0x6b,0x03,0x8d},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0xd5,0xfe,0xbf,0x48,0x6e,0x34,0x71,0x83,0x08,0x56,0x51,0xaf,0x91,0x1d,0x64,0x87},{0xd5,0xfe,0xbf,0x48,0x6e,0x34,0x71,0x83,0x08,0x56,0x51,0xaf,0x91,0x1d,0x64,0x87},{0x06,0x4e,0xa1,0xb4,0x51,0x55,0x90,0x94,0xea,0x39,0x8e,0x80,0xc9,0x08,0xf6,0x77},
        #else
            {0x15,0x50,0x9e,0x8f,0xdf,0x85,0x37,0x10,0xac,0x85,0x5e,0xdc,0x98,0xa4,0x35,0x86},{0xd5,0xfe,0xbf,0x48,0x6e,0x34,0x71,0x83,0x08,0x56,0x51,0xaf,0x91,0x1d,0x64,0x87},{0x68,0x56,0x73,0xc3,0x41,0xcf,0x16,0xc6,0xf8,0xda,0xe4,0xcf,0x0c,0x39,0xcd,0x98},
        #endif
            {0xe6,0x82,0x54,0x75,0xe9,0x54,0x39,0xdb,0xcf,0xab,0x0b,0x00,0x53,0x48,0x86,0x8c},{0x44,0x39,0x74,0xb3,0x2e,0x72,0x02,0xe3,0x1d,0x62,0xc9,0x08,0xc7,0xec,0x3a,0x30} },
        { ALTMD5B(8) | ALTMD5(3), 160, 128, { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testpatternYUY2.dds",
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0x55,0x9d,0xca,0x7e,0x9d,0x13,0x48,0xd2,0x11,0xcd,0x57,0x6e,0x8d,0x57,0xa7,0xf9},{0x16,0x5f,0xbb,0x02,0x84,0x48,0x8e,0x38,0xeb,0x74,0xef,0x84,0xf8,0x40,0x84,0xb6},{0xe1,0x32,0xda,0xee,0xb8,0xe2,0x52,0x1d,0xe7,0xd4,0x0c,0x05,0xc3,0xd7,0xfe,0xe0},
        #else
            {0xa0,0x63,0xe2,0xc6,0x2c,0x64,0x23,0xa0,0x19,0x24,0xcf,0x55,0x40,0x26,0x58,0x94},{0x16,0x5f,0xbb,0x02,0x84,0x48,0x8e,0x38,0xeb,0x74,0xef,0x84,0xf8,0x40,0x84,0xb6},{0xe1,0x32,0xda,0xee,0xb8,0xe2,0x52,0x1d,0xe7,0xd4,0x0c,0x05,0xc3,0xd7,0xfe,0xe0},
        #endif
            {0x87,0x92,0xb0,0xe1,0x3f,0xff,0xe3,0xb9,0x59,0xe7,0x07,0x67,0x0a,0x36,0xbc,0x5f},{0xec,0xb7,0x75,0x97,0xe7,0x2a,0x3a,0xcb,0x95,0xf0,0x9c,0x0f,0x87,0x89,0xfc,0xa7},{0x15,0xa2,0xb4,0xb6,0x40,0xa4,0x48,0x5a,0x4e,0xae,0x73,0xbc,0xbb,0xcd,0xd3,0x00},
            {0},{0} },
        { ALTMD5(4), 128, 128, { 200, 200, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaCubeYUY2.dds",
            {0xe2,0xfb,0x0e,0x0c,0x78,0x88,0x48,0x86,0x2d,0x16,0xed,0x61,0x9b,0x3e,0xa0,0x40},{0xb3,0x30,0x5f,0x15,0x93,0x90,0xbf,0x09,0x88,0x14,0xaf,0xef,0x97,0x85,0x57,0x22},{0x4c,0x53,0x73,0x5a,0x2a,0x2c,0xc0,0xf1,0x3b,0xeb,0x8a,0xfe,0x6c,0x60,0xa1,0xfa},
        #if defined(_M_ARM64) || defined(_M_ARM64EC)
            {0},{0xe1,0xc1,0x8a,0xb9,0xfe,0xb8,0x48,0xb9,0x07,0xf4,0x22,0x53,0xd4,0x0b,0x13,0x84},{0x2e,0xc9,0x13,0x06,0x8f,0x02,0x62,0xa1,0x03,0xde,0x6f,0xd2,0xb9,0xec,0x3a,0x3e},
        #elif defined(_M_X64)
            {0},{0xe1,0xc1,0x8a,0xb9,0xfe,0xb8,0x48,0xb9,0x07,0xf4,0x22,0x53,0xd4,0x0b,0x13,0x84},{0x2e,0xc9,0x13,0x06,0x8f,0x02,0x62,0xa1,0x03,0xde,0x6f,0xd2,0xb9,0xec,0x3a,0x3e},
        #else
            {0xa1,0x21,0x78,0x5b,0xbf,0x5a,0x53,0x95,0x3a,0xa8,0x2e,0x53,0xd8,0x3a,0x9e,0x1a},{0},{0x2e,0xc9,0x13,0x06,0x8f,0x02,0x62,0xa1,0x03,0xde,0x6f,0xd2,0xb9,0xec,0x3a,0x3e},
        #endif
            {0xee,0xd5,0x88,0xf6,0x90,0x5b,0x0d,0xc7,0x15,0xa4,0x6d,0x7b,0x5d,0x23,0xf3,0x40},{0} },
        { ALTMD5(5), 128, 128, { 200, 200, 4, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"lenaVolYUY2.dds",
            {0x06,0x29,0x99,0x69,0xb2,0xc3,0xbb,0xc4,0x14,0xfa,0xfd,0x35,0x82,0xa7,0x33,0xb3},{0x4a,0xae,0x5f,0xc0,0xff,0x8c,0xc6,0x53,0xdd,0x0f,0x8c,0x01,0xdc,0x69,0x1a,0xbf},{0xbc,0xc6,0x6e,0x45,0x4c,0x2e,0x3a,0x07,0x0d,0xbc,0x59,0x38,0xb6,0x4d,0x91,0xf7},
        #ifdef _M_X64
            {0},{0x35,0xf3,0x6c,0xe2,0x41,0xf5,0xd7,0x66,0xfb,0x9e,0x6c,0xee,0x4d,0xbd,0xc7,0x44},{0x8f,0x4b,0x7b,0xe8,0x2d,0xc6,0x32,0x62,0xb6,0xf2,0x11,0xb9,0xf1,0xe1,0x79,0x7f},
        #else
            {0x92,0xc2,0xe2,0xfd,0x31,0xdc,0x0a,0x54,0x6e,0x10,0x89,0x40,0x85,0x5e,0x31,0x83},{0},{0x8f,0x4b,0x7b,0xe8,0x2d,0xc6,0x32,0x62,0xb6,0xf2,0x11,0xb9,0xf1,0xe1,0x79,0x7f},
        #endif
            {0x05,0xd9,0x77,0xee,0x63,0xf9,0x3c,0x8e,0x21,0x56,0x92,0xb2,0x2e,0xdd,0xa8,0x58},{0} },

        #if defined(_M_X64) && !defined(_M_ARM64EC)
        { FLAGS_NAMECONFLICT | FLAGS_SKIP_EXHAUSTIVE, 16384, 8192, { 21600, 10800, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.png",
        // Very large images
            {0xca,0x0f,0xb6,0x59,0x97,0xe5,0xe3,0x9d,0xea,0xe9,0x6e,0x7b,0xaa,0x4f,0xaa,0x4d},{0},{0},
            {0},{0},{0},
            {0},{0} },
        { FLAGS_SKIP_EXHAUSTIVE, 16384, 8192, { 8192, 4096, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"callisto.png",
            {0x29,0x4b,0xed,0x4e,0x0e,0xdf,0x10,0xe2,0x8d,0x78,0x32,0x45,0x77,0xe9,0xff,0x85},{0},{0},
            {0},{0},{0},
            {0},{0} },
        { FLAGS_SKIP_EXHAUSTIVE, 8192, 8192, { 16384, 16384, 1, 1, 15, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k.dds",
            {0xf1,0xcb,0xce,0x07,0x80,0x4c,0x7f,0x82,0x24,0x62,0x70,0xbe,0x63,0x36,0xef,0x37},{ 0 },{ 0 },
            {0},{0},{0},
            {0},{0} },
        { FLAGS_SKIP_EXHAUSTIVE, 8192, 8192, { 16384, 16384, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_SNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth16kby16k_snorm.dds",
            {0xc6,0xff,0x3a,0x67,0x01,0x3c,0x2c,0x07,0x07,0xfa,0xec,0xb1,0x19,0xba,0xd0,0x0e}, { 0 }, { 0 },
            {0},{0},{0},
            {0},{0} },
        #endif
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

    const AltMD5 g_AltMD5[] =
    {
        { {0},{0},{0},
          {0},{0xef,0x5d,0x7c,0xe7,0xfa,0xce,0x8b,0x83,0x5b,0x3d,0x0d,0x15,0x5b,0xef,0x02,0xc8},
          {0},{0} }, // ALTMD5(1)
        { {0xce,0x79,0x68,0x49,0xe1,0x71,0x85,0xb6,0xe0,0xa7,0x26,0x07,0x09,0xf2,0x99,0xf9},{0},{0xce,0x79,0x68,0x49,0xe1,0x71,0x85,0xb6,0xe0,0xa7,0x26,0x07,0x09,0xf2,0x99,0xf9},
          {0},{0},
          {0},{0} }, // ALTMD5(2)
        { {0x91,0xbc,0x51,0xa9,0x41,0x0c,0x0f,0x68,0x79,0x05,0x1d,0x80,0xdc,0xff,0xe0,0x42},{0},{0},
          {0},{0x5b,0xcc,0x55,0x5d,0x8e,0x86,0xff,0x5a,0x8e,0xc3,0x2c,0xd7,0x1a,0xf1,0x3a,0xf6},
          {0},{0} }, // ALTMD5(3)
        { {0xb4,0x6e,0x65,0x7b,0x63,0xe5,0xae,0xfd,0x0f,0xa6,0x52,0xe1,0x71,0x08,0x0c,0x57},{0},{0},
          {0},{0},
          {0x31,0x57,0xd1,0x5a,0x5b,0x1a,0x3d,0xd7,0x99,0x4f,0x65,0x5f,0x9c,0x1d,0x79,0x76},{0} }, // ALTMD5(4)
        { {0xf1,0x12,0xc8,0xfe,0x3e,0x27,0x1a,0xf1,0x74,0xeb,0x4d,0xcc,0xfd,0x5b,0x00,0xfc},{0},{0},
          {0},{0},
          {0x50,0x17,0xba,0xfb,0xfd,0xa3,0xbe,0xfa,0x9b,0x19,0x99,0x59,0x47,0x02,0xdb,0x9d},{0} }, // ALTMD5(5)
        {}, // ALTMD5(6) unused
        {}, // ALTMD5(7) unused
        { {0x29,0xe8,0xc5,0x1f,0x1c,0x6f,0xef,0xbb,0xa3,0x98,0xf3,0x64,0x19,0xbd,0x29,0x81}, {0}, {0},
          {0}, {0},
          {0}, {0} }, // ALTMD5(8)
        { { 0x74,0xc2,0xea,0x7f,0x41,0x3d,0x38,0x3f,0x8f,0xc5,0x36,0xfb,0x96,0xc7,0xd6,0xda }, {0}, {0},
          {0}, {0},
          {0}, {0} }, // ALTMD5(9)
        { { 0x21,0x06,0x9b,0x6c,0x21,0xa3,0x83,0x7f,0x03,0x22,0xbd,0x82,0xbe,0x84,0xe3,0x84 }, {0}, {0},
          {0}, {0},
          {0}, {0} }, // ALTMD5(10)
        {}, // ALTMD5(11) unused
        {}, // ALTMD5(12) unused
        { { 0x8f,0x02,0x67,0xdb,0xb9,0xae,0xd1,0xcc,0x16,0x3a,0x62,0x11,0x38,0x05,0x17,0x33 }, {0}, {0},
          {0}, {0},
          {0}, {0} }, // ALTMD5(13)
        { { 0x4d,0x17,0x0c,0xc9,0x21,0x08,0x6a,0xd4,0xae,0x73,0xda,0x30,0xcc,0x14,0xd5,0x4e }, { 0xe9,0xb1,0x7b,0x3c,0x02,0x02,0x65,0x99,0x92,0xe6,0x31,0x92,0x62,0x2c,0x23,0xd8 }, {0},
          {0}, {0},
          {0}, { 0x4d,0x17,0x0c,0xc9,0x21,0x08,0x6a,0xd4,0xae,0x73,0xda,0x30,0xcc,0x14,0xd5,0x4e } }, // ALTMD5(14)
    };

    inline bool IsErrorTooSmall(float f, float threshold)
    {
        return (fabsf(f) < threshold) != 0;
    }

    inline bool IsErrorTooLarge(float f, float threshold)
    {
        return (fabsf(f) > threshold) != 0;
    }
}

//-------------------------------------------------------------------------------------

extern HRESULT MD5Checksum(_In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages = 0);
extern HRESULT SaveScratchImage(_In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image);

//-------------------------------------------------------------------------------------
// Resize
bool FilterTest::Test01()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_ResizeMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_ResizeMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        if ( g_ResizeMedia[index].options & FLAGS_NAMECONFLICT )
        {
            wcscat_s( fname, L"B" );
        }

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"resize", tempDir, MAX_PATH);
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

        const TexMetadata* check = &g_ResizeMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting DDS data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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
            bool pass = true;

            print(".");

            UINT twidth = g_ResizeMedia[index].twidth;
            UINT theight = g_ResizeMedia[index].theight;

            wchar_t fname2[MAX_PATH] = {};
            swprintf_s( fname2, L"%s_%ux%u", fname, twidth, theight );

            //--- Simple resize -------------------------------------------------------
            ScratchImage image;
            hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_DEFAULT, image );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
            }
            else if ( image.GetMetadata().width != twidth || image.GetMetadata().height != theight )
            {
                success = false;
                pass = false;
                printe( "Failed resizing result is %zu x %zu:\n",
                        image.GetMetadata().width, image.GetMetadata().height );
                printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
            }
            else
            {
                // Verify the image data
                const uint8_t* expected2 = nullptr;
                if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK )
                {
                    expected2 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5;
                }

                const uint8_t* expected3 = nullptr;
                if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASKB )
                {
                    expected3 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASKB) >> 12) - 1 ].md5;
                }

                uint8_t digest[16];
                hr = MD5Checksum( image, digest );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                }
                else if ( memcmp( digest, g_ResizeMedia[index].md5, 16 ) != 0
                          && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                          && (!expected3 || memcmp( digest, expected3, 16 ) != 0 )
                          && (index != 22) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed comparing image MD5 checksum (%u x %u):\n%ls\n", twidth, theight, szPath );
                    printdigest( "computed", digest );
                    printdigest( "expected", g_ResizeMedia[index].md5 );
                    if ( expected2 )
                    {
                        printdigest( "expected2", expected2 );
                    }
                    if ( expected3 )
                    {
                        printdigest( "expected3", expected3 );
                    }
                }

                wchar_t szDestPath[MAX_PATH] = {};
                _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname2, L".dds" );

                SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );

#if defined(DEBUG) && !defined(NO_WMP)
                _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname2, L".wdp" );
                SaveToWICFile( *image.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
            }

            if (FAILED(hr))
                continue;

            ScratchImage imagePoint;
            ScratchImage imageLinear;
            ScratchImage imageCubic;
            ScratchImage imageTriangle;
            if ( !(g_ResizeMedia[index].options & FLAGS_SKIP_EXHAUSTIVE) )
            {
                // POINT
                hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_POINT, imagePoint );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Point (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else if ( imagePoint.GetMetadata().width != twidth || imagePoint.GetMetadata().height != theight )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Point result is %zu x %zu:\n",
                            imagePoint.GetMetadata().width, imagePoint.GetMetadata().height );
                    printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                }
                else
                {
                    // Verify the image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_point;
                    }

                    const uint8_t* expected3 = nullptr;
                    if (g_ResizeMedia[index].options & FLAGS_ALTMD5_MASKB)
                    {
                        expected3 = g_AltMD5[((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASKB) >> 12) - 1].md5_point;
                    }

                    uint8_t digest[16];
                    hr = MD5Checksum( imagePoint, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image Point data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( memcmp( digest, g_ResizeMedia[index].md5_point, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 )
                              && (!expected3 || memcmp(digest, expected3, 16) != 0) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing image Point MD5 checksum (%u x %u):\n%ls\n", twidth, theight, szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_ResizeMedia[index].md5_point );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                        if (expected3)
                        {
                            printdigest("expected3", expected3);
                        }
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname2 );
                    wcscat_s( tname, L"_POINT" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, imagePoint );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *imagePoint.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                }

                // LINEAR
                hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_LINEAR, imageLinear );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Linear (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else if ( imageLinear.GetMetadata().width != twidth || imageLinear.GetMetadata().height != theight )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Linear result is %zu x %zu:\n",
                            imageLinear.GetMetadata().width, imageLinear.GetMetadata().height );
                    printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                }
                else
                {
                    // Verify the image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_linear;
                    }

                    uint8_t digest[16];
                    hr = MD5Checksum( imageLinear, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image Linear data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( memcmp( digest, g_ResizeMedia[index].md5_linear, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing image Linear MD5 checksum (%u x %u):\n%ls\n", twidth, theight, szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_ResizeMedia[index].md5_linear );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    float targMSE = 0.06f;
                    float mse = 0, mseV[4] = {};
                    const Image* img = imagePoint.GetImage(0,0,0);
                    hr = (img) ? ComputeMSE( *img, *imageLinear.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Linear image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Linear MSE = %f (%f %f %f %f)... %f (%u x %u):\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname2 );
                    wcscat_s( tname, L"_LINEAR" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, imageLinear );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *imageLinear.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                }

                // CUBIC
                hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_CUBIC, imageCubic );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Cubic (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else if ( imageCubic.GetMetadata().width != twidth || imageCubic.GetMetadata().height != theight )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Cubic result is %zu x %zu:\n",
                            imageCubic.GetMetadata().width, imageCubic.GetMetadata().height );
                    printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                }
                else
                {
                    // Verify the image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_cubic;
                    }

                    uint8_t digest[16];
                    hr = MD5Checksum( imageCubic, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image Cubic data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( memcmp( digest, g_ResizeMedia[index].md5_cubic, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing image Cubic MD5 checksum  (%u x %u):\n%ls\n", twidth, theight, szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_ResizeMedia[index].md5_cubic );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    float targMSE = 0.07f;
                    float mse = 0, mseV[4] = {};
                    const Image* img = imagePoint.GetImage(0,0,0);
                    hr = (img) ? ComputeMSE( *img, *imageCubic.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Cubic image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing Point vs. Cubic MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                    }

                    targMSE = 0.02f;
                    img = imageLinear.GetImage(0,0,0);
                    hr = (img) ? ComputeMSE( *img, *imageCubic.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing Linear vs. Cubic image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing Linear vs. Cubic MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname2 );
                    wcscat_s( tname, L"_CUBIC" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, imageCubic );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *imageCubic.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                }

                // TRIANGLE
                hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_TRIANGLE, imageTriangle );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Triangle (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else if ( imageTriangle.GetMetadata().width != twidth || imageTriangle.GetMetadata().height != theight )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing Triangle result is %zu x %zu:\n",
                            imageTriangle.GetMetadata().width, imageTriangle.GetMetadata().height );
                    printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                }
                else
                {
                    // Verify the image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_tri;
                    }

                    uint8_t digest[16];
                    hr = MD5Checksum( imageTriangle, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image Triangle data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( memcmp( digest, g_ResizeMedia[index].md5_tri, 16 ) != 0
                              && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ))
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing image Triangle MD5 checksum  (%u x %u):\n%ls\n", twidth, theight, szPath );
                        printdigest( "computed", digest );
                        printdigest( "expected", g_ResizeMedia[index].md5_tri );
                        if ( expected2 )
                        {
                            printdigest( "expected2", expected2 );
                        }
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname2 );
                    wcscat_s( tname, L"_TRIANGLE" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, imageTriangle );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *imageTriangle.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                }
            }

            // SEPALPHA
            if ( g_ResizeMedia[index].options & FLAGS_SEPALPHA )
            {
                ScratchImage imageSepAlpha;
                hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_SEPARATE_ALPHA, imageSepAlpha );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing sep alpha (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else if ( imageSepAlpha.GetMetadata().width != twidth || imageSepAlpha.GetMetadata().height != theight )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing sep alpha result is %zu x %zu:\n",
                            imageSepAlpha.GetMetadata().width, imageSepAlpha.GetMetadata().height );
                    printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                }
                else
                {
                    // Verify the image data
                    const uint8_t* expected2 = nullptr;
                    if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK )
                    {
                        expected2 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_sepalpha;
                    }

                    const uint8_t* expected3 = nullptr;
                    if (g_ResizeMedia[index].options & FLAGS_ALTMD5_MASKB)
                    {
                        expected3 = g_AltMD5[((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASKB) >> 12) - 1].md5_sepalpha;
                    }

                    uint8_t digest[16];
                    hr = MD5Checksum( imageSepAlpha, digest );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed computing MD5 checksum of image sep alpha data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if (memcmp(digest, g_ResizeMedia[index].md5_sepalpha, 16) != 0
                        && (!expected2 || memcmp(digest, expected2, 16) != 0)
                        && (!expected3 || memcmp(digest, expected3, 16) != 0))
                    {
                        success = false;
                        pass = false;
                        printe("Failed comparing image sep alpha MD5 checksum  (%u x %u):\n%ls\n", twidth, theight, szPath);
                        printdigest("computed", digest);
                        printdigest("expected", g_ResizeMedia[index].md5_sepalpha);
                        if (expected2)
                        {
                            printdigest("expected2", expected2);
                        }
                        if (expected3)
                        {
                            printdigest("expected3", expected3);
                        }
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname2 );
                    wcscat_s( tname, L"_SEPALPHA" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, imageSepAlpha );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *imageSepAlpha.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                }
            }

            //--- WIC vs. non-WIC resize ----------------------------------------------
            ScratchImage nwimage;
            ScratchImage nwimageLinear;
            ScratchImage nwimageCubic;
            if ( BitsPerColor( metadata.format ) <= 8 && !IsSRGB(metadata.format) )
            {
                // non-WIC is already used when color-depth is > 8 and for sRGB

                hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_FORCE_NON_WIC, nwimage );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed non-WIC resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else if ( nwimage.GetMetadata().width != twidth || nwimage.GetMetadata().height != theight )
                {
                    success = false;
                    pass = false;
                    printe( "Failed non-WIC resizing result is %zu x %zu:\n",
                            nwimage.GetMetadata().width, nwimage.GetMetadata().height );
                    printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                }
                else
                {
                    float targMSE = ( ( g_ResizeMedia[index].options & FLAGS_SEPALPHA ) || ( twidth > metadata.width || theight > metadata.height) ) ? 0.071f : 0.003f;
                    float mse = 0, mseV[4] = {};
                    const Image* img = image.GetImage(0,0,0);
                    hr = (img) ? ComputeMSE( *img, *nwimage.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing non-WIC to WIC image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing non-WIC to WIC MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname2 );
                    wcscat_s( tname, L"_nowic" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwimage );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *nwimage.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                }

                if ( !(g_ResizeMedia[index].options & FLAGS_SKIP_EXHAUSTIVE) )
                {
                    // POINT
                    ScratchImage nwimagePoint;
                    hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_FORCE_NON_WIC | TEX_FILTER_POINT, nwimagePoint );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed non-WIC Point resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                    }
                    else if ( nwimagePoint.GetMetadata().width != twidth || nwimagePoint.GetMetadata().height != theight )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed non-WIC Point resizing result is %zu x %zu:\n",
                                nwimagePoint.GetMetadata().width, nwimagePoint.GetMetadata().height );
                        printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                    }
                    else
                    {
                        if ( !(g_ResizeMedia[index].options & FLAGS_SKIP_POINTNOWIC) )
                        {
                            float targMSE = ( g_ResizeMedia[index].options & FLAGS_SEPALPHA ) ? 0.11f : 0.047f;
                            float mse = 0, mseV[4] = {};
                            const Image* img = imagePoint.GetImage(0,0,0);
                            hr = (img) ? ComputeMSE( *img, *nwimagePoint.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                            if ( FAILED(hr) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC to WIC Point image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                            }
                            else if ( IsErrorTooLarge( mse, targMSE ) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC to WIC Point MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                            }
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname2 );
                        wcscat_s( tname, L"_POINT_nowic" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwimagePoint );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *nwimagePoint.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                    }

                    // LINEAR
                    hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_FORCE_NON_WIC | TEX_FILTER_LINEAR, nwimageLinear );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed non-WIC Linear resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                    }
                    else if ( nwimageLinear.GetMetadata().width != twidth || nwimageLinear.GetMetadata().height != theight )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed non-WIC Linear resizing result is %zu x %zu:\n",
                                nwimageLinear.GetMetadata().width, nwimageLinear.GetMetadata().height );
                        printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                    }
                    else
                    {
                        float targMSE = ( g_ResizeMedia[index].options & FLAGS_SEPALPHA ) ? 0.072f : 0.003f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = imageLinear.GetImage(0,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwimageLinear.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Linear image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Linear MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname2 );
                        wcscat_s( tname, L"_LINEAR_nowic" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwimageLinear );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *nwimageLinear.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                    }

                    // CUBIC
                    hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_FORCE_NON_WIC | TEX_FILTER_CUBIC, nwimageCubic );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed non-WIC Cubic resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                    }
                    else if ( nwimageCubic.GetMetadata().width != twidth || nwimageCubic.GetMetadata().height != theight )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed non-WIC Cubic resizing result is %zu x %zu:\n",
                                nwimageCubic.GetMetadata().width, nwimageCubic.GetMetadata().height );
                        printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                    }
                    else
                    {
                        float targMSE = ( g_ResizeMedia[index].options & FLAGS_SEPALPHA ) ? 0.09f : 0.06f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = imageCubic.GetImage(0,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwimageCubic.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Cubic image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC to WIC Cubic MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                        }

                        if ( !(g_ResizeMedia[index].options & FLAGS_SKIP_POINTNOWIC) )
                        {
                            targMSE = 0.052f;
                            img = nwimagePoint.GetImage(0,0,0);
                            hr = (img) ? ComputeMSE( *img, *nwimageCubic.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                            if ( FAILED(hr) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC Point vs. Cubic image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                            }
                            else if ( IsErrorTooLarge( mse, targMSE ) )
                            {
                                success = false;
                                printe( "Failed comparing non-WIC Point vs. Cubic MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                            }
                        }

                        targMSE = (g_ResizeMedia[index].options & FLAGS_SKIP_POINTNOWIC) ? 0.053f : 0.03f;
                        img = nwimageLinear.GetImage(0,0,0);
                        hr = (img) ? ComputeMSE( *img, *nwimageCubic.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC Linear vs. Cubic image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing non-WIC Linear vs. Cubic MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname2 );
                        wcscat_s( tname, L"_CUBIC_nowic" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, nwimageCubic );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *nwimageCubic.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                    }
                }
            }

            //--- sRGB correct filtering -----------------------------------------------
            DXGI_FORMAT srgb = MakeSRGB( metadata.format );
            if ( srgb != metadata.format && !(g_ResizeMedia[index].options & FLAGS_SKIP_SRGB) )
            {
                ScratchImage srgbimage;
                hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_SRGB, srgbimage );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed sRGB resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else if ( srgbimage.GetMetadata().width != twidth || srgbimage.GetMetadata().height != theight )
                {
                    success = false;
                    pass = false;
                    printe( "Failed sRGB resizing result is %zu x %zu:\n",
                            srgbimage.GetMetadata().width, srgbimage.GetMetadata().height );
                    printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                }
                else
                {
                    float targMSE = 0.0011f;
                    float mse = 0, mseV[4] = {};
                    const Image* img = nwimage.GetImage(0,0,0);
                    hr = (img) ? ComputeMSE( *img, *srgbimage.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                    if ( FAILED(hr) )
                    {
                        success = false;
                        printe( "Failed comparing sRGB to non-WIC image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( IsErrorTooSmall( mse, 0.00005f ) )
                    {
                        success = false;
                        printe( "Failed comparing sRGB to non-WIC which appear near identical MSE = %f (%f %f %f %f)  (%u x %u):\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], twidth, theight, szPath );
                    }
                    else if ( IsErrorTooLarge( mse, targMSE ) )
                    {
                        success = false;
                        printe( "Failed comparing sRGB to non-WIC MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname2 );
                    wcscat_s( tname, L"_srgb" );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbimage );

#if defined(DEBUG) && !defined(NO_WMP)
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                    SaveToWICFile( *srgbimage.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                }

                if ( !(g_ResizeMedia[index].options & FLAGS_SKIP_EXHAUSTIVE) )
                {
                    // LINEAR
                    ScratchImage srgbimageLinear;
                    hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_SRGB | TEX_FILTER_LINEAR, srgbimageLinear );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed sRGB Linear resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                    }
                    else if ( srgbimageLinear.GetMetadata().width != twidth || srgbimageLinear.GetMetadata().height != theight )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed sRGB Linear resizing result is %zu x %zu:\n",
                                srgbimageLinear.GetMetadata().width, srgbimageLinear.GetMetadata().height );
                        printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                    }
                    else
                    {
                        float targMSE = 0.0011f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = nwimageLinear.GetImage(0,0,0);
                        hr = (img) ? ComputeMSE( *img, *srgbimageLinear.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Linear to non-WIC image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( IsErrorTooSmall( mse, 0.00005f ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Linear to non-WIC which appear near identical MSE = %f (%f %f %f %f)  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], twidth, theight, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Linear to non-WIC MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname2 );
                        wcscat_s( tname, L"_LINEAR_srgb" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbimageLinear );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *srgbimageLinear.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                    }

                    // CUBIC
                    ScratchImage srgbimageCubic;
                    hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_SRGB | TEX_FILTER_CUBIC, srgbimageCubic );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed sRGB Cubic resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                    }
                    else if ( srgbimageCubic.GetMetadata().width != twidth || srgbimageCubic.GetMetadata().height != theight )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed sRGB Cubic resizing result is %zu x %zu:\n",
                                srgbimageCubic.GetMetadata().width, srgbimageCubic.GetMetadata().height );
                        printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                    }
                    else
                    {
                        float targMSE = 0.0011f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = nwimageCubic.GetImage(0,0,0);
                        hr = (img) ? ComputeMSE( *img, *srgbimageCubic.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Cubic to non-WIC image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( IsErrorTooSmall( mse, 0.00005f ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Cubic to non-WIC which appear near identical MSE = %f (%f %f %f %f)  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], twidth, theight, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Cubic to non-WIC MSE = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname2 );
                        wcscat_s( tname, L"_CUBIC_srgb" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbimageCubic );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *srgbimageCubic.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                    }

                    // TRIANGLE
                    ScratchImage srgbimageTriangle;
                    hr = Resize( *srcimage.GetImage(0,0,0), twidth, theight, TEX_FILTER_SRGB | TEX_FILTER_TRIANGLE, srgbimageTriangle );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed sRGB Triangle resizing (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                    }
                    else if ( srgbimageTriangle.GetMetadata().width != twidth || srgbimageTriangle.GetMetadata().height != theight )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed sRGB Triangle resizing result is %zu x %zu:\n",
                                srgbimageTriangle.GetMetadata().width, srgbimageTriangle.GetMetadata().height );
                        printe( "\n...(check) %u x %u:\n%ls\n", twidth, theight, szPath );
                    }
                    else
                    {
                        float targMSE = 0.0011f;
                        float mse = 0, mseV[4] = {};
                        const Image* img = imageTriangle.GetImage(0,0,0);
                        hr = (img) ? ComputeMSE( *img, *srgbimageTriangle.GetImage(0,0,0), mse, mseV ) : E_POINTER;
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Triangle to default image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( IsErrorTooSmall( mse, 0.00005f ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Triangle to default which appear near identical MSE = %f (%f %f %f %f)  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], twidth, theight, szPath );
                        }
                        else if ( IsErrorTooLarge( mse, targMSE ) )
                        {
                            success = false;
                            printe( "Failed comparing sRGB Triangle to default = %f (%f %f %f %f)... %f  (%u x %u):\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, twidth, theight, szPath );
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname2 );
                        wcscat_s( tname, L"_TRIANGLE_srgb" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, srgbimageTriangle );

#if defined(DEBUG) && !defined(NO_WMP)
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".wdp" );
                        SaveToWICFile( *srgbimageTriangle.GetImage(0,0,0), WIC_FLAGS_NONE, GetWICCodec( WIC_CODEC_WMP ), szDestPath );
#endif
                    }
                }
            }

            //--- non-WIC resize ------------------------------------------------------
// TODO - TEX_FILTER_MIRROR_U, TEX_FILTER_MIRROR_V
// TODO - TEX_FILTER_WRAP_U, TEX_FILTER_WRAP_V

            //--- Complex resize ------------------------------------------------------
            if ( srcimage.GetImageCount() > 1 && ( (metadata.width * metadata.height) < (16384 * 16384) ) )
            {
                ScratchImage imageComplex;
                hr = Resize( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), twidth, theight, TEX_FILTER_DEFAULT, imageComplex );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed resizing complex (HRESULT %08X) to %u x %u:\n%ls\n", static_cast<unsigned int>(hr), twidth, theight, szPath );
                }
                else
                {
                    const TexMetadata& metadata2 = imageComplex.GetMetadata();

                    if ( metadata2.width != twidth
                         || metadata2.height != theight
                         || metadata2.mipLevels != 1
                         || metadata2.depth != check->depth
                         || metadata2.arraySize != check->arraySize
                         || metadata2.miscFlags != check->miscFlags
                         || metadata2.format != check->format
                         || metadata2.dimension != check->dimension )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed resizing complex result:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                        printmetachk( check );
                        printe( "...\n(check) w: %u x h: %u x d: %zu\narraySize: %zu  mips: 1\nmiscFlags: %08X miscFlags2: %08X format: %d dimension: %u\n",
                                twidth, theight, check->depth, check->arraySize, check->miscFlags, check->miscFlags2, check->format, check->dimension );
                    }
                    else
                    {
                        // Verify the image data
                        const uint8_t* expected2 = nullptr;
                        if ( g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK )
                        {
                            expected2 = g_AltMD5[ ((g_ResizeMedia[index].options & FLAGS_ALTMD5_MASK) >> 4) - 1 ].md5_c;
                        }

                        // Verify the image data
                        uint8_t digest[16];
                        hr = MD5Checksum( imageComplex, digest );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed computing MD5 checksum of image complex data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( memcmp( digest, g_ResizeMedia[index].md5_c, 16 ) != 0
                                  && (!expected2 || memcmp( digest, expected2, 16 ) != 0 ) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed comparing image complex MD5 checksum  (%u x %u):\n%ls\n", twidth, theight, szPath );
                            printdigest( "computed", digest );
                            printdigest( "expected", g_ResizeMedia[index].md5_c );
                            if ( expected2 )
                            {
                                printdigest( "expected2", expected2 );
                            }
                        }

                        wchar_t tname[MAX_PATH] = {};
                        wcscpy_s( tname, fname2 );
                        wcscat_s( tname, L"_complex" );

                        wchar_t szDestPath[MAX_PATH] = {};
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                        SaveToDDSFile( imageComplex.GetImages(), imageComplex.GetImageCount(), imageComplex.GetMetadata(), DDS_FLAGS_NONE, szDestPath );
                    }
                }
            }

            if ( pass )
                ++npass;
        }

        ++ncount;
    }

    print("\n%zu images tested, %zu images passed ", ncount, npass );

    return success;
}
