//-------------------------------------------------------------------------------------
// tga.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

#include "scoped.h"

using namespace DirectX;

#define ALTMD5(n) (n << 4)

namespace
{
    enum
    {
        FLAGS_NONE = 0x0,
        FLAGS_24BPP = 0x1,
        FLAGS_RGB_BGR_SAME = 0x2,
        FLAGS_ZALPHA = 0x4,

        FLAGS_ALTMD5_MASK = 0xf0,
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
        // test-options | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { FLAGS_NONE, { 320, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bounce.tga",{ 0x0a,0x27,0xfb,0x35,0x57,0x6a,0x07,0x10,0x4a,0xf0,0x11,0x79,0xec,0x64,0x13,0x2e } },
        { FLAGS_24BPP, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth.tga", { 0x28,0xe6,0x0f,0x9b,0xc5,0x66,0xf3,0x4e,0xb0,0x0e,0x93,0xff,0x59,0x57,0xb5,0xc5 } },
        { FLAGS_ZALPHA, { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.tga", { 0x00,0x5c,0x5d,0x42,0x6c,0xed,0x64,0xc4,0xfd,0x2d,0xc5,0x67,0x89,0x2f,0x34,0x8c } },
        { FLAGS_NONE, { 320, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testunc.tga", { 0x0a,0x27,0xfb,0x35,0x57,0x6a,0x07,0x10,0x4a,0xf0,0x11,0x79,0xec,0x64,0x13,0x2e } },
        { FLAGS_NONE, { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex.tga", { 0x15,0xf3,0xd1,0x76,0x0c,0xb0,0x39,0x8d,0xd3,0x0f,0x8c,0x03,0x4f,0xd1,0x09,0x95 } },

        // TGA 2.0 conformance suite
        { FLAGS_NONE, { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CBW8.TGA", { 0x57,0x38,0x4e,0xb8,0x4c,0x01,0xb8,0x1a,0x08,0x91,0xe1,0xc1,0x11,0x76,0x93,0xfd } },
        { FLAGS_ZALPHA, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CTC16.TGA", { 0x6c,0x11,0xa9,0x94,0xa2,0x84,0x0c,0xe5,0x8c,0x34,0xbf,0xbb,0x6f,0x1d,0x3e,0x8c } },
        { FLAGS_24BPP, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CTC24.TGA", { 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
        { FLAGS_ZALPHA, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"CTC32.TGA", { 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
        { FLAGS_ZALPHA, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UTC16.TGA",{ 0x6c,0x11,0xa9,0x94,0xa2,0x84,0x0c,0xe5,0x8c,0x34,0xbf,0xbb,0x6f,0x1d,0x3e,0x8c } },
        { FLAGS_24BPP, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UTC24.TGA",{ 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
        { FLAGS_ZALPHA, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UTC32.TGA",{ 0x42,0xe3,0xbb,0x17,0x12,0x73,0x59,0x6b,0x26,0x06,0x0b,0xe0,0x21,0xeb,0x5a,0xfd } },
        { FLAGS_NONE, { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"UBW8.TGA",{ 0x57,0x38,0x4e,0xb8,0x4c,0x01,0xb8,0x1a,0x08,0x91,0xe1,0xc1,0x11,0x76,0x93,0xfd } },
        // ccm8.tga, ucm8.tga: unsupported color mapped files

        // TGA sample files
        { FLAGS_ZALPHA, { 124, 124, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_B16.TGA",{ 0xd0,0x77,0x67,0x8c,0x1b,0x6c,0xf4,0x38,0x5f,0xa6,0x04,0x66,0xbc,0xf6,0xd7,0x42 } },
        { FLAGS_24BPP, { 124, 124, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_B24.TGA",{ 0x41,0x7c,0x52,0x54,0x51,0x70,0xf8,0x39,0x17,0xf4,0xf4,0x8b,0xa4,0x75,0xdf,0x44 } },
        { FLAGS_ZALPHA, { 124, 124, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_B32.TGA",{ 0x41,0x7c,0x52,0x54,0x51,0x70,0xf8,0x39,0x17,0xf4,0xf4,0x8b,0xa4,0x75,0xdf,0x44 } },
        { FLAGS_ZALPHA, { 124, 124, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_T16.TGA",{ 0xd0,0x77,0x67,0x8c,0x1b,0x6c,0xf4,0x38,0x5f,0xa6,0x04,0x66,0xbc,0xf6,0xd7,0x42 } },
        { FLAGS_ZALPHA, { 124, 124, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"FLAG_T32.TGA",{ 0x41,0x7c,0x52,0x54,0x51,0x70,0xf8,0x39,0x17,0xf4,0xf4,0x8b,0xa4,0x75,0xdf,0x44 } },
        { FLAGS_24BPP, { 1419, 1001, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"MARBLES.TGA",{ 0x2a,0x10,0x31,0x6e,0xee,0x64,0x5b,0xb8,0x3e,0x9c,0x6e,0xca,0xc6,0xd0,0x5e,0x0b } },
        { FLAGS_ZALPHA, { 240, 164, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_B16.TGA",{ 0x57,0x9f,0x98,0xdd,0x7d,0x65,0x26,0x2f,0xf1,0x68,0x98,0x84,0x25,0x65,0x62,0x6b } },
        { FLAGS_24BPP, { 240, 164, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_B24.TGA",{ 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },
        { FLAGS_ZALPHA, { 240, 164, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_B32.TGA",{ 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },
        { FLAGS_ZALPHA, { 240, 164, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_T16.TGA",{ 0x57,0x9f,0x98,0xdd,0x7d,0x65,0x26,0x2f,0xf1,0x68,0x98,0x84,0x25,0x65,0x62,0x6b } },
        { FLAGS_24BPP, { 240, 164, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_T24.TGA",{ 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },
        { FLAGS_ZALPHA, { 240, 164, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"XING_T32.TGA",{ 0x8f,0x35,0x7c,0xc6,0x45,0x0b,0x0c,0x72,0x30,0x7e,0x93,0x5d,0x82,0x1f,0xde,0xe6 } },

        // D3DX test files
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_16.tga",{ 0xe3,0x32,0x13,0xd0,0xd2,0xd1,0xfe,0xfb,0x43,0xb8,0x7e,0xe0,0x39,0x50,0x08,0x88 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_16_rle.tga",{ 0xe3,0x32,0x13,0xd0,0xd2,0xd1,0xfe,0xfb,0x43,0xb8,0x7e,0xe0,0x39,0x50,0x08,0x88 } },
        { FLAGS_24BPP, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_24.tga",{ 0x55,0xb2,0xc2,0x63,0xe3,0x42,0x6c,0x5c,0x66,0xac,0xe0,0xf4,0x7f,0xcb,0x46,0xd1 } },
        { FLAGS_24BPP, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_24_rle.tga",{ 0x55,0xb2,0xc2,0x63,0xe3,0x42,0x6c,0x5c,0x66,0xac,0xe0,0xf4,0x7f,0xcb,0x46,0xd1 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_32.tga",{ 0x55,0xb2,0xc2,0x63,0xe3,0x42,0x6c,0x5c,0x66,0xac,0xe0,0xf4,0x7f,0xcb,0x46,0xd1 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_32_rle.tga",{ 0x55,0xb2,0xc2,0x63,0xe3,0x42,0x6c,0x5c,0x66,0xac,0xe0,0xf4,0x7f,0xcb,0x46,0xd1 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a8r8g8b8.tga",{ 0x64,0x77,0x03,0x4c,0x8d,0x27,0x07,0xb9,0xf7,0x86,0x44,0xdc,0x70,0x5d,0x0c,0x08 } },
        { FLAGS_NONE, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grad4d_a8r8g8b8_rle.tga",{ 0x64,0x77,0x03,0x4c,0x8d,0x27,0x07,0xb9,0xf7,0x86,0x44,0xdc,0x70,0x5d,0x0c,0x08 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust00.tga",{ 0x3d,0x47,0xf8,0x51,0x7a,0x94,0x55,0xe3,0x30,0x0c,0xf8,0xfc,0xb2,0x38,0x36,0xd3 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust01.tga",{ 0x50,0xc1,0x64,0x8b,0xbb,0xe6,0x47,0xc5,0xe7,0x3e,0xa5,0xb8,0xb8,0xbb,0xf0,0x5a } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust02.tga",{ 0xab,0x29,0xd9,0x78,0x96,0x0a,0xc1,0x87,0x4e,0x73,0xd8,0xe0,0x79,0xcc,0x3f,0xa4 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust03.tga",{ 0xd9,0x45,0xf8,0xd3,0x66,0x2c,0xd2,0xdc,0x4d,0x35,0xd1,0x9a,0x65,0xd2,0xa3,0xa1 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust04.tga",{ 0xbc,0x47,0x4d,0xf3,0xa9,0x09,0x6a,0x89,0x90,0x44,0x12,0xb3,0x1b,0x30,0x41,0x79 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust05.tga",{ 0xcf,0xf2,0xe7,0xf6,0x42,0x88,0x43,0x30,0x6d,0x7c,0x18,0x8c,0x6b,0x91,0x91,0xcf } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust06.tga",{ 0x54,0xbd,0x1d,0x60,0x25,0xfc,0x52,0x9d,0x50,0xd8,0x5d,0xd1,0x30,0x02,0xb2,0x5b } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust07.tga",{ 0xb5,0xbe,0x91,0xdb,0x4b,0xb3,0x00,0x47,0x5f,0x01,0x69,0xd4,0xb2,0x85,0xa4,0x2a } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust08.tga",{ 0x26,0x8d,0x40,0xdf,0xd0,0xb5,0x96,0x9d,0x5f,0x53,0x0f,0x9c,0x43,0x60,0x80,0x09 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust09.tga",{ 0x26,0xe7,0x57,0x56,0x92,0xe7,0xbe,0xbb,0xf0,0xaa,0xd7,0xa7,0x62,0x63,0x18,0x92 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust10.tga",{ 0xfa,0xaf,0x7e,0xb3,0x5d,0xfb,0x0e,0x54,0xad,0x0f,0x83,0xbd,0x99,0xb5,0xb6,0x4d } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust11.tga",{ 0xb6,0xcd,0x44,0x2f,0x52,0xf5,0x3b,0xf2,0xf2,0x92,0x7a,0x17,0x59,0x29,0x42,0x10 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust12.tga",{ 0x2e,0xad,0x6e,0xfc,0x7a,0xfe,0x85,0x93,0x24,0xa6,0x69,0xd4,0x58,0xcc,0xdd,0x6b } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust13.tga",{ 0x7b,0x12,0xae,0xc0,0x96,0xc5,0xc8,0xf5,0x75,0x98,0xb1,0xd0,0x33,0x4a,0xad,0x53 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust14.tga",{ 0x0d,0x17,0x06,0x88,0x72,0x01,0x43,0x03,0x16,0x0c,0xce,0x97,0xaf,0xdb,0xce,0x28 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust15.tga",{ 0xa3,0x8c,0x7a,0xe8,0xcb,0x24,0x35,0xba,0x0d,0xfa,0x61,0x66,0x0e,0xce,0x62,0xe7 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust16.tga",{ 0xaf,0x66,0x12,0x7b,0x18,0xa6,0xf3,0x90,0xe8,0x07,0xa8,0xbf,0xaf,0x73,0xd2,0xa8 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust17.tga",{ 0x94,0x04,0xbc,0x5a,0x3a,0x5e,0x14,0x5f,0x23,0x67,0xda,0x09,0x2c,0xb5,0x1a,0x1e } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust18.tga",{ 0xb7,0x14,0xaf,0xc3,0xfe,0xc4,0x1a,0xbb,0xd3,0x71,0x12,0x56,0xd5,0x02,0xcb,0xc1 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust19.tga",{ 0x27,0xb7,0xb9,0x3d,0x8a,0xb5,0x84,0x3a,0x49,0xaa,0x94,0x6b,0x72,0x9e,0x20,0x14 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust20.tga",{ 0xf5,0xf8,0xb9,0x63,0x1b,0xa0,0x3c,0x0c,0xb5,0x65,0x27,0x32,0xfd,0x8b,0xb1,0x62 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust21.tga",{ 0x6e,0x4b,0x85,0xa4,0xa0,0x06,0xbd,0xff,0x2e,0xa1,0xe6,0x90,0x2a,0xc2,0xb6,0xfd } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust22.tga",{ 0xf5,0x2b,0xa1,0xda,0x96,0xfb,0x63,0xb3,0xd3,0x65,0x60,0x77,0x4d,0xea,0xf4,0x0c } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust23.tga",{ 0x18,0x35,0xef,0xd8,0x7d,0x1a,0x73,0x9d,0x5c,0x7a,0x1b,0xd1,0x47,0x77,0x3a,0x00 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust24.tga",{ 0xaf,0xab,0x25,0xd6,0xc1,0x2a,0x7b,0x75,0xc0,0x3f,0xa6,0xf8,0x6d,0xf8,0x53,0xc4 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust25.tga",{ 0xe6,0xed,0xe3,0xee,0xc7,0xb5,0x2f,0xbd,0xc9,0x51,0xd8,0xc4,0xc0,0x23,0x79,0x68 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust26.tga",{ 0x1e,0x72,0x42,0xaa,0xff,0x0c,0xcf,0x3b,0xa5,0x7e,0x1e,0x1f,0x83,0x9a,0xc5,0xf9 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust27.tga",{ 0xb7,0xbe,0x4c,0xef,0x0b,0x07,0xa8,0x57,0x52,0x80,0x4a,0xcf,0x73,0xaf,0x7b,0x05 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust28.tga",{ 0x8d,0x22,0x29,0xa9,0xba,0x72,0xc3,0xf8,0xf2,0x27,0xf6,0xa6,0x3e,0xd4,0x9d,0x7d } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust29.tga",{ 0xb6,0x87,0xfa,0x38,0x81,0x60,0x9a,0x61,0x73,0xb1,0xcd,0xf7,0xa4,0x8d,0x49,0x2b } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust30.tga",{ 0x86,0xbe,0xd5,0x6a,0xba,0x8c,0x86,0x9b,0x74,0xc5,0x69,0xb3,0xdc,0x4f,0x4e,0x33 } },
        { FLAGS_RGB_BGR_SAME, { 64, 64, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"caust31.tga",{ 0xcc,0x9b,0x80,0xc3,0x9d,0x2b,0x6c,0x4d,0x60,0xf9,0x3f,0xed,0x0e,0xb8,0x97,0x79 } },
        { FLAGS_NONE, { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"emboss1.tga",{ 0x97,0x17,0x7b,0xb8,0x16,0xd4,0xe2,0x72,0x9d,0xbf,0x42,0x2f,0x6d,0x57,0x1e,0xce } },
        { FLAGS_NONE, { 206, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree01s.tga",{ 0xae,0xd4,0xc1,0x3b,0x56,0xfc,0xed,0x77,0x84,0xb9,0x7f,0xa7,0x7b,0x29,0x78,0x0e } },
        { FLAGS_NONE, { 302, 266, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02s.tga",{ 0x7f,0x31,0x75,0x71,0x9d,0x3d,0x4f,0xc6,0xc6,0xf6,0x58,0xe1,0xe6,0xdf,0x2b,0x80 } },
        { FLAGS_NONE, { 228, 337, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree35s.tga",{ 0x28,0x50,0x9e,0x18,0xa3,0x9b,0x41,0x0f,0x93,0x2c,0x40,0x14,0x6b,0x79,0x9b,0x8a } },

        // Legacy DirectX SDK files
        { FLAGS_24BPP, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"checker.tga",{ 0x28,0xea,0x8e,0xe6,0x87,0x9b,0x02,0xfe,0xe1,0xb7,0x69,0x54,0xd0,0xee,0xc0,0xbe } },
        { FLAGS_RGB_BGR_SAME, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"triangle.tga",{ 0x87,0xbe,0xee,0x51,0x66,0x96,0xc8,0x46,0x90,0x89,0x25,0xff,0x0c,0xb7,0x84,0xb3 } },
        { FLAGS_24BPP, { 1024, 1024, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"car2_diff.tga",{ 0xf0,0xb7,0x0b,0x89,0xa3,0x2f,0x0a,0x0d,0x16,0x71,0xc3,0xb2,0x75,0xc4,0x25,0x52 } },
        { FLAGS_NONE, { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bump_NM_height.tga",{ 0x63,0x93,0x18,0x17,0x9c,0xb0,0xa1,0x38,0xe4,0x8d,0x31,0xca,0x84,0x52,0xd8,0x6a } },
        { FLAGS_NONE, { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dent_NM_height.tga",{ 0x74,0xd4,0x32,0x31,0x14,0x3e,0x3f,0x12,0x02,0x1a,0xcc,0xb7,0xac,0xf5,0xab,0x9b } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"four_NM_height.tga",{ 0x6e,0x5b,0x27,0x84,0x0b,0xff,0xf7,0x44,0x5c,0xbb,0x6b,0x2e,0x78,0xb1,0xff,0x9f } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rocks_NM_height.tga",{ 0x57,0x85,0x66,0xab,0x31,0x35,0x3c,0xc1,0xec,0xec,0x2b,0x1b,0x8e,0x1a,0x19,0x0d } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saint_NM_height.tga",{ 0xd3,0x8f,0x64,0x26,0xdc,0x9f,0xff,0x26,0x1f,0xff,0x7e,0xe5,0x98,0xf5,0xf6,0xe9 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"stones_NM_height.tga",{ 0xa7,0x81,0x6f,0x51,0x19,0x5c,0xd9,0x2d,0x41,0xf4,0x22,0xc8,0xe4,0x65,0xf4,0xf4 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wall_NM_height.tga",{ 0x97,0xb3,0xa8,0x79,0xe1,0xbb,0xa1,0xb0,0x7b,0x22,0x16,0x0f,0x2e,0xf8,0xfe,0x50 } },
        { FLAGS_24BPP, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"concrete.tga",{ 0x8c,0x4a,0x1e,0xe7,0x0f,0x6c,0x09,0xa5,0x93,0x56,0x59,0xc2,0x78,0x81,0x49,0xde } },
        { FLAGS_24BPP, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"concreteWithBar.tga",{ 0xdd,0x94,0xda,0x77,0x16,0xf3,0xe0,0x72,0x25,0x28,0xbb,0xdb,0x16,0x9b,0x39,0x17 } },
        { FLAGS_24BPP, { 256, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lightPnlHoriz.tga",{ 0x20,0xd4,0x94,0x75,0x61,0x65,0xb4,0xba,0x9e,0x02,0x23,0x48,0x3c,0x5d,0x83,0x2c } },
        { FLAGS_24BPP, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"satDishPnl.tga",{ 0xbd,0x7b,0x32,0x52,0x3c,0x23,0x75,0xf2,0x96,0xc0,0x7e,0xeb,0x17,0x98,0x6e,0xde } },
        { FLAGS_24BPP, { 128, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wallPnl.tga",{ 0x06,0x4c,0x38,0x33,0x68,0x58,0xc3,0xa1,0x02,0x3d,0xa3,0xd5,0xe0,0x9d,0x7d,0x32 } },
        { FLAGS_NONE, { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"triangle16.tga",{ 0x27,0xb9,0x54,0x28,0x9a,0xab,0x64,0xca,0xb6,0xb2,0x74,0x23,0x31,0x9c,0xed,0xf6 } },
        { FLAGS_RGB_BGR_SAME, { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"triangle32.tga",{ 0x49,0xb2,0x8f,0x9b,0x46,0x1e,0xb7,0xe1,0xa4,0x95,0x31,0xb6,0xa3,0x32,0xb9,0xed } },

        // XNA Game Studio Sample files
        { FLAGS_NONE, { 256, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"asteroid1.tga",{ 0x5b,0x49,0x06,0x64,0x0a,0xd9,0x50,0x6e,0xcd,0x8c,0x9f,0x3d,0x6f,0xff,0x04,0xd1 } },
        { FLAGS_24BPP, { 512, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"asteroid1N.tga",{ 0xe2,0xe9,0x11,0xdc,0x24,0xee,0x44,0xd9,0x7f,0x1c,0xc3,0x4c,0xaa,0xf3,0x2a,0x48 } },
        { FLAGS_NONE, { 256, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"asteroid2.tga",{ 0x8f,0x5d,0x9b,0x84,0xa4,0x31,0xea,0x78,0x07,0x14,0x8e,0xed,0xf2,0x53,0x75,0xc3 } },
        { FLAGS_24BPP, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bfg_proj.tga",{ 0xea,0xeb,0x97,0x66,0xd6,0x89,0xab,0xb2,0x46,0xd6,0x3c,0xdf,0x09,0xf3,0xfb,0xc5 } },
        { FLAGS_RGB_BGR_SAME, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Black.tga",{ 0x9d,0x7a,0x4f,0xbb,0x84,0xdf,0x46,0x86,0x96,0x02,0xed,0x94,0xea,0x21,0xd3,0x22 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"cat.tga",{ 0x68,0x6e,0x25,0x75,0x4b,0xad,0xec,0xb2,0xb3,0x3e,0x2c,0x23,0x58,0xbd,0x5f,0xfc } },
        { FLAGS_RGB_BGR_SAME, { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"circle.tga",{ 0xbf,0x10,0x7e,0xe6,0x92,0xb5,0x7b,0xed,0x39,0x30,0x21,0xdf,0xcf,0xce,0x67,0xda } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"grass.tga",{ 0xe7,0x8d,0xbc,0xb9,0x75,0x29,0xf8,0x17,0xe7,0x56,0x5d,0x92,0xdd,0x49,0x10,0xd1 } },
        { FLAGS_NONE, { 320, 176, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"in-game_score_overlay.tga",{ 0x39,0x19,0xe2,0xfd,0xd3,0xb1,0x97,0xa1,0xbd,0x28,0x86,0x64,0x01,0x56,0xa9,0x10 } },
        { FLAGS_NONE, { 740, 500, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"info_screen.tga",{ 0x2e,0xd8,0xcc,0x0e,0x4a,0xa7,0x54,0x52,0xb6,0x70,0x2c,0x28,0xa9,0x9f,0xde,0xa6 } },
        { FLAGS_24BPP, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"mgun_proj.tga",{ 0xb6,0xbb,0xf8,0xbc,0xbb,0x3c,0xca,0xe6,0x53,0x1d,0xfe,0x50,0x9b,0x83,0xc7,0xbd } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p1_back.tga",{ 0x9f,0xb4,0x1e,0x3d,0x05,0xf7,0x47,0x43,0xcb,0x78,0x3e,0x58,0x11,0xaf,0x4c,0x67 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p1_bfg.tga",{ 0xf4,0x4d,0xfe,0x42,0x2f,0xe6,0x9a,0x13,0x54,0x52,0xfc,0xa0,0x29,0xac,0xe3,0x50 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p1_dual.tga",{ 0xe8,0x30,0x57,0xdc,0x16,0x51,0x72,0x36,0x48,0x05,0xe7,0x29,0x37,0x36,0x30,0x71 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p1_rocket.tga",{ 0xd2,0xc7,0xaa,0x38,0xcd,0xfa,0xac,0x46,0xa2,0x13,0x49,0x64,0x17,0xa4,0xb2,0x1a } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p2_back.tga",{ 0x72,0xde,0x35,0x95,0xcc,0x26,0x6f,0x06,0x8d,0xfd,0x69,0x2f,0xbb,0x8b,0x6b,0x74 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p2_bfg.tga",{ 0xac,0x63,0x7f,0x5f,0xdf,0xe2,0x98,0x26,0x91,0x60,0xc7,0x7a,0x36,0x5e,0xef,0xb6 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p2_dual.tga",{ 0xf8,0xe6,0x62,0x58,0x92,0x97,0xbf,0x51,0xee,0xe3,0xe9,0x74,0xfd,0xad,0xd6,0xb8 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"p2_rocket.tga",{ 0x83,0xc0,0x90,0xb8,0x9a,0xc0,0x23,0x81,0x89,0x23,0x43,0x30,0x12,0x47,0xb9,0xb4 } },
        { FLAGS_24BPP, { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pawball.tga",{ 0xcc,0xbb,0x8d,0xe7,0x69,0xc0,0xa3,0xdf,0x5f,0x74,0x4d,0xa2,0xe8,0xe2,0xa3,0xf4 } },
        { FLAGS_24BPP, { 32, 32, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pea_proj.tga",{ 0x63,0x43,0x0d,0xad,0x33,0x5d,0xf3,0x66,0x1c,0x77,0x04,0x8d,0x9f,0x02,0x5c,0xe7 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pencil_p1_diff_v1.tga",{ 0xf1,0x97,0x21,0x76,0x90,0x58,0xa6,0x58,0x55,0xb4,0x03,0x37,0x8a,0x6a,0x32,0xe2 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pencil_p1_diff_v2.tga",{ 0x94,0x0a,0xf2,0x21,0xda,0x36,0xe9,0x80,0xd2,0x66,0xa5,0x1d,0xa0,0x61,0x67,0x2e } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pencil_p1_diff_v3.tga",{ 0x1f,0x64,0x65,0x7a,0xea,0x4d,0xd5,0x70,0x48,0x13,0xf0,0xd5,0x5e,0x23,0xd1,0xb5 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pencil_p2_diff_v1.tga",{ 0x50,0x8c,0xe1,0x3e,0x2a,0x79,0xd2,0x36,0x1e,0x74,0x2a,0x73,0xea,0x76,0xfa,0xb4 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pencil_p2_diff_v2.tga",{ 0xd4,0x55,0x4d,0x11,0x11,0xc5,0x74,0x2e,0x75,0x0f,0x2d,0x79,0xdf,0x4a,0xb2,0xb3 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"pencil_p2_diff_v3.tga",{ 0xca,0xdc,0xdc,0x9d,0x7c,0xaf,0x2b,0x72,0x28,0x66,0xd1,0xd6,0xdf,0xb4,0x21,0x81 } },
        { FLAGS_24BPP, { 128, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"rocket_proj.tga",{ 0x07,0x2a,0x32,0xad,0x9e,0x9f,0xb5,0xd7,0xd9,0x6f,0xf1,0x03,0xe5,0x2b,0x26,0x5c } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saucer_p1_diff_v1.tga",{ 0x9f,0x2b,0x08,0x36,0x37,0xc4,0x57,0xd4,0x81,0xa2,0xc9,0xeb,0x20,0xfc,0x50,0xe7 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saucer_p1_diff_v2.tga",{ 0xfe,0xba,0x15,0xc0,0x02,0x43,0x08,0xa8,0x02,0x2a,0x8d,0x65,0x33,0xe9,0xb9,0x20 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saucer_p1_diff_v3.tga",{ 0x42,0xea,0xa2,0x6c,0xc0,0xa9,0x1b,0x70,0xf6,0x32,0xbd,0x1e,0x3b,0x75,0x3f,0x20 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saucer_p2_diff_v1.tga",{ 0x9b,0x1c,0xf4,0x9d,0x1d,0x86,0x67,0xee,0x1e,0x75,0xb8,0xff,0xf7,0xbf,0x87,0xc0 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saucer_p2_diff_v2.tga",{ 0x1d,0x43,0x95,0x5e,0xdf,0x49,0x61,0xca,0xc3,0x14,0xd1,0xe6,0x34,0xdd,0x20,0x4b } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"saucer_p2_diff_v3.tga",{ 0x2d,0x0e,0xda,0x73,0x95,0xe5,0x25,0x35,0x3c,0xa4,0x89,0xc7,0x03,0xbe,0xff,0x6c } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"ShipDiffuse.tga",{ 0x0c,0x1a,0x9a,0x84,0x15,0x9f,0xae,0xc9,0xf9,0x49,0x48,0x7f,0x70,0x1a,0xf1,0x63 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"suntest1.tga",{ 0x76,0x43,0x4e,0xdc,0x29,0x38,0x10,0xf7,0xab,0x6d,0x15,0xb0,0xdd,0xed,0xc2,0x69 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"suntest2.tga",{ 0x08,0x0c,0xa7,0xbd,0x32,0x8a,0x5b,0xf9,0x16,0x62,0x45,0xeb,0xe9,0x7b,0x0e,0xd4 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"suntest3.tga",{ 0x92,0x4d,0x57,0x38,0x0a,0x32,0x31,0x01,0x0f,0x86,0x18,0x50,0x1f,0xc9,0x25,0xb6 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"suntest4.tga",{ 0xbe,0x2c,0xb3,0xf1,0x29,0x1c,0x2f,0x54,0xfa,0x89,0xf8,0x14,0x9e,0x98,0x1d,0x88 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"suntest5.tga",{ 0xe4,0x20,0x44,0x60,0x45,0x4a,0xda,0x12,0xb8,0x45,0xe6,0x34,0x53,0xbb,0xae,0x48 } },
        { FLAGS_NONE, { 1920, 16, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"thrust_stripSmall.tga",{ 0xc7,0x40,0x18,0x66,0x9f,0xd5,0xab,0x36,0xa1,0xc9,0xb4,0x63,0xe0,0xf9,0x33,0x1e } },
        { FLAGS_24BPP, { 330, 270, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Title_button_overlay.tga",{ 0x73,0x0a,0x47,0xe3,0x22,0xe7,0xbd,0xe4,0x32,0x97,0xb2,0xdf,0x1a,0x61,0x52,0x03 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree.tga",{ 0x08,0xdd,0xc9,0x8d,0xc0,0xeb,0x97,0xb6,0xf3,0x1f,0xfb,0x7e,0x68,0xb4,0xa2,0x94 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wedge_p1_diff_v1.tga",{ 0x43,0xb2,0xf2,0xa7,0x04,0xe9,0x6b,0x25,0x0d,0x62,0x08,0x26,0x56,0x6c,0x02,0x1f } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wedge_p1_diff_v2.tga",{ 0xc2,0xaf,0xa4,0x63,0xdb,0xf8,0xaf,0x94,0xf7,0x99,0xf5,0xcc,0x28,0xd9,0x5b,0x2b } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wedge_p1_diff_v3.tga",{ 0x0c,0x1a,0x9a,0x84,0x15,0x9f,0xae,0xc9,0xf9,0x49,0x48,0x7f,0x70,0x1a,0xf1,0x63 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wedge_p2_diff_v1.tga",{ 0x04,0x35,0xf2,0xfd,0x35,0xba,0x4d,0x56,0xcc,0x69,0xe1,0x47,0x02,0x0a,0x53,0x43 } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wedge_p2_diff_v2.tga",{ 0x0b,0x9d,0x04,0x8a,0x7b,0x40,0xd9,0x55,0x84,0xa7,0xf8,0x32,0xa9,0x55,0xec,0x2e } },
        { FLAGS_NONE, { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"wedge_p2_diff_v3.tga",{ 0x5d,0xee,0x87,0x25,0xcf,0x4b,0xb0,0x94,0x15,0x86,0xb4,0x97,0x07,0xc4,0x64,0xe7 } },
        { FLAGS_NONE, { 83, 79, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerBack.tga",{ 0xc0,0x1d,0x96,0xee,0x23,0xd5,0xcb,0xb7,0x68,0xab,0x0b,0xa4,0xe2,0xe2,0x2e,0x29 } },
        { FLAGS_NONE, { 80, 80, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerButtonA.tga",{ 0x02,0x0e,0xb2,0x66,0xa5,0x09,0x47,0x2b,0x64,0x4b,0x6c,0x09,0x02,0xb1,0xd1,0xc7 } },
        { FLAGS_NONE, { 80, 80, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerButtonB.tga",{ 0x6e,0x70,0xfc,0xec,0x89,0x9f,0x1e,0xbd,0xa4,0xcb,0x9f,0x40,0xdd,0x49,0x4c,0xc6 } },
        { FLAGS_NONE, { 138, 138, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerButtonGuide.tga",{ 0xbc,0x27,0x20,0x3b,0x55,0x63,0xc5,0x2b,0xfd,0xf9,0x6f,0xb6,0x81,0xec,0x59,0x53 } },
        { FLAGS_NONE, { 80, 80, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerButtonX.tga",{ 0x58,0xec,0x1c,0x71,0xde,0x7c,0x23,0xe0,0x0a,0x24,0xe5,0xc1,0x41,0x82,0xc6,0xba } },
        { FLAGS_NONE, { 80, 80, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerButtonY.tga",{ 0xc5,0x32,0x17,0xa0,0xda,0x7a,0x9d,0x4a,0xd9,0x75,0x89,0xce,0x07,0xee,0x16,0x8f } },
        { FLAGS_NONE, { 186, 186, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerDPad.tga",{ 0xb5,0xe4,0xea,0x5c,0x4d,0xe6,0x0d,0xc0,0xb2,0xdc,0xc6,0x27,0x7f,0x2f,0xf3,0xb6 } },
        { FLAGS_NONE, { 218, 92, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerLeftShoulder.tga",{ 0xd1,0xb0,0xb3,0xd7,0x3a,0x49,0x01,0xfc,0x0a,0xca,0xe9,0x5b,0x81,0xd2,0x8d,0xa8 } },
        { FLAGS_NONE, { 153, 158, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerLeftThumbstick.tga",{ 0xb7,0x65,0x5b,0x4d,0xbc,0xb6,0x8f,0x80,0x7b,0x79,0x80,0x50,0x8e,0x3f,0xc0,0x46 } },
        { FLAGS_NONE, { 88, 169, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerLeftTrigger.tga",{ 0x5f,0xac,0xa8,0x18,0xd9,0x55,0x93,0xc2,0xec,0xa8,0x10,0xb6,0x38,0xf4,0xd4,0x27 } },
        { FLAGS_NONE, { 218, 92, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerRightShoulder.tga",{ 0xf4,0x5f,0xbf,0x4f,0x36,0x79,0xbd,0x5f,0x6a,0x03,0xae,0xe0,0x30,0xf7,0x52,0x2c } },
        { FLAGS_NONE, { 153, 160, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerRightThumbstick.tga",{ 0x25,0xaa,0x09,0xd4,0xf2,0x32,0x4f,0x27,0xb1,0xea,0xf1,0x60,0x43,0x92,0xf1,0x3f } },
        { FLAGS_NONE, { 88, 169, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerRightTrigger.tga",{ 0x07,0xdb,0xa4,0x7a,0xb5,0xe6,0xb4,0x17,0xcc,0x3e,0x1c,0x79,0x5e,0xed,0xde,0x0e } },
        { FLAGS_NONE, { 1729, 188, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerSpriteFont.tga",{ 0xf3,0xb2,0x59,0x2b,0x5e,0x42,0xa0,0x3d,0xaf,0xdd,0xd4,0xbc,0x67,0x4c,0x65,0xfe } },
        { FLAGS_NONE, { 86, 78, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"xboxControllerStart.tga",{ 0x4d,0x0a,0x57,0x9d,0xc4,0x0f,0x47,0x8c,0xda,0x1a,0x96,0xdf,0x12,0x03,0xf3,0x30 } },
    };

    //-------------------------------------------------------------------------------------

    struct SaveMedia
    {
        DWORD options;
        DXGI_FORMAT src_format;
        DXGI_FORMAT sav_format;
        TEX_ALPHA_MODE save_alpha;
        const wchar_t *source;
    };

    const SaveMedia g_SaveMedia[] =
    {
        // flags | source-dxgi-format save-dxgi-format | source-filename
        { ALTMD5(2), DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_ALPHA_MODE_OPAQUE, MEDIA_PATH L"test8888.dds" },
        { ALTMD5(3), DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_ALPHA_MODE_OPAQUE, MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { FLAGS_NONE, DXGI_FORMAT_B5G5R5A1_UNORM, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_ALPHA_MODE_OPAQUE, MEDIA_PATH L"test555.dds" },
        { FLAGS_NONE, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, TEX_ALPHA_MODE_UNKNOWN, MEDIA_PATH L"windowslogo_L8.dds" },
        { FLAGS_NONE, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_R8_UNORM, TEX_ALPHA_MODE_UNKNOWN, MEDIA_PATH L"alphaedge.dds" },
        { ALTMD5(4), DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_ALPHA_MODE_UNKNOWN, MEDIA_PATH L"tree02S.dds" },
    };

    struct AltMD5
    {
        uint8_t md5[16];
    };

    const AltMD5 g_AltMD5[] =
    {
        { 0xa9,0xc6,0xf1,0xfa,0xe0,0x92,0x4f,0x44,0x31,0x24,0x3e,0xa2,0xc2,0xec,0x3a,0xa1 }, // ALTMD5(1)
        { 0x35,0x80,0x55,0xf9,0xd6,0x91,0x58,0x0a,0x8c,0xf3,0x37,0x29,0x57,0xc1,0x85,0xf0 }, // ALTMD5(2)
        { 0x8d,0x2c,0xc5,0xd3,0xb1,0x44,0x64,0x39,0x30,0xe0,0xca,0x36,0x1c,0xeb,0x17,0xaa }, // ALTMD5(3)
        { 0x8c,0xeb,0x7a,0x9a,0x80,0xc0,0xe7,0xf3,0xfe,0x75,0x2f,0xdf,0xf5,0x72,0x3e,0x46 }, // ALTMD5(4)
    };

    const TEX_ALPHA_MODE g_AlphaModes[] =
    {
        TEX_ALPHA_MODE_STRAIGHT,
        TEX_ALPHA_MODE_PREMULTIPLIED,
        TEX_ALPHA_MODE_OPAQUE,
        TEX_ALPHA_MODE_CUSTOM,
    };
}

//-------------------------------------------------------------------------------------

extern HRESULT LoadBlobFromFile( _In_z_ const wchar_t* szFile, Blob& blob );
extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest );
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

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
        HRESULT hr = GetMetadataFromTGAFile( szPath, metadata );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else if (metadata.width != check->width
                 || metadata.height != check->height
                 || metadata.depth != check->depth
                 || metadata.arraySize != check->arraySize
                 || metadata.mipLevels != check->mipLevels
                 || metadata.miscFlags != check->miscFlags /* ignore miscFlags2 */
                 || metadata.format != check->format
                 || metadata.dimension != check->dimension)
        {
            success = false;
            printe( "Metadata error in:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;

            TexMetadata metadata2;
            hr = GetMetadataFromTGAFile(szPath, TGA_FLAGS_BGR, metadata2);
            if (FAILED(hr))
            {
                success = pass = false;
                printe("Failed getting BGR data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else if (metadata2.width != check->width
                || metadata2.height != check->height
                || metadata2.depth != check->depth
                || metadata2.arraySize != check->arraySize
                || metadata2.mipLevels != check->mipLevels
                || metadata2.miscFlags != check->miscFlags /* ignore miscFlags2, ignore format */
                || metadata2.dimension != check->dimension)
            {
                success = pass = false;
                printe("Metadata error BGR in:\n%ls\n", szPath);
                printmeta(&metadata2);
                printmetachk(check);
            }
            else if (g_TestMedia[index].options & FLAGS_24BPP)
            {
                if (metadata2.format != DXGI_FORMAT_B8G8R8X8_UNORM)
                {
                    success = pass = false;
                    printe("Metadata error expected BGRX in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                }
            }
            else if (check->format == DXGI_FORMAT_R8G8B8A8_UNORM)
            {
                if (metadata2.format != DXGI_FORMAT_B8G8R8A8_UNORM)
                {
                    success = pass = false;
                    printe("Metadata error expected BGRA in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                }
            }
            else if (metadata2.format != check->format)
            {
                success = pass = false;
                printe("Metadata error BGR format in:\n%ls\n", szPath);
                printmeta(&metadata2);
                printmetachk(check);
            }

            if (pass)
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

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
            bool pass = true;

            TexMetadata metadata;
            ScratchImage image;
            hr = LoadFromTGAMemory(blob.GetBufferPointer(), blob.GetBufferSize(), &metadata, image);

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if (FAILED(hr))
            {
                success = pass = false;
                printe("Failed loading tga from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else if (memcmp(&metadata, check, sizeof(TexMetadata)) != 0)
            {
                success = pass = false;
                printe("Metadata error in:\n%ls\n", szPath);
                printmeta(&metadata);
                printmetachk(check);
            }
            else
            {
                uint8_t digest[16];
                hr = MD5Checksum(image, digest);
                if (FAILED(hr))
                {
                    success = pass = false;
                    printe("Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else if (memcmp(digest, g_TestMedia[index].md5, 16) != 0)
                {
                    success = pass = false;
                    printe("Failed comparing MD5 checksum:\n%ls\n", szPath);
                    printdigest("computed", digest);
                    printdigest("expected", g_TestMedia[index].md5);
                }

                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );
            }

            image.Release();

            // BGR
            if (check->format == DXGI_FORMAT_R8G8B8A8_UNORM)
            {
                TexMetadata metadata2;
                hr = LoadFromTGAMemory(blob.GetBufferPointer(), blob.GetBufferSize(), TGA_FLAGS_BGR, &metadata2, image);
                if (FAILED(hr))
                {
                    success = pass = false;
                    printe("Failed loading tga BGR from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else if (metadata2.width != check->width
                    || metadata2.height != check->height
                    || metadata2.depth != check->depth
                    || metadata2.arraySize != check->arraySize
                    || metadata2.mipLevels != check->mipLevels
                    || metadata2.miscFlags != check->miscFlags /* ignore miscFlags2, ignore format */
                    || metadata2.dimension != check->dimension)
                {
                    success = pass = false;
                    printe("Metadata error BGR in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                    printmetachk(check);
                }
                else if ((g_TestMedia[index].options & FLAGS_24BPP) && (metadata2.format != DXGI_FORMAT_B8G8R8X8_UNORM))
                {
                    success = pass = false;
                    printe("Metadata error expected BGRX in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                }
                else if (!(g_TestMedia[index].options & FLAGS_24BPP) && (metadata2.format != DXGI_FORMAT_B8G8R8A8_UNORM))
                {
                    success = pass = false;
                    printe("Metadata error expected BGRA in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                }
                else
                {
                    uint8_t digest2[16];
                    hr = MD5Checksum(image, digest2);
                    if (FAILED(hr))
                    {
                        success = pass = false;
                        printe("Failed computing MD5 checksum of BGR image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else if (!(g_TestMedia[index].options & FLAGS_RGB_BGR_SAME) && memcmp(digest2, g_TestMedia[index].md5, 16) == 0)
                    {
                        success = pass = false;
                        printe("Expected different MD5 checksum from RGB:\n%ls\n", szPath);
                        printdigest("computed", digest2);
                    }
                    else if ((g_TestMedia[index].options & FLAGS_RGB_BGR_SAME) && memcmp(digest2, g_TestMedia[index].md5, 16) != 0)
                    {
                        success = pass = false;
                        printe("Failed comparing MD5 checksum:\n%ls\n", szPath);
                        printdigest("computed", digest2);
                        printdigest("expected", g_TestMedia[index].md5);
                    }

                    // TESTTEST- SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image);
                }
            }

            // ALLOW_ALL_ZERO_ALPHA
            if (g_TestMedia[index].options & FLAGS_ZALPHA)
            {
                TexMetadata metadata2;
                hr = LoadFromTGAMemory(blob.GetBufferPointer(), blob.GetBufferSize(), TGA_FLAGS_ALLOW_ALL_ZERO_ALPHA, &metadata2, image);
                if (FAILED(hr))
                {
                    success = pass = false;
                    printe("Failed loading tga ZALPHA from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else if (metadata.width != check->width
                    || metadata.height != check->height
                    || metadata.depth != check->depth
                    || metadata.arraySize != check->arraySize
                    || metadata.mipLevels != check->mipLevels
                    || metadata.miscFlags != check->miscFlags /* ignore miscFlags2 */
                    || metadata.format != check->format
                    || metadata.dimension != check->dimension)
                {
                    success = pass = false;
                    printe("Metadata error ZALPHA in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                    printmetachk(check);
                }
                else
                {
                    uint8_t digest2[16];
                    hr = MD5Checksum(image, digest2);
                    if (FAILED(hr))
                    {
                        success = pass = false;
                        printe("Failed computing MD5 checksum of ZALPHA image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else if (memcmp(digest2, g_TestMedia[index].md5, 16) == 0)
                    {
                        success = pass = false;
                        printe("Expected different MD5 checksum from ZALPHA:\n%ls\n", szPath);
                        printdigest("computed", digest2);
                    }

                    // TESTTEST- SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image);
                }
            }

            if (pass)
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

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
        HRESULT hr = LoadFromTGAFile( szPath, &metadata, image );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed loading tga (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
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

            uint8_t digest[316];
            hr = MD5Checksum( image, digest );
            if ( FAILED(hr) )
            {
                success = pass = false;
                printe( "Failed computing MD5 checksum of image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else if ( memcmp( digest, g_TestMedia[index].md5, 16 ) != 0 )
            {
                success = pass = false;
                printe( "Failed comparing MD5 checksum:\n%ls\n", szPath );
                printdigest( "computed", digest );
                printdigest( "expected", g_TestMedia[index].md5 );
            }

            // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );

            image.Release();

            // BGR
            if (check->format == DXGI_FORMAT_R8G8B8A8_UNORM)
            {
                TexMetadata metadata2;
                hr = LoadFromTGAFile(szPath, TGA_FLAGS_BGR, &metadata2, image);
                if (FAILED(hr))
                {
                    success = pass = false;
                    printe("Failed loading tga BGR (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else if (metadata2.width != check->width
                    || metadata2.height != check->height
                    || metadata2.depth != check->depth
                    || metadata2.arraySize != check->arraySize
                    || metadata2.mipLevels != check->mipLevels
                    || metadata2.miscFlags != check->miscFlags /* ignore miscFlags2, ignore format */
                    || metadata2.dimension != check->dimension)
                {
                    success = pass = false;
                    printe("Metadata error BGR in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                    printmetachk(check);
                }
                else if ((g_TestMedia[index].options & FLAGS_24BPP) && (metadata2.format != DXGI_FORMAT_B8G8R8X8_UNORM))
                {
                    success = pass = false;
                    printe("Metadata error expected BGRX in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                }
                else if (!(g_TestMedia[index].options & FLAGS_24BPP) && (metadata2.format != DXGI_FORMAT_B8G8R8A8_UNORM))
                {
                    success = pass = false;
                    printe("Metadata error expected BGRA in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                }
                else
                {
                    uint8_t digest2[316];
                    hr = MD5Checksum(image, digest2);
                    if (FAILED(hr))
                    {
                        success = pass = false;
                        printe("Failed computing MD5 checksum of BGR image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else if (!(g_TestMedia[index].options & FLAGS_RGB_BGR_SAME) && memcmp(digest2, g_TestMedia[index].md5, 16) == 0)
                    {
                        success = pass = false;
                        printe("Expected different MD5 checksum from RGB:\n%ls\n", szPath);
                        printdigest("computed", digest2);
                    }
                    else if ((g_TestMedia[index].options & FLAGS_RGB_BGR_SAME) && memcmp(digest2, g_TestMedia[index].md5, 16) != 0)
                    {
                        success = pass = false;
                        printe("Failed comparing MD5 checksum:\n%ls\n", szPath);
                        printdigest("computed", digest2);
                        printdigest("expected", g_TestMedia[index].md5);
                    }

                    // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image );
                }
            }

            // ALLOW_ALL_ZERO_ALPHA
            if (g_TestMedia[index].options & FLAGS_ZALPHA)
            {
                TexMetadata metadata2;
                hr = LoadFromTGAFile(szPath, TGA_FLAGS_ALLOW_ALL_ZERO_ALPHA, &metadata2, image);
                if (FAILED(hr))
                {
                    success = pass = false;
                    printe("Failed loading tga ZALPHA (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                }
                else if (metadata.width != check->width
                    || metadata.height != check->height
                    || metadata.depth != check->depth
                    || metadata.arraySize != check->arraySize
                    || metadata.mipLevels != check->mipLevels
                    || metadata.miscFlags != check->miscFlags /* ignore miscFlags2 */
                    || metadata.format != check->format
                    || metadata.dimension != check->dimension)
                {
                    success = pass = false;
                    printe("Metadata error ZALPHA in:\n%ls\n", szPath);
                    printmeta(&metadata2);
                    printmetachk(check);
                }
                else
                {
                    uint8_t digest2[16];
                    hr = MD5Checksum(image, digest2);
                    if (FAILED(hr))
                    {
                        success = pass = false;
                        printe("Failed computing MD5 checksum of ZALPHA image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                    }
                    else if (memcmp(digest2, g_TestMedia[index].md5, 16) == 0)
                    {
                        success = pass = false;
                        printe("Expected different MD5 checksum from ZALPHA:\n%ls\n", szPath);
                        printdigest("computed", digest2);
                    }

                    // TESTTEST- SaveScratchImage(L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image);
                }
            }

            if (pass)
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

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

                Blob blob;
                hr = SaveToTGAMemory( *image.GetImage(0,0,0), blob );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing tga to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;
                    hr = LoadFromTGAMemory( blob.GetBufferPointer(), blob.GetBufferSize(), &metadata2, image2 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written tga to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata2.mipLevels != 1
                              || metadata.dimension != metadata2.dimension
                              || g_SaveMedia[index].save_alpha != metadata2.GetAlphaMode()
                              || g_SaveMedia[index].sav_format != metadata2.format )
                    { // Formats can vary for readback, and miplevel is going to be 1 for TGA images
                        success = false;
                        pass = false;
                        printe( "Metadata error in tga memory readback:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else
                    {
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
                            pass = false;
                            printe( "Failed computing MD5 checksum of reloaded image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( memcmp( expected, digest2, 16 ) != 0 )
                        {
                            success = false;
                            pass = false;
                            printe( "MD5 checksum of reloaded data doesn't match original:\n%ls\n", szPath );
                        }
                    }
                }

                // TGA 2.0 tests
                if (g_SaveMedia[index].save_alpha != TEX_ALPHA_MODE_OPAQUE)
                {
                    for (size_t j = 0; j < _countof(g_AlphaModes); ++j)
                    {
                        TexMetadata alphamdata = metadata;
                        alphamdata.SetAlphaMode(g_AlphaModes[j]);

                        hr = SaveToTGAMemory(*image.GetImage(0, 0, 0), blob, &alphamdata);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed writing tga to memory [tga20] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                        }
                        else
                        {
                            TexMetadata metadata2;
                            ScratchImage image2;
                            hr = LoadFromTGAMemory(blob.GetBufferPointer(), blob.GetBufferSize(), &metadata2, image2);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed reading back written tga to memory [tga20] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                            }
                            else if (alphamdata.width != metadata2.width
                                || alphamdata.height != metadata2.height
                                || alphamdata.arraySize != metadata2.arraySize
                                || metadata2.mipLevels != 1
                                || alphamdata.dimension != metadata2.dimension
                                || metadata2.GetAlphaMode() != g_AlphaModes[j]
                                || g_SaveMedia[index].sav_format != metadata2.format)
                            { // Formats can vary for readback, and miplevel is going to be 1 for TGA images
                                success = false;
                                pass = false;
                                printe("Metadata error in tga memory readback [tga20]:\n%ls\n", szPath);
                                printmeta(&metadata2);
                                printmetachk(&alphamdata);
                            }
                            else
                            {
                                const uint8_t* expected = digest;
                                if (g_SaveMedia[index].options & FLAGS_ALTMD5_MASK)
                                {
                                    expected = g_AltMD5[((g_SaveMedia[index].options & 0xf0) >> 4) - 1].md5;
                                }

                                uint8_t digest2[16];
                                hr = MD5Checksum(image2, digest2);
                                if (FAILED(hr))
                                {
                                    success = false;
                                    pass = false;
                                    printe("Failed computing MD5 checksum of reloaded image  [tga20] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                                }
                                else if (memcmp(expected, digest2, 16) != 0)
                                {
                                    success = false;
                                    pass = false;
                                    printe("MD5 checksum of reloaded data doesn't match original [tga20]:\n%ls\n", szPath);
                                }
                            }
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
// SaveTGAToFile
bool Test05()
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

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"tga", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        wchar_t szDestPath[MAX_PATH] = {};
        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".tga" );

        wchar_t szDestPath2[MAX_PATH] = {};
        wchar_t tname[_MAX_FNAME] = {};
        wcscpy_s(tname, fname);
        wcscat_s(tname, L"_tga20");
        _wmakepath_s(szDestPath2, MAX_PATH, nullptr, tempDir, tname, L".tga");

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

                hr = SaveToTGAFile( *image.GetImage(0,0,0), szDestPath );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed writing tga to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                }
                else
                {
                    TexMetadata metadata2;
                    ScratchImage image2;
                    hr = LoadFromTGAFile( szDestPath, &metadata2, image2 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed reading back written tga to file (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath );
                    }
                    else if ( metadata.width != metadata2.width
                              || metadata.height != metadata2.height
                              || metadata.arraySize != metadata2.arraySize
                              || metadata2.mipLevels != 1
                              || metadata.dimension != metadata2.dimension
                              || g_SaveMedia[index].save_alpha != metadata2.GetAlphaMode()
                              || g_SaveMedia[index].sav_format != metadata2.format  )
                    {   // Formats can vary for readback, and miplevel is going to be 1 for TGA images
                        success = false;
                        pass = false;
                        printe( "Metadata error in tga readback:\n%ls\n", szDestPath );
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
                            pass = false;
                            success = false;
                            printe( "Failed computing MD5 checksum of reloaded image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( memcmp( expected, digest2, 16 ) != 0 )
                        {
                            pass = false;
                            success = false;
                            printe( "MD5 checksum of reloaded data doesn't match original:\n%ls\n", szPath );
                        }
                    }
                }

                // TGA 2.0 tests
                if (g_SaveMedia[index].save_alpha != TEX_ALPHA_MODE_OPAQUE)
                {
                    for (size_t j = 0; j < _countof(g_AlphaModes); ++j)
                    {
                        TexMetadata alphamdata = metadata;
                        alphamdata.SetAlphaMode(g_AlphaModes[j]);

                        hr = SaveToTGAFile(*image.GetImage(0, 0, 0), szDestPath2, &alphamdata);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed writing tga to file [tga20] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                        }
                        else
                        {
                            TexMetadata metadata2;
                            ScratchImage image2;
                            hr = LoadFromTGAFile(szDestPath2, &metadata2, image2);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed reading back written tga to file [tga20] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath2);
                            }
                            else if (alphamdata.width != metadata2.width
                                || alphamdata.height != metadata2.height
                                || alphamdata.arraySize != metadata2.arraySize
                                || metadata2.mipLevels != 1
                                || alphamdata.dimension != metadata2.dimension
                                || metadata2.GetAlphaMode() != g_AlphaModes[j]
                                || g_SaveMedia[index].sav_format != metadata2.format)
                            {   // Formats can vary for readback, and miplevel is going to be 1 for TGA images
                                success = false;
                                pass = false;
                                printe("Metadata error in tga readback [tga20]:\n%ls\n", szDestPath2);
                                printmeta(&metadata2);
                                printmetachk(&metadata);
                            }
                            else
                            {
                                const uint8_t* expected = digest;
                                if (g_SaveMedia[index].options & FLAGS_ALTMD5_MASK)
                                {
                                    expected = g_AltMD5[((g_SaveMedia[index].options & 0xf0) >> 4) - 1].md5;
                                }

                                uint8_t digest2[16];
                                hr = MD5Checksum(image2, digest2);
                                if (FAILED(hr))
                                {
                                    pass = false;
                                    success = false;
                                    printe("Failed computing MD5 checksum of reloaded image data [tga20] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
                                }
                                else if (memcmp(expected, digest2, 16) != 0)
                                {
                                    pass = false;
                                    success = false;
                                    printe("MD5 checksum of reloaded data doesn't match original [tga20]:\n%ls\n", szPath);
                                }
                            }
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

            bool istga = false;
            {
                wchar_t ext[_MAX_EXT];
                wchar_t fname[_MAX_FNAME];
                _wsplitpath_s(findData.cFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

                istga = (_wcsicmp(ext, L".tga") == 0);
            }

            OutputDebugString(findData.cFileName);
            OutputDebugStringA("\n");

            for (int j = 0; j < 2; ++j)
            {
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
                            hr = LoadFromTGAMemory(blob.GetBufferPointer(), blob.GetBufferSize(), !j ? TGA_FLAGS_NONE : TGA_FLAGS_BGR, &metadata, image);

                            if (FAILED(hr) && istga)
                            {
                                if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                                {
                                    success = false;
                                    printe("ERROR: frommemory expected success! (%08X) case %d\n%ls\n", static_cast<unsigned int>(hr), j, szPath);
                                }
                            }
                            else if (SUCCEEDED(hr) && !istga)
                            {
                                success = false;
                                printe("ERROR: frommemory expected failure case %d\n%ls\n", j, szPath);
                            }
                        }
                    }
                }

                // file
                {
                    TexMetadata metadata;
                    ScratchImage image;
                    HRESULT hr = LoadFromTGAFile(szPath, !j ? TGA_FLAGS_NONE : TGA_FLAGS_BGR, &metadata, image);

                    if (FAILED(hr) && istga)
                    {
                        if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                        {
                            success = false;
                            printe("ERROR: fromfile expected success ! (%08X) case %d\n%ls\n", static_cast<unsigned int>(hr), j, szPath);
                        }
                    }
                    else if (SUCCEEDED(hr) && !istga)
                    {
                        success = false;
                        printe("ERROR: fromfile expected failure case %d\n%ls\n", j, szPath);
                    }
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
