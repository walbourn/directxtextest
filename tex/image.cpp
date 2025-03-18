//-------------------------------------------------------------------------------------
// image.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"
#include "tex.h"

#include "DirectXTexP.h"

using namespace DirectX;

namespace
{
    const TexMetadata g_TestData[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension
        { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D },
        { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },

        { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D },
        { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D },
        { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC6H_SF16, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC6H_UF16, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D },
        { 2048, 2048, 1, 1, 12, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 1019, 677, 1, 1, 1, 0, 0, DXGI_FORMAT_R1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 8192, 4096, 1, 1, 14, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 756, 512, 1, 1, 10, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 756, 512, 1, 1, 10, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 512, 1, 1, 10, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 512, 1, 1, 10, 0, 0, DXGI_FORMAT_BC1_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D },

        // FL 10.1 Cubemap Array
        { 512, 512, 1, 12, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 512, 1, 12, 10, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },

        // DWORD alignment variant
        { 373, 525, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 101, 93, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D },

        // 16-bits
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D },

        // Luminance
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16_UNORM, TEX_DIMENSION_TEXTURE2D },

        // Premul alpha
        { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 304, 268, 1, 1, 9, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 128, 128, 128, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE3D },

        // HDR formats
        { 2048, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 2048, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 2048, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R11G11B10_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 2048, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R9G9B9E5_SHAREDEXP, TEX_DIMENSION_TEXTURE2D },
        { 1296, 972, 1, 1, 11, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D },

        // Normal maps
        { 512, 512, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 2048, 1024, 1, 1, 12, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },

        // Bump maps (legacy)
        { 1024, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8_SNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8_SNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_SNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16_SNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8_SNORM, TEX_DIMENSION_TEXTURE2D },

        // Legacy DirectX SDK Media files
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 128, 128, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },

        // Legacy DirectX SDK test files
        { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 16, 16, 1, 1, 5, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 512, 512, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 128, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 64, 64, 1, 1, 7, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 32, 32, 32, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D },

        // D3DX test files
        { 32, 1, 1, 6, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE1D },
        { 32, 1, 1, 6, 6, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE1D },
        { 32, 1, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE1D },
        { 32, 1, 1, 1, 6, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE1D },

        // Depth/stencil images
        { 1280, 720, 1, 1, 1, 0, 0, DXGI_FORMAT_D16_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 1280, 720, 1, 1, 1, 0, 0, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, TEX_DIMENSION_TEXTURE2D },
        { 1280, 720, 1, 1, 1, 0, 0, DXGI_FORMAT_D32_FLOAT, TEX_DIMENSION_TEXTURE2D },

        // YUV test images
        { 200, 200, 1, 1, 8, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 8, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_AYUV, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y210, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y216, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y410, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_Y416, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 4, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D },
        { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D },
        { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D },
        { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_AYUV, TEX_DIMENSION_TEXTURE2D },
        { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_Y210, TEX_DIMENSION_TEXTURE2D },
        { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_Y216, TEX_DIMENSION_TEXTURE2D },
        { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_Y410, TEX_DIMENSION_TEXTURE2D },
        { 1280, 1024, 1, 1, 1, 0, 0, DXGI_FORMAT_Y416, TEX_DIMENSION_TEXTURE2D },
        { 3264, 2448, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P010, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P016, TEX_DIMENSION_TEXTURE2D },
        { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV11, TEX_DIMENSION_TEXTURE2D },

        // Xbox One test images
        { 1920, 1485, 1, 1, 1, 0, 0, XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT, TEX_DIMENSION_TEXTURE2D },
        { 1920, 1485, 1, 1, 1, 0, 0, XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT, TEX_DIMENSION_TEXTURE2D },

        // Very large images
        { 16384, 16384, 1, 1, 15, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D },
        { 16384, 16384, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_SNORM, TEX_DIMENSION_TEXTURE2D },

        // Additional textures
        { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D },

        { 128, 128, 1, 6, 8, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC6H_UF16, TEX_DIMENSION_TEXTURE2D },
        { 1024, 1024, 1, 6, 11, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC6H_UF16, TEX_DIMENSION_TEXTURE2D  },

        { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D },
        {  1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D },
        { 1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE2D },
        {  1024, 1024, 1, 1, 11, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D  },

        { 256, 256, 1, 1, 1, 0, 0, WIN11_DXGI_FORMAT_A4B4G4R4_UNORM, TEX_DIMENSION_TEXTURE2D },
    };
}

