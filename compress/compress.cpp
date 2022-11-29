//-------------------------------------------------------------------------------------
// compress.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"

#include <wrl/client.h>

//#define DISABLE_BC6HBC7_COMPRESS

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wswitch"
#endif

namespace
{
    struct BCMedia
    {
        TexMetadata metadata;
        DXGI_FORMAT format;
        const wchar_t *fname;
    };

    const BCMedia g_BCMedia[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { { 32, 128, 1, 6, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOff.DDS" },
        { { 32, 128, 1, 6, 8, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOn.DDS" },
        { { 32, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURE2D_MipOff.DDS" },
        { { 32, 128, 1, 1, 8, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURE2D_MipOn.DDS" },
        { { 32, 128, 32, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURE3D_MipOff.DDS" },
        { { 32, 128, 32, 1, 8, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURE3D_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURECube_MipOff.DDS" },
        { { 32, 32, 1, 6, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC1_UNORM_SRV_DIMENSION_TEXTURECube_MipOn.DDS" },
        { { 32, 128, 1, 6, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOff.DDS" },
        { { 32, 128, 1, 6, 8, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOn.DDS" },
        { { 32, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURE2D_MipOff.DDS" },
        { { 32, 128, 1, 1, 8, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURE2D_MipOn.DDS" },
        { { 32, 128, 32, 1, 1, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURE3D_MipOff.DDS" },
        { { 32, 128, 32, 1, 8, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURE3D_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURECube_MipOff.DDS" },
        { { 32, 32, 1, 6, 6, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC2_UNORM_SRV_DIMENSION_TEXTURECube_MipOn.DDS" },
        { { 32, 128, 1, 6, 1, 0, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOff.DDS" },
        { { 32, 128, 1, 6, 8, 0, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOn.DDS" },
        { { 32, 128, 1, 1, 1, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURE2D_MipOff.DDS" },
        { { 32, 128, 1, 1, 8, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURE2D_MipOn.DDS" },
        { { 32, 128, 32, 1, 1, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURE3D_MipOff.DDS" },
        { { 32, 128, 32, 1, 8, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURE3D_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, 0, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURECube_MipOff.DDS" },
        { { 32, 32, 1, 6, 6, 0, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"io_BC3_UNORM_SRV_DIMENSION_TEXTURECube_MipOn.DDS" },
        { { 32, 128, 1, 6, 1, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURE2DArray_MipOff.DDS" },
        { { 32, 128, 1, 6, 8, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURE2DArray_MipOn.DDS" },
        { { 32, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURE2D_MipOff.DDS" },
        { { 32, 128, 1, 1, 8, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURE2D_MipOn.DDS" },
        { { 32, 128, 32, 1, 1, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURE3D_MipOff.DDS" },
        { { 32, 128, 32, 1, 8, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURE3D_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURECube_MipOff.DDS" },
        { { 32, 32, 1, 6, 6, 0, 0, DXGI_FORMAT_BC4_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_SNORM, MEDIA_PATH L"io_BC4_SNORM_SRV_DIMENSION_TEXTURECube_MipOn.DDS" },
        { { 32, 128, 1, 6, 1, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOff.DDS" },
        { { 32, 128, 1, 6, 8, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOn.DDS" },
        { { 32, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURE2D_MipOff.DDS" },
        { { 32, 128, 1, 1, 8, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURE2D_MipOn.DDS" },
        { { 32, 128, 32, 1, 1, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURE3D_MipOff.DDS" },
        { { 32, 128, 32, 1, 8, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURE3D_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURECube_MipOff.DDS" },
        { { 32, 32, 1, 6, 6, 0, 0, DXGI_FORMAT_BC4_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8_UNORM, MEDIA_PATH L"io_BC4_UNORM_SRV_DIMENSION_TEXTURECube_MipOn.DDS" },
        { { 32, 128, 1, 6, 1, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURE2DArray_MipOff.DDS" },
        { { 32, 128, 1, 6, 8, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURE2DArray_MipOn.DDS" },
        { { 32, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURE2D_MipOff.DDS" },
        { { 32, 128, 1, 1, 8, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURE2D_MipOn.DDS" },
        { { 32, 128, 32, 1, 1, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURE3D_MipOff.DDS" },
        { { 32, 128, 32, 1, 8, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURE3D_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURECube_MipOff.DDS" },
        { { 32, 32, 1, 6, 6, 0, 0, DXGI_FORMAT_BC5_SNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_SNORM, MEDIA_PATH L"io_BC5_SNORM_SRV_DIMENSION_TEXTURECube_MipOn.DDS" },
        { { 32, 128, 1, 6, 1, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOff.DDS" },
        { { 32, 128, 1, 6, 8, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURE2DArray_MipOn.DDS" },
        { { 32, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURE2D_MipOff.DDS" },
        { { 32, 128, 1, 1, 8, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURE2D_MipOn.DDS" },
        { { 32, 128, 32, 1, 1, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURE3D_MipOff.DDS" },
        { { 32, 128, 32, 1, 8, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURE3D_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURECube_MipOff.DDS" },
        { { 32, 32, 1, 6, 6, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8_UNORM, MEDIA_PATH L"io_BC5_UNORM_SRV_DIMENSION_TEXTURECube_MipOn.DDS" },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"testcubedxt5.dds" },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"testcubedxt5mip.dds" },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"testdxt1.dds" },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"testdxt1mip.dds" },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"testvoldxt1mip.dds" },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"windowslogo_DXT3.dds" },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC6H_SF16, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R32G32B32A32_FLOAT, MEDIA_PATH L"bc6h_sf16.dds" },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC6H_UF16, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R32G32B32A32_FLOAT, MEDIA_PATH L"bc6h_uf16.dds" },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"bc7_unorm.dds" },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MEDIA_PATH L"bc7_unorm_srgb.dds" },
        { { 8192, 4096, 1, 1, 14, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"world8192.dds" },
        { { 756, 512, 1, 1, 10, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"grydirt1.dds" },
        { { 756, 512, 1, 1, 10, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, DXGI_FORMAT_R8G8B8A8_UNORM, MEDIA_PATH L"TreeBark.dds" },
    };

    //-------------------------------------------------------------------------------------

    struct CompressMedia
    {
        DWORD flags;
        TexMetadata metadata;
        const wchar_t *fname;
    };

    enum CMEDIA_FLAGS
    {
        FLAGS_NONE = 0x0,
        FLAGS_BC45 = 0x1,
        FLAGS_BC6H = 0x2,
        FLAGS_BC7 = 0x4,
        FLAGS_ALL = 0xf,
        FLAGS_NOBC1 = 0x10,
        FLAGS_GPU = 0x20,
        FLAGS_GPU6 = (FLAGS_GPU | FLAGS_BC6H),
        FLAGS_GPU7 = (FLAGS_GPU | FLAGS_BC7),
        FLAGS_NOALPHA = 0x40,
        FLAGS_SKIP_WIDE = 0x80,
    };

    const CompressMedia g_CompressMedia[] =
    {
        // cmedia-flags | <source> width height depth arraySize mipLevels miscFlags format dimension | filename
        { FLAGS_ALL, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds" },
        { FLAGS_ALL
          & ~FLAGS_BC45,{ 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds" },
        { FLAGS_ALL
          & ~FLAGS_BC45,{ 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds" },
        { FLAGS_ALL
          & ~FLAGS_BC45,{ 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds" },
        { FLAGS_ALL
          & ~FLAGS_BC45,{ 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds" },
        { FLAGS_GPU7,   { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds" },
        { FLAGS_GPU7,   { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds" },
        { FLAGS_GPU7,   { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds" },
        { FLAGS_GPU7,   { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds" },
        { FLAGS_GPU7,   { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgb565.dds" },
        { FLAGS_GPU7,   { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8R8G8B8.dds" },
        { FLAGS_NONE,   { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds" },
        { FLAGS_GPU6,   { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds" },
        { FLAGS_BC45,   { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"normalmap.dds" },
        { FLAGS_BC45,   { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"normalmap2ch.dds" },

        { FLAGS_BC45
          | FLAGS_NOBC1
          | FLAGS_SKIP_WIDE, { 2048, 1024, 1, 1, 12, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"normals.dds" },
        { FLAGS_BC45
          | FLAGS_GPU7
          | FLAGS_SKIP_WIDE, { 2048, 2048, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tex4.png" },

        { FLAGS_BC6H,   { 268, 204, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"hdrtest.dds" },
        { FLAGS_NOBC1
          | FLAGS_BC6H, { 1296, 972, 1, 1, 11, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"yucca.dds" },
        #if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) || defined(_WIN7_PLATFORM_UPDATE)
        { FLAGS_BC6H,   { 768, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"96bpp_RGB_FP.TIF" },
        #endif

        { FLAGS_BC7,    { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds" },
        { FLAGS_BC7,    { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.jpg" },
        { FLAGS_BC7,    { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Omega-SwanNebula-M17.png" },

        #ifndef NO_WMP
        { FLAGS_NOBC1
          | FLAGS_BC7,  { 64, 64, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"Maui-32bppBGRA_64x64.wdp" },
        #endif

        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim01.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim02.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim03.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim04.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim05.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim06.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim07.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim08.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim09.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim10.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim11.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim12.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim13.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim14.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim15.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim16.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim17.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim18.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 512, 768, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim19.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim20.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim21.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim22.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim23.png" },
        { FLAGS_BC7 | FLAGS_GPU7, { 768, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"kodim24.png" },

        #ifdef _M_X64
        // Large test cases (64-bit only)
        { FLAGS_NOALPHA
          | FLAGS_SKIP_WIDE, { 16384, 8192, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuseTexture.png" },
        #endif
    };
}

//-------------------------------------------------------------------------------------

extern HRESULT CreateDevice( ID3D11Device** pDev, ID3D11DeviceContext** pContext );
extern HRESULT CreateWideImage( _In_count_(nimages) const Image* images, size_t nimages, _In_ const TexMetadata& metadata, _Out_ ScratchImage& result ); 
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );
extern const wchar_t* GetName( DXGI_FORMAT fmt );


//-------------------------------------------------------------------------------------
namespace
{
    double g_freq = 1.f;
    uint64_t g_counter = 0;

    void InitCounter()
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        g_freq = double(li.QuadPart) / 1000.0;
    }


    void StartCounter()
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        g_counter = li.QuadPart;
    }


    void StopCounter(const char* str)
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);

#if DEBUG
        char buff[256];
        sprintf_s(buff, "%s: %f ms", str, double(li.QuadPart - g_counter) / g_freq);
        OutputDebugStringA(buff);
#else
        UNREFERENCED_PARAMETER(str);
#endif
    }

    //-------------------------------------------------------------------------------------

    inline bool IsErrorTooLarge(float f, float threshold)
    {
        return (fabsf(f) > threshold) != 0;
    }

    inline bool SkipCompressCases(DXGI_FORMAT format, DXGI_FORMAT cformat, DWORD flags, bool gpu)
    {
        bool filter = false;

        switch (cformat)
        {
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            if (flags & FLAGS_NOBC1)
                filter = true;

            // BC1 does not encode alpha channel and is not HDR
            else if (format == DXGI_FORMAT_A8_UNORM)
                filter = true;
            else if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
                filter = true;
            break;

        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            if (flags & FLAGS_NOALPHA)
                filter = true;

            // BC2 and BC3 are not HDR formats
            else if (format == DXGI_FORMAT_R16G16B16A16_FLOAT || format == DXGI_FORMAT_R32G32B32A32_FLOAT)
                filter = true;
            break;

        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
            if (!(flags & FLAGS_BC45))
                filter = true;
            break;

        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
            if (!(flags & FLAGS_BC6H))
                filter = true;

            if ((flags & FLAGS_GPU) && !gpu)
                filter = true;

            // BC6H does not encode alpha channel
            if (format == DXGI_FORMAT_A8_UNORM)
                filter = true;
            break;

        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            if (!(flags & FLAGS_BC7))
                filter = true;

            if ((flags & FLAGS_GPU) && !gpu)
                filter = true;

            // BC7 is not an HDR format
            if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
                filter = true;
            break;
        }

        return filter;
    }
}


//-------------------------------------------------------------------------------------
// Decompress
bool Test01()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_BCMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_BCMedia[index].fname, szPath, MAX_PATH);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"bc_dec", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        wchar_t szDestPath[MAX_PATH] = {};
        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, fname, L".dds" );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );

        const TexMetadata* check = &g_BCMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting source data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in source file:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;

            // Test single image decompress
            ScratchImage image;
            hr = Decompress( *srcimage.GetImage(0,0,0), DXGI_FORMAT_UNKNOWN, image );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed decompress [single] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else if ( image.GetMetadata().format != g_BCMedia[index].format
                      || image.GetMetadata().width != metadata.width
                      || image.GetMetadata().height != metadata.height )
            {
                success = false;
                pass = false;
                printe( "Unexpected decompress result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                        image.GetMetadata().width, image.GetMetadata().height, GetName( image.GetMetadata().format ),
                        metadata.width, metadata.height, GetName( g_BCMedia[index].format ), szPath );
            }
            else
            {
                // TODO - Verify the image data (perhaps MD5 checksum)

                if ( metadata.mipLevels > 1 || metadata.arraySize > 1 || metadata.depth > 1 )
                {
                    // Test complex image decompress
                    ScratchImage image2;
                    hr = Decompress( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), DXGI_FORMAT_UNKNOWN, image2 );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed decompress [complex] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( image2.GetMetadata().format != g_BCMedia[index].format
                              || image2.GetMetadata().width != metadata.width
                              || image2.GetMetadata().height != metadata.height )
                    {
                        success = false;
                        pass = false;
                        printe( "Unexpected decompress [complex] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                metadata.width, metadata.height, GetName( g_BCMedia[index].format ), szPath );
                    }
                    else
                    {
                        // TODO - Verify the image data (perhaps MD5 checksum)
                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image2 );
                    }
                }
                else
                {
                    SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );

                    // Wide image test
                    ScratchImage imageWide;
                    hr = CreateWideImage( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), imageWide ); 
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed creating wide test image (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                    }
                    else if ( srcimage.GetImage(0,0,0)->rowPitch >= imageWide.GetImage(0,0,0)->rowPitch
                              || srcimage.GetImage(0,0,0)->format != imageWide.GetImage(0,0,0)->format )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed creating wide test image - result is same as source: %zu %zu\n%ls\n", srcimage.GetImage(0,0,0)->rowPitch, imageWide.GetImage(0,0,0)->rowPitch, szPath );
                    }
                    else
                    {
                        ScratchImage image2;
                        hr = Decompress( *imageWide.GetImage(0,0,0), DXGI_FORMAT_UNKNOWN, image2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed decompress [wide] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( image2.GetMetadata().format != g_BCMedia[index].format
                                  || image2.GetMetadata().width != metadata.width
                                  || image2.GetMetadata().height != metadata.height )
                        {
                            success = false;
                            pass = false;
                            printe( "Unexpected decompress [wide] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                    image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                    metadata.width, metadata.height, GetName( g_BCMedia[index].format ), szPath );
                        }
                        else
                        {
                            float targMSE = 0.00000001f;
                            float mse = 0, mseV[4] = {};
                            hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                printe( "Failed comparing decompress vs. decompress wide image data (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                            }
                            else if ( IsErrorTooLarge( mse, targMSE ) )
                            {
                                success = false;
                                printe( "Failed comparing decompress vs. decompress wide MSE = %f (%f %f %f %f)... %f:\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, szPath );
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

    print("%zu images tested, %zu images passed ", ncount, npass );
                                                                                                                              
    return success;
}


//-------------------------------------------------------------------------------------
// Compress (CPU)
bool Test02()
{
    InitCounter();

    static const DXGI_FORMAT s_BC[] = { DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM,
                                        DXGI_FORMAT_BC4_UNORM, DXGI_FORMAT_BC4_SNORM, DXGI_FORMAT_BC5_UNORM, DXGI_FORMAT_BC5_SNORM,
#ifdef DISABLE_BC6HBC7_COMPRESS
#pragma message("BC6HBC7 Compression testing disabled for development testing")
#else
                                        DXGI_FORMAT_BC6H_UF16, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM
#endif
                                      };

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_CompressMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_CompressMedia[index].fname, szPath, MAX_PATH);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"bc_enc", tempDir, MAX_PATH);
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

        const TexMetadata* check = &g_CompressMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting source data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in source file:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;

            print(".");

            for( UINT cindex = 0; cindex < std::size(s_BC); ++cindex )
            {
                DXGI_FORMAT cformat = s_BC[cindex];

                if ( IsSRGB( check->format ) )
                    cformat = MakeSRGB( cformat );

                if ( SkipCompressCases( check->format, cformat, g_CompressMedia[index].flags, false ) )
                    continue;

#ifdef _DEBUG
                OutputDebugString( GetName( cformat ) );
                OutputDebugStringA("\n");
#endif

                CMSE_FLAGS flags = CMSE_DEFAULT;

                switch( cformat )
                {
                case DXGI_FORMAT_BC1_UNORM:
                case DXGI_FORMAT_BC1_UNORM_SRGB:
                case DXGI_FORMAT_BC6H_UF16:
                case DXGI_FORMAT_BC6H_SF16:
                    flags |= CMSE_IGNORE_ALPHA;
                    break;

                case DXGI_FORMAT_BC4_UNORM:
                    flags |= CMSE_IGNORE_GREEN | CMSE_IGNORE_BLUE | CMSE_IGNORE_ALPHA;
                    break;

                case DXGI_FORMAT_BC4_SNORM:
                    flags |= CMSE_IGNORE_GREEN | CMSE_IGNORE_BLUE | CMSE_IGNORE_ALPHA | CMSE_IMAGE1_X2_BIAS;
                    break;

                case DXGI_FORMAT_BC5_UNORM:
                    flags |= CMSE_IGNORE_BLUE | CMSE_IGNORE_ALPHA;
                    break;

                case DXGI_FORMAT_BC5_SNORM:
                    flags |= CMSE_IGNORE_BLUE | CMSE_IGNORE_ALPHA | CMSE_IMAGE1_X2_BIAS;
                    break;
                }

                float mse, mseV[4];

                // Test single image compress (parallel version)
                StartCounter();

                ScratchImage image;
                hr = Compress( *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_DEFAULT
#ifdef _OPENMP
                                                                   | TEX_COMPRESS_PARALLEL
#endif
                                                                   , TEX_THRESHOLD_DEFAULT, image );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed compress [single M/T] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                }
                else if ( image.GetMetadata().format != cformat
                          || image.GetMetadata().width != metadata.width
                          || image.GetMetadata().height != metadata.height )
                {
                    success = false;
                    pass = false;
                    printe( "Unexpected compress [single M/T] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                            image.GetMetadata().width, image.GetMetadata().height, GetName( image.GetMetadata().format ),
                            metadata.width, metadata.height, GetName( cformat ), szPath );
                }
                else
                {
                    StopCounter( "CPU BC compress (parallel)" );

                    hr = ComputeMSE( *srcimage.GetImage(0,0,0), *image.GetImage(0,0,0), mse, mseV, flags );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing source to BC image (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                    }
                    else if ( IsErrorTooLarge( mse, 0.024f ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing source to BC image MSE = %f (%f %f %f %f)... 0.024f converting to %ls:\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                    }

                    //--- Test the faster cases with additional flag options
                    switch( cformat )
                    {
                    case DXGI_FORMAT_BC6H_UF16:
                    case DXGI_FORMAT_BC6H_SF16:
                    case DXGI_FORMAT_BC7_UNORM:
                    case DXGI_FORMAT_BC7_UNORM_SRGB:
                        if ( metadata.width > 32 || metadata.height > 32 )
                        {
                            // These cases are slow so not good choices for exhaustive testing
                            break;
                        }

                    default:
                        if ( metadata.width > 8192 || metadata.height > 8192 )
                        {
                            // These cases are slow so not good choices for exhaustive testing
                            break;
                        }

#ifdef _OPENMP
                        // Single-thread
                        {
                            ScratchImage image2;
                            hr = Compress( *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, image2 );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed compress [single S/T] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                            }
                            else if ( image2.GetMetadata().format != cformat
                                      || image2.GetMetadata().width != metadata.width
                                      || image2.GetMetadata().height != metadata.height )
                            {
                                success = false;
                                pass = false;
                                printe( "Unexpected compress [single S/T] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                        image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                        metadata.width, metadata.height, GetName( cformat ), szPath );
                            }
                            else
                            {
                                // Verify this matches the parallel version
                                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                                hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV, flags & ~CMSE_IMAGE1_X2_BIAS );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing single vs. parallel images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                }
                                else if ( IsErrorTooLarge( mse, 0.000001f ) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing single vs. parallel images MSE = %f (%f %f %f %f)... 0.000001f converting to %ls:\n%ls\n",
                                            mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                                }
                            }
                        }
#endif

                        // Dithering
                        if ( cformat == DXGI_FORMAT_BC1_UNORM || cformat == DXGI_FORMAT_BC1_UNORM_SRGB
                             || cformat == DXGI_FORMAT_BC2_UNORM || cformat == DXGI_FORMAT_BC2_UNORM_SRGB
                             || cformat == DXGI_FORMAT_BC3_UNORM || cformat == DXGI_FORMAT_BC3_UNORM_SRGB )
                        {
                            ScratchImage image2;
                            hr = Compress( *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_DITHER, TEX_THRESHOLD_DEFAULT, image2 );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed compress [dithering] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                            }
                            else if ( image2.GetMetadata().format != cformat
                                      || image2.GetMetadata().width != metadata.width
                                      || image2.GetMetadata().height != metadata.height )
                            {
                                success = false;
                                pass = false;
                                printe( "Unexpected compress [dithering] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                        image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                        metadata.width, metadata.height, GetName( cformat ), szPath );
                            }
                            else
                            {
                                // Verify the image is (slightly) different than the one produced without dithering
                                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                                hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV, flags & ~CMSE_IMAGE1_X2_BIAS );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing straight vs. dither images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                }
                                else if ( IsErrorTooLarge( mse, 0.01f ) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing straight vs. dither images MSE = %f (%f %f %f %f)... 0.01f converting to %ls:\n%ls\n",
                                            mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                                }
                            }
                        }

                        // Uniform color weighting (rather than perceptual)
                        if ( cformat == DXGI_FORMAT_BC1_UNORM || cformat == DXGI_FORMAT_BC1_UNORM_SRGB
                             || cformat == DXGI_FORMAT_BC2_UNORM || cformat == DXGI_FORMAT_BC2_UNORM_SRGB
                             || cformat == DXGI_FORMAT_BC3_UNORM || cformat == DXGI_FORMAT_BC3_UNORM_SRGB )
                        {
                            ScratchImage image2;
                            hr = Compress( *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_UNIFORM, TEX_THRESHOLD_DEFAULT, image2 );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed compress [uniform] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                            }
                            else if ( image2.GetMetadata().format != cformat
                                      || image2.GetMetadata().width != metadata.width
                                      || image2.GetMetadata().height != metadata.height )
                            {
                                success = false;
                                pass = false;
                                printe( "Unexpected compress [uniform] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                        image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                        metadata.width, metadata.height, GetName( cformat ), szPath );
                            }
                            else
                            {
                                // Verify the image here is (slightly) different than the one produced above (the ST non-dithered version)
                                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                                hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV, flags & ~CMSE_IMAGE1_X2_BIAS );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing straight vs. uniform images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                }
                                else if ( IsErrorTooLarge( mse, 0.022f ) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing straight vs. uniform images MSE = %f (%f %f %f %f)... 0.022f converting to %ls:\n%ls\n",
                                            mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                                }
                            }
                        }

                        // BC7 quick (aka mode 6)
                        if ( cformat == DXGI_FORMAT_BC7_UNORM || cformat == DXGI_FORMAT_BC7_UNORM_SRGB )
                        {
                            ScratchImage image2;
                            hr = Compress( *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_BC7_QUICK, TEX_THRESHOLD_DEFAULT, image2 );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed compress [quick] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                            }
                            else if ( image2.GetMetadata().format != cformat
                                      || image2.GetMetadata().width != metadata.width
                                      || image2.GetMetadata().height != metadata.height )
                            {
                                success = false;
                                pass = false;
                                printe( "Unexpected compress [quick] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                        image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                        metadata.width, metadata.height, GetName( cformat ), szPath );
                            }
                            else
                            {
                                // Verify the image here is (slightly) different than the one produced above (the ST version)
                                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                                hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV, flags & ~CMSE_IMAGE1_X2_BIAS );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing straight vs. quick images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                }
                                else if ( IsErrorTooLarge( mse, 0.0001f ) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing straight vs. quick images MSE = %f (%f %f %f %f)... 0.0001f converting to %ls:\n%ls\n",
                                            mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                                }
                            }
                        }

                        // BC7 mode 0 & 2
                        if (cformat == DXGI_FORMAT_BC7_UNORM || cformat == DXGI_FORMAT_BC7_UNORM_SRGB)
                        {
                            ScratchImage image2;
                            hr = Compress(*srcimage.GetImage(0, 0, 0), cformat, TEX_COMPRESS_BC7_USE_3SUBSETS, TEX_THRESHOLD_DEFAULT, image2);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed compress [3subsets] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath);
                            }
                            else if (image2.GetMetadata().format != cformat
                                || image2.GetMetadata().width != metadata.width
                                || image2.GetMetadata().height != metadata.height)
                            {
                                success = false;
                                pass = false;
                                printe("Unexpected compress [3subsets] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                    image2.GetMetadata().width, image2.GetMetadata().height, GetName(image2.GetMetadata().format),
                                    metadata.width, metadata.height, GetName(cformat), szPath);
                            }
                            else
                            {
                                // Verify the image here is (slightly) different than the one produced above (the ST version)
                                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                                hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV, flags & ~CMSE_IMAGE1_X2_BIAS);
                                if (FAILED(hr))
                                {
                                    success = false;
                                    pass = false;
                                    printe("Failed comparing straight vs. 3subsets images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath);
                                }
                                else if (IsErrorTooLarge(mse, 0.0001f))
                                {
                                    success = false;
                                    pass = false;
                                    printe("Failed comparing straight vs. 3subsets images MSE = %f (%f %f %f %f)... 0.0001f converting to %ls:\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath);
                                }
                            }
                        }

                        // Wide image tests
                        if ( !(g_CompressMedia[index].flags & FLAGS_SKIP_WIDE) )
                        {
                            ScratchImage imageWide;
                            hr = CreateWideImage( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), imageWide ); 
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed creating wide test image (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                            }
                            else if ( srcimage.GetImage(0,0,0)->rowPitch >= imageWide.GetImage(0,0,0)->rowPitch )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed creating wide test image - result is same as source: %zu %zu\n%ls\n", srcimage.GetImage(0,0,0)->rowPitch, imageWide.GetImage(0,0,0)->rowPitch, szPath );
                            }
                            else
                            {
                                ScratchImage image2;
                                hr = Compress( *imageWide.GetImage(0,0,0), cformat, TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, image2 );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed compress [wide image] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                }
                                else if ( image2.GetMetadata().format != cformat
                                          || image2.GetMetadata().width != metadata.width
                                          || image2.GetMetadata().height != metadata.height )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Unexpected compress [wide image] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                            image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                            metadata.width, metadata.height, GetName( cformat ), szPath );
                                }
                                else
                                {
                                    // Verify this matches the parallel version
                                    // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                                    hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV, flags & ~CMSE_IMAGE1_X2_BIAS );
                                    if ( FAILED(hr) )
                                    {
                                        success = false;
                                        pass = false;
                                        printe( "Failed comparing wide vs. parallel images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                    }
                                    else if ( IsErrorTooLarge( mse, 0.000001f ) )
                                    {
                                        success = false;
                                        pass = false;
                                        printe( "Failed comparing wide vs. parallel images MSE = %f (%f %f %f %f)... 0.000001f converting to %ls:\n%ls\n",
                                                mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                                    }
                                }
                            }
                        }
                        break;
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_" );
                    wcscat_s( tname, GetName(cformat) );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    if ( metadata.mipLevels > 1 || metadata.arraySize > 1 || metadata.depth > 1 )
                    {
                        // Test complex image compress
                        ScratchImage image2;
                        hr = Compress( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), cformat,
                                       TEX_COMPRESS_DEFAULT
#ifdef _OPENMP
                                       | TEX_COMPRESS_PARALLEL
#endif
                                       , TEX_THRESHOLD_DEFAULT, image2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed compress [complex] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                        }
                        else if ( image2.GetMetadata().format != cformat
                                    || image2.GetMetadata().width != metadata.width
                                    || image2.GetMetadata().height != metadata.height
                                    || image2.GetImageCount() != srcimage.GetImageCount() )
                        {
                            success = false;
                            pass = false;
                            printe( "Unexpected compress [complex] result %zu x %zu %ls %zu\n... %zu x %zu %ls %zu:\n%ls\n",
                                    image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ), image2.GetImageCount(),
                                    metadata.width, metadata.height, GetName( cformat ), srcimage.GetImageCount(), szPath );
                        }
                        else
                        {
                            // Verify the image data
                            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image2 );

                            float targMSE = ( cformat == DXGI_FORMAT_BC6H_UF16 || cformat == DXGI_FORMAT_BC6H_SF16) ? 0.2f : 0.07f;

                            for( size_t j = 0; j < srcimage.GetImageCount(); ++j )
                            {
                                auto& src = srcimage.GetImages()[ j ];

                                if ( src.width < 16 || src.height < 16 )
                                    break;

                                hr = ComputeMSE( src, image2.GetImages()[ j ], mse, mseV, flags );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing source [complex , %zu] to BC image (HRESULT %08X) converting to %ls:\n%ls\n", j, static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                    break;
                                }
                                else if ( IsErrorTooLarge( mse, targMSE ) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing source [complex, %zu] to BC image MSE = %f (%f %f %f %f)... %f converting to %ls:\n%ls\n",
                                            j, mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, GetName(cformat), szPath );
                                }
                            }
                        }
                    }
                    else
                    {
                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                    }
                }
            }

            if (pass)
                ++npass;

            ++ncount;
        }
    }

    print("\n%zu images tested, %zu images passed (%zu source images) ", ncount, npass, std::size(g_CompressMedia) );

    return success;
 }


 //-------------------------------------------------------------------------------------
// Compress (GPU)
bool Test03()
{
    InitCounter();

    ComPtr<ID3D11Device> device;
    HRESULT hr = CreateDevice( device.GetAddressOf(), nullptr );
    if ( FAILED(hr) )
    {
        printe( "Failed creating Direct3D device (HRESULT %08X)\n", static_cast<unsigned int>(hr) );
        return false;
    }

    static const DXGI_FORMAT s_BC[] = { DXGI_FORMAT_BC6H_UF16, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };

    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_CompressMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_CompressMedia[index].fname, szPath, MAX_PATH);
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
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"bc_gpu", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        TexMetadata metadata;
        ScratchImage srcimage;

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );
        }
        else
        {
            hr = LoadFromWICFile( szPath, WIC_FLAGS_NONE, &metadata, srcimage );
        }

        const TexMetadata* check = &g_CompressMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting source data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in source file:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;

            print(".");

            for( UINT cindex = 0; cindex < std::size(s_BC); ++cindex )
            {
                DXGI_FORMAT cformat = s_BC[cindex];

                if ( SkipCompressCases( check->format, cformat, g_CompressMedia[index].flags, true ) )
                    continue;

                CMSE_FLAGS flags = (cformat == DXGI_FORMAT_BC6H_UF16 || cformat == DXGI_FORMAT_BC6H_SF16) ? CMSE_IGNORE_ALPHA : CMSE_DEFAULT;

                float mse, mseV[4];

                // Test single GPU image compress
                StartCounter();

                ScratchImage image;
                hr = Compress( device.Get(), *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_DEFAULT, 1.f, image );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed GPU compress [single] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                }
                else if ( image.GetMetadata().format != cformat
                          || image.GetMetadata().width != metadata.width
                          || image.GetMetadata().height != metadata.height )
                {
                    success = false;
                    pass = false;
                    printe( "Unexpected GPU compress [single] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                            image.GetMetadata().width, image.GetMetadata().height, GetName( image.GetMetadata().format ),
                            metadata.width, metadata.height, GetName( cformat ), szPath );
                }
                else
                {
                    StopCounter( "GPU BC compress" );

                    hr = ComputeMSE( *srcimage.GetImage(0,0,0), *image.GetImage(0,0,0), mse, mseV, flags );
                    if ( FAILED(hr) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing source to GPU BC image (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                    }
                    else if ( IsErrorTooLarge( mse, 0.022f ) )
                    {
                        success = false;
                        pass = false;
                        printe( "Failed comparing source to GPU BC image MSE = %f (%f %f %f %f)... 0.022f converting to %ls:\n%ls\n",
                                mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                    }

                    wchar_t tname[MAX_PATH] = {};
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_" );
                    wcscat_s( tname, GetName(cformat) );

                    wchar_t szDestPath[MAX_PATH] = {};
                    _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                    if ( metadata.mipLevels > 1 || metadata.arraySize > 1 || metadata.depth > 1 )
                    {
                        // Test complex image compress
                        ScratchImage image2;
                        hr = Compress( device.Get(), srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), cformat, TEX_COMPRESS_DEFAULT, 1.f, image2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed GPU compress [complex] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                        }
                        else if ( image2.GetMetadata().format != cformat
                                    || image2.GetMetadata().width != metadata.width
                                    || image2.GetMetadata().height != metadata.height
                                    || image2.GetImageCount() != srcimage.GetImageCount() )
                        {
                            success = false;
                            pass = false;
                            printe( "Unexpected GPU compress [complex] result %zu x %zu %ls %zu\n... %zu x %zu %ls %zu:\n%ls\n",
                                    image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ), image2.GetImageCount(),
                                    metadata.width, metadata.height, GetName( cformat ), srcimage.GetImageCount(), szPath );
                        }
                        else
                        {
                            // Verify the image data
                            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image2 );

                            float targMSE = ( cformat == DXGI_FORMAT_BC6H_UF16 || cformat == DXGI_FORMAT_BC6H_SF16) ? 0.2f : 0.05f;

                            for( size_t j = 0; j < srcimage.GetImageCount(); ++j )
                            {
                                auto& src = srcimage.GetImages()[ j ];

                                if ( src.width < 16 || src.height < 16 )
                                    break;

                                hr = ComputeMSE( src, image2.GetImages()[ j ], mse, mseV, flags );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing source [complex , %zu] to GPU BC image (HRESULT %08X) converting to %ls:\n%ls\n", j, static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                    break;
                                }
                                else if ( IsErrorTooLarge( mse, targMSE ) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed comparing source [complex, %zu] to GPU BC image MSE = %f (%f %f %f %f)... %f converting to %ls:\n%ls\n",
                                            j, mse, mseV[0], mseV[1], mseV[2], mseV[3], targMSE, GetName(cformat), szPath );
                                }
                            }
                        }
                    }
                    else
                    {
                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );
                    }

                    // Alpha weight
                    if ( cformat == DXGI_FORMAT_BC7_UNORM || cformat == DXGI_FORMAT_BC7_UNORM_SRGB )
                    {
                        ScratchImage image2;
                        hr = Compress( device.Get(), *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_DEFAULT, 2.f, image2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed GPU compress [alpha weight] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                        }
                        else if ( image2.GetMetadata().format != cformat
                                    || image2.GetMetadata().width != metadata.width
                                    || image2.GetMetadata().height != metadata.height )
                        {
                            success = false;
                            pass = false;
                            printe( "Unexpected GPU compress [alpha weight] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                    image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                    metadata.width, metadata.height, GetName( cformat ), szPath );
                        }
                        else
                        {
                            // Verify the image is (slightly) different than the one produced without alpha weight
                            // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                            hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed GPU comparing straight vs. alpha weight images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                            }
                            else if ( IsErrorTooLarge( mse, 0.01f ) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed GPU comparing straight vs. alpha weight images MSE = %f (%f %f %f %f)... 0.01f converting to %ls:\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                            }
                        }
                    }

                    // BC7 quick (aka mode 6)
                    if (cformat == DXGI_FORMAT_BC7_UNORM || cformat == DXGI_FORMAT_BC7_UNORM_SRGB)
                    {
                        ScratchImage image2;
                        hr = Compress(device.Get(), *srcimage.GetImage(0, 0, 0), cformat, TEX_COMPRESS_BC7_QUICK, 2.f, image2);
                        if (FAILED(hr))
                        {
                            success = false;
                            pass = false;
                            printe("Failed GPU compress [quick] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath);
                        }
                        else if (image2.GetMetadata().format != cformat
                            || image2.GetMetadata().width != metadata.width
                            || image2.GetMetadata().height != metadata.height)
                        {
                            success = false;
                            pass = false;
                            printe("Unexpected GPU compress [quick] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                image2.GetMetadata().width, image2.GetMetadata().height, GetName(image2.GetMetadata().format),
                                metadata.width, metadata.height, GetName(cformat), szPath);
                        }
                        else
                        {
                            // Verify the image is (slightly) different than the one produced without alpha weight
                            // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                            hr = ComputeMSE(*image.GetImage(0, 0, 0), *image2.GetImage(0, 0, 0), mse, mseV);
                            if (FAILED(hr))
                            {
                                success = false;
                                pass = false;
                                printe("Failed GPU comparing straight vs. quick images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath);
                            }
                            else if (IsErrorTooLarge(mse, 0.002f))
                            {
                                success = false;
                                pass = false;
                                printe("Failed GPU comparing straight vs. quick images MSE = %f (%f %f %f %f)... 0.002f converting to %ls:\n%ls\n",
                                    mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath);
                            }
                        }
                    }

                    // BC7 mode 0 & 2
                    if ( cformat == DXGI_FORMAT_BC7_UNORM || cformat == DXGI_FORMAT_BC7_UNORM_SRGB )
                    {
                        ScratchImage image2;
                        hr = Compress( device.Get(), *srcimage.GetImage(0,0,0), cformat, TEX_COMPRESS_BC7_USE_3SUBSETS, 2.f, image2 );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed GPU compress [3subsets] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                        }
                        else if ( image2.GetMetadata().format != cformat
                                    || image2.GetMetadata().width != metadata.width
                                    || image2.GetMetadata().height != metadata.height )
                        {
                            success = false;
                            pass = false;
                            printe( "Unexpected GPU compress [3subsets] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                    image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                    metadata.width, metadata.height, GetName( cformat ), szPath );
                        }
                        else
                        {
                            // Verify the image is (slightly) different than the one produced without alpha weight
                            // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                            hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed GPU comparing straight vs. 3subsets images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                            }
                            else if ( IsErrorTooLarge( mse, 0.002f ) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed GPU comparing straight vs. 3subsets images MSE = %f (%f %f %f %f)... 0.002f converting to %ls:\n%ls\n",
                                        mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                            }
                        }
                    }

                    // Wide image tests
                    if ( !(g_CompressMedia[index].flags & FLAGS_SKIP_WIDE) )
                    {
                        ScratchImage imageWide;
                        hr = CreateWideImage( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), imageWide ); 
                        if ( FAILED(hr) )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed creating wide test image (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                        }
                        else if ( srcimage.GetImage(0,0,0)->rowPitch >= imageWide.GetImage(0,0,0)->rowPitch )
                        {
                            success = false;
                            pass = false;
                            printe( "Failed creating wide test image - result is same as source: %zu %zu\n%ls\n", srcimage.GetImage(0,0,0)->rowPitch, imageWide.GetImage(0,0,0)->rowPitch, szPath );
                        }
                        else
                        {
                            ScratchImage image2;
                            hr = Compress( device.Get(), *imageWide.GetImage(0,0,0), cformat, TEX_COMPRESS_DEFAULT, 1.f, image2 );
                            if ( FAILED(hr) )
                            {
                                success = false;
                                pass = false;
                                printe( "Failed GPU compress [wide] (HRESULT %08X) to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                            }
                            else if ( image2.GetMetadata().format != cformat
                                        || image2.GetMetadata().width != metadata.width
                                        || image2.GetMetadata().height != metadata.height )
                            {
                                success = false;
                                pass = false;
                                printe( "Unexpected GPU compress [wide] result %zu x %zu %ls\n... %zu x %zu %ls:\n%ls\n",
                                        image2.GetMetadata().width, image2.GetMetadata().height, GetName( image2.GetMetadata().format ),
                                        metadata.width, metadata.height, GetName( cformat ), szPath );
                            }
                            else
                            {
                                // Verify this matches the original GPU compressed version
                                // TESTTEST- SaveScratchImage( L"C:\\Temp\\XXX.DDS", DDS_FLAGS_NONE, image2 );
                                hr = ComputeMSE( *image.GetImage(0,0,0), *image2.GetImage(0,0,0), mse, mseV );
                                if ( FAILED(hr) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed GPU comparing standard vs. wide images (HRESULT %08X) converting to %ls:\n%ls\n", static_cast<unsigned int>(hr), GetName(cformat), szPath );
                                }
                                else if ( IsErrorTooLarge( mse, 0.01f ) )
                                {
                                    success = false;
                                    pass = false;
                                    printe( "Failed GPU comparing standard vs. wide images MSE = %f (%f %f %f %f)... 0.01f converting to %ls:\n%ls\n",
                                            mse, mseV[0], mseV[1], mseV[2], mseV[3], GetName(cformat), szPath );
                                }
                            }
                        }
                    }
                }
            }

            if (pass)
                ++npass;

            ++ncount;
        }
    }

    print("\n%zu images tested, %zu images passed (%zu source images) ", ncount, npass, std::size(g_CompressMedia) );

    return success;
}