//-------------------------------------------------------------------------------------
// TexMetadata
bool TEXTest::Test18()
{
    bool success = true;

    for( size_t index=0; index < std::size(g_TestData); ++index )
    {
        const auto& mdata = g_TestData[index];

        if (mdata.ComputeIndex(0, 0, 0) != 0)
        {
            printe( "ERROR: ComputeIndex failed (%zu)\n", index );
            success = false;
        }

        if (mdata.ComputeIndex(UINT32_MAX, UINT32_MAX, UINT32_MAX) != size_t(-1)
            || mdata.ComputeIndex(0, UINT32_MAX, UINT32_MAX) != size_t(-1))
        {
            printe( "ERROR: ComputeIndex invalid failed (%zu)\n", index );
            success = false;
        }

        if (mdata.CalculateSubresource(0, 0) != 0)
        {
            printe( "ERROR: CalculateSubresource failed (%zu)\n", index );
            success = false;
        }

        if (mdata.CalculateSubresource(UINT32_MAX, UINT32_MAX) != uint32_t(-1)
            || mdata.CalculateSubresource(0, UINT32_MAX, UINT32_MAX) != uint32_t(-1))
        {
            printe( "ERROR: CalculateSubresource invalid failed (%zu)\n", index );
            success = false;
        }

        // This function doesn't care about the format of the data so we can test plane offset on any format.
        if (mdata.CalculateSubresource(0, 0, 0) != 0
            || mdata.CalculateSubresource(0, 0, 1) == 0)
        {
            printe( "ERROR: CalculateSubresource plane failed (%zu)\n", index );
            success = false;
        }

        if (mdata.CalculateSubresource(UINT32_MAX, UINT32_MAX, 1) != uint32_t(-1)
            || mdata.CalculateSubresource(0, UINT32_MAX, 1) != uint32_t(-1))
        {
            printe( "ERROR: CalculateSubresource plane invalid failed (%zu)\n", index );
            success = false;
        }

        if (mdata.IsCubemap())
        {
            if ((mdata.dimension != TEX_DIMENSION_TEXTURE2D) || (mdata.arraySize < 6) || ((mdata.arraySize % 6) != 0) || !(mdata.miscFlags & TEX_MISC_TEXTURECUBE))
            {
                printe( "ERROR: IsCubemap failed (%zu)\n", index );
                success = false;
            }
        }
        else if (mdata.miscFlags & TEX_MISC_TEXTURECUBE)
        {
            printe( "ERROR: IsCubemap failed (%zu)\n", index );
            success = false;
        }

        if (mdata.IsPMAlpha())
        {
            if (mdata.GetAlphaMode() != TEX_ALPHA_MODE_PREMULTIPLIED)
            {
                printe( "ERROR: IsPMAlpha failed (%zu)\n", index );
                success = false;
            }
        }
        else if (mdata.GetAlphaMode() == TEX_ALPHA_MODE_PREMULTIPLIED)
        {
            printe( "ERROR: IsPMAlpha failed (%zu)\n", index );
            success = false;
        }

        if (mdata.IsVolumemap())
        {
            if (mdata.dimension != TEX_DIMENSION_TEXTURE3D)
            {
                printe( "ERROR: IsVolumemap failed (%zu)\n", index );
                success = false;
            }
        }
        else if (mdata.dimension == TEX_DIMENSION_TEXTURE3D)
        {
            printe( "ERROR: IsVolumemap failed (%zu)\n", index );
            success = false;
        }

        for(uint32_t j = 0; j < 5; ++j)
        {
            TexMetadata mdata2 = mdata;
            mdata2.miscFlags2 = 0;

            if (mdata2.GetAlphaMode() != TEX_ALPHA_MODE_UNKNOWN)
            {
                printe( "ERROR: GetAlphaMode failed (%zu)\n", index );
                success = false;
            }
            else
            {
                mdata2.SetAlphaMode(static_cast<TEX_ALPHA_MODE>(j));

                if (mdata2.GetAlphaMode() != static_cast<TEX_ALPHA_MODE>(j))
                {
                    printe( "ERROR: SetAlphaMode/GetAlphaMode failed (%zu)\n", index );
                    success = false;
                }
            }
        }

        // invalid metadata
        TexMetadata imdata = mdata;
        imdata.dimension = static_cast<TEX_DIMENSION>(0);
        if (imdata.ComputeIndex(0, 0, 0) != size_t(-1))
        {
            printe( "ERROR: ComputeIndex invalid dimension failed (%zu)\n", index );
            success = false;
        }

        if (imdata.CalculateSubresource(0, 0) != uint32_t(-1))
        {
            printe( "ERROR: CalculateSubresource invalid dimension failed (%zu)\n", index );
            success = false;
        }

        if (imdata.CalculateSubresource(0, 0, 0) != uint32_t(-1))
        {
            printe( "ERROR: CalculateSubresource 3 arg invalid dimension failed (%zu)\n", index );
            success = false;
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// ScratchImage
bool TEXTest::Test19()
{
    bool success = true;

    for( size_t index=0; index < std::size(g_TestData); ++index )
    {
        const auto& mdata = g_TestData[index];

        ScratchImage image;
        HRESULT hr = image.Initialize(mdata);
        if (FAILED(hr)
            || image.GetPixels() == nullptr
            || memcmp(&image.GetMetadata(), &mdata, sizeof(TexMetadata)) != 0)
        {
            printe( "ERROR: Initialize failed (%zu)\n", index );
            success = false;
        }

        image.Release();

        // invalid args
        {
            TexMetadata imdata = image.GetMetadata();
            imdata.dimension = static_cast<TEX_DIMENSION>(0);
            hr = image.Initialize(imdata);
            if (SUCCEEDED(hr))
            {
                printe( "Failed invalid metadata dimension test\n");
                success = false;
            }

            imdata = image.GetMetadata();
            imdata.format = DXGI_FORMAT_UNKNOWN;
            hr = image.Initialize(imdata);
            if (SUCCEEDED(hr))
            {
                printe( "Failed invalid metadata format test\n");
                success = false;
            }

            imdata = image.GetMetadata();
            imdata.format = DXGI_FORMAT_P8;
            hr = image.Initialize(imdata);
            if (SUCCEEDED(hr))
            {
                printe( "Failed unsupported palettized format metadata test\n");
                success = false;
            }
        }

        image.Release();

        // TODO: CP_FLAGS_LIMIT_4GB

        switch(mdata.dimension)
        {
        case TEX_DIMENSION_TEXTURE1D:
            hr = image.Initialize1D(mdata.format, mdata.width, mdata.arraySize, mdata.mipLevels);
            if (FAILED(hr)
                || image.GetPixels() == nullptr
                || image.GetMetadata().format != mdata.format
                || image.GetMetadata().width != mdata.width
                || image.GetMetadata().height != 1
                || image.GetMetadata().arraySize != mdata.arraySize
                || image.GetMetadata().mipLevels != mdata.mipLevels
                || image.GetMetadata().dimension != TEX_DIMENSION_TEXTURE1D)
            {
                printe( "ERROR: Initialize1D failed (%zu)\n", index );
                success = false;
            }
            break;

        case TEX_DIMENSION_TEXTURE2D:
            hr = image.Initialize2D(mdata.format, mdata.width, mdata.height, mdata.arraySize, mdata.mipLevels);
            if (FAILED(hr)
                || image.GetPixels() == nullptr
                || image.GetMetadata().format != mdata.format
                || image.GetMetadata().width != mdata.width
                || image.GetMetadata().height != mdata.height
                || image.GetMetadata().arraySize != mdata.arraySize
                || image.GetMetadata().mipLevels != mdata.mipLevels
                || image.GetMetadata().dimension != TEX_DIMENSION_TEXTURE2D)
            {
                printe( "ERROR: Initialize2D failed (%zu)\n", index );
                success = false;
            }

            // invalid args
            hr = image.Initialize2D(DXGI_FORMAT_UNKNOWN, mdata.width, mdata.height, mdata.arraySize, mdata.mipLevels);
            if (SUCCEEDED(hr))
            {
                printe( "Failed invalid format 2D test\n");
                success = false;
            }

            hr = image.Initialize2D(DXGI_FORMAT_P8, mdata.width, mdata.height, mdata.arraySize, mdata.mipLevels);
            if (SUCCEEDED(hr))
            {
                printe( "Failed unsupported palettized format 2D test\n");
                success = false;
            }

            if (mdata.miscFlags & TEX_MISC_TEXTURECUBE)
            {
                image.Release();

                hr = image.InitializeCube(mdata.format, mdata.width, mdata.height, std::min<size_t>(1, mdata.arraySize / 6), mdata.mipLevels);
                if (FAILED(hr)
                    || image.GetPixels() == nullptr
                    || image.GetMetadata().format != mdata.format
                    || image.GetMetadata().width != mdata.width
                    || image.GetMetadata().height != mdata.height
                    || (image.GetMetadata().arraySize % 6) != 0
                    || image.GetMetadata().mipLevels != mdata.mipLevels
                    || image.GetMetadata().dimension != TEX_DIMENSION_TEXTURE2D)
                {
                    printe( "ERROR: InitializeCube failed (%zu)\n", index );
                    success = false;
                }

                // invalid args
                hr = image.InitializeCube(mdata.format, mdata.width, mdata.height, 0, mdata.mipLevels);
                if (SUCCEEDED(hr))
                {
                    printe( "Failed invalid count cube test\n");
                    success = false;
                }
            }
            break;

        case TEX_DIMENSION_TEXTURE3D:
            hr = image.Initialize3D(mdata.format, mdata.width, mdata.height, mdata.depth, mdata.mipLevels);
            if (FAILED(hr)
                || image.GetPixels() == nullptr
                || image.GetMetadata().format != mdata.format
                || image.GetMetadata().width != mdata.width
                || image.GetMetadata().height != mdata.height
                || image.GetMetadata().depth != mdata.depth
                || image.GetMetadata().arraySize != mdata.arraySize
                || image.GetMetadata().mipLevels != mdata.mipLevels
                || image.GetMetadata().dimension != TEX_DIMENSION_TEXTURE3D)
            {
                printe( "ERROR: Initialize3D failed (%zu)\n", index );
                success = false;
            }

            // invalid args
            hr = image.Initialize3D(DXGI_FORMAT_UNKNOWN, mdata.width, mdata.height, mdata.depth, mdata.mipLevels);
            if (SUCCEEDED(hr))
            {
                printe( "Failed invalid format 3D test\n");
                success = false;
            }

            hr = image.Initialize3D(DXGI_FORMAT_P8, mdata.width, mdata.height, mdata.depth, mdata.mipLevels);
            if (SUCCEEDED(hr))
            {
                printe( "Failed unsupported palettized format 3D test\n");
                success = false;
            }

            hr = image.Initialize3D(mdata.format, mdata.width, mdata.height, UINT32_MAX, mdata.mipLevels);
            if (SUCCEEDED(hr))
            {
                printe( "Failed invalid depth 3D test\n");
                success = false;
            }
            break;

        default:
            // Unknown value to skip it
            continue;
        }

        // TODO: these are used by texassemble
        // InitializeArrayFromImages
        // InitializeCubeFromImages
        // Initialize3DFromImages

        if (!image.OverrideFormat(DXGI_FORMAT_R32G32B32A32_FLOAT)
            || image.GetMetadata().format != DXGI_FORMAT_R32G32B32A32_FLOAT)
        {
            printe( "ERROR: OverrideFormat failed (%zu)\n", index );
            success = false;
        }

        if (!image.OverrideFormat(DXGI_FORMAT_BC1_TYPELESS)
            || image.GetMetadata().format != DXGI_FORMAT_BC1_TYPELESS)
        {
            printe( "ERROR: OverrideFormat bc failed (%zu)\n", index );
            success = false;
        }

        if (image.OverrideFormat(DXGI_FORMAT_UNKNOWN))
        {
            printe( "ERROR: OverrideFormat unknown failed (%zu)\n", index );
            success = false;
        }

        image.Release();

        if (image.OverrideFormat(DXGI_FORMAT_R8G8_UNORM))
        {
            printe( "ERROR: OverrideFormat empty failed (%zu)\n", index );
            success = false;
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// IsAlphaAllOpaque
bool TEXTest::Test14()
{
    bool success = true;

    {
        ScratchImage image;
        HRESULT hr = image.Initialize1D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1);
        if (FAILED(hr))
        {
            success = false;
            printe("Failed creating test image (HRESULT %08X)\n", static_cast<unsigned int>(hr));
        }
        else
        {
            auto img = image.GetImage(0, 0, 0);

            for (unsigned j = 0; j < 256; ++j)
            {
                DWORD pixel = j | (j << 8) | (j << 16) | (j << 24);
                *reinterpret_cast<uint32_t*>(img->pixels) = pixel;

                bool isao = image.IsAlphaAllOpaque();
                if (isao != (j >= 255))
                {
                    success = false;
                    printe("Failed IsAlphaAllOpaque (%u): %s ... %s\n", j, (isao) ? "true" : "false", (j >= 255) ? "true" : "false");
                }
            }
        }
    }

    struct TestMedia
    {
        TexMetadata metadata;
        const wchar_t *fname;
        bool isAlphaAllOpaque;
    };

    static const TestMedia s_TestMedia[] =
    {
        // width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename | bool
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"reftexture.dds", true },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888.dds", true },
        { { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888.dds", true },
        { { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1.dds", true },

    #ifndef BUILD_BVT_ONLY
        { { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test555.dds", true },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"test8888mip.dds", true },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888.dds", true },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcube8888mip.dds", true },
        { { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5.dds", true },
        { { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testcubedxt5mip.dds", true },
        { { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"testdxt1mip.dds", true },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvol8888mip.dds", true },
        { { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, MEDIA_PATH L"testvoldxt1mip.dds", true },
        { { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2B10G10R10.dds", true },
        { { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earth_A2R10G10B10.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bc7_unorm.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_BC7_UNORM_SRGB, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"bc7_unorm_srgb.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A1R5G5B5.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_BC2_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_DXT3.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B5G6R5_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_R5G6B5.dds", true },
        { { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X8B8G8R8.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A8R3G3B2.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R16G16B16A16_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba16f.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_rgba32f.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"alphaedge.dds", false },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4L4.dds", true },
        { { 256, 256, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_X4R4G4B4.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, DXGI_FORMAT_B4G4R4A4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_A4R4G4B4.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"dx5_logo.dds", true },
        { { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"win95.dds", true },
        { { 256, 256, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lobbycube.dds", true },
        { { 8192, 4096, 1, 1, 14, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"world8192.dds", true },
        { { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S.dds", false },
        { { 304, 268, 1, 1, 9, 0, TEX_ALPHA_MODE_PREMULTIPLIED, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"tree02S_pmalpha.dds", false },
        { { 1024, 512, 1, 1, 11, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"earthdiffuse.dds", false },
        { { 800, 800, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"SplashScreen2.dds", false },
        { { 200, 200, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lena.dds", true },
        { { 256, 256, 1, 1, 1, 0, 0, WIN11_DXGI_FORMAT_A4B4G4R4_UNORM, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"windowslogo_191.dds", true },
    #endif
    };

    // TODO: DXGI_FORMAT_BC1_TYPELESS, DXGI_FORMAT_BC2_TYPELESS, DXGI_FORMAT_BC3_TYPELESS, DXGI_FORMAT_BC7_TYPELESS

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(s_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(s_TestMedia[index].fname, szPath, MAX_PATH);
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
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, image );

        const TexMetadata* check = &s_TestMedia[index].metadata;
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
            bool isao = image.IsAlphaAllOpaque();
            if ( isao != s_TestMedia[index].isAlphaAllOpaque )
            {
                success = false;
                printe( "Failed IsAlphaAllOpaque: %s ... %s\n%ls\n", (isao) ? "true" : "false", (s_TestMedia[index].isAlphaAllOpaque) ? "true" : "false", szPath );
            }
            else
                ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    // invalid args test
    {
        ScratchImage emptyImage;
        bool res = emptyImage.IsAlphaAllOpaque();
        if (res)
        {
            success = false;
            printe("Failed empty image test\n");
        }
    }

    return success;
}
