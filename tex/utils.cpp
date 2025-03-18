//-------------------------------------------------------------------------------------
// utils.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "tex.h"

#include "DirectXTexP.h"

#include <wrl/client.h>

#include <algorithm>

#include "getname.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace
{
    const UINT DXGI_START = 1;
    const UINT DXGI_END = 191; // as of DirectX Agility SDK 1.610
}

//-------------------------------------------------------------------------------------

static_assert(!std::is_copy_assignable<ScratchImage>::value, "Copy Assign.");
static_assert(!std::is_copy_constructible<ScratchImage>::value, "Copy Ctor.");
static_assert(std::is_nothrow_move_constructible<ScratchImage>::value, "Move Ctor.");
static_assert(std::is_nothrow_move_assignable<ScratchImage>::value, "Move Assign.");

static_assert(!std::is_copy_assignable<Blob>::value, "Copy Assign.");
static_assert(!std::is_copy_constructible<Blob>::value, "Copy Ctor.");
static_assert(std::is_nothrow_move_constructible<Blob>::value, "Move Ctor.");
static_assert(std::is_nothrow_move_assignable<Blob>::value, "Move Assign.");

//-------------------------------------------------------------------------------------
// IsValid
// IsCompressed
// IsPacked
// IsVideo
// IsPlanar
// IsPalettized
// IsDepthStencil
// IsSRGB
// IsBGR
// IsTypeless
// BitsPerPixel
// BitsPerColor
// BytesPerBlock
// FormatDataType
bool TEXTest::Test01()
{
    static const DXGI_FORMAT bcFmts[] =
    {
        DXGI_FORMAT_BC1_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB,
        DXGI_FORMAT_BC2_TYPELESS, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB,
        DXGI_FORMAT_BC3_TYPELESS, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB,
        DXGI_FORMAT_BC4_TYPELESS, DXGI_FORMAT_BC4_UNORM, DXGI_FORMAT_BC4_SNORM,
        DXGI_FORMAT_BC5_TYPELESS, DXGI_FORMAT_BC5_UNORM, DXGI_FORMAT_BC5_SNORM,
        DXGI_FORMAT_BC6H_TYPELESS, DXGI_FORMAT_BC6H_UF16, DXGI_FORMAT_BC6H_SF16,
        DXGI_FORMAT_BC7_TYPELESS, DXGI_FORMAT_BC7_UNORM, DXGI_FORMAT_BC7_UNORM_SRGB
    };
    static const DXGI_FORMAT packedFmts[] =
    {
        DXGI_FORMAT_R8G8_B8G8_UNORM, DXGI_FORMAT_G8R8_G8B8_UNORM,
        DXGI_FORMAT_YUY2, DXGI_FORMAT_Y210, DXGI_FORMAT_Y216,
    };
    static const DXGI_FORMAT srgb[] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
        DXGI_FORMAT_BC1_UNORM_SRGB, DXGI_FORMAT_BC2_UNORM_SRGB, DXGI_FORMAT_BC3_UNORM_SRGB, DXGI_FORMAT_BC7_UNORM_SRGB
    };
    static const DXGI_FORMAT bgr[]
    {
        DXGI_FORMAT_B5G6R5_UNORM, DXGI_FORMAT_B5G5R5A1_UNORM, DXGI_FORMAT_B4G4R4A4_UNORM,
        DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_B8G8R8X8_TYPELESS, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
        WIN11_DXGI_FORMAT_A4B4G4R4_UNORM,
    };
    static const DXGI_FORMAT typeless[] =
    {
        DXGI_FORMAT_R32G32B32A32_TYPELESS, DXGI_FORMAT_R32G32B32_TYPELESS, DXGI_FORMAT_R16G16B16A16_TYPELESS, DXGI_FORMAT_R32G32_TYPELESS,
        DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, DXGI_FORMAT_R10G10B10A2_TYPELESS,
        DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R16G16_TYPELESS, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT_X24_TYPELESS_G8_UINT, DXGI_FORMAT_R8G8_TYPELESS, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R8_TYPELESS,
        DXGI_FORMAT_BC1_TYPELESS, DXGI_FORMAT_BC2_TYPELESS, DXGI_FORMAT_BC3_TYPELESS, DXGI_FORMAT_BC4_TYPELESS, DXGI_FORMAT_BC5_TYPELESS,
        DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8X8_TYPELESS, DXGI_FORMAT_BC6H_TYPELESS, DXGI_FORMAT_BC7_TYPELESS,
        XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS, XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT,
    };
    static const DXGI_FORMAT partialTypeless[] =
    {
        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_X24_TYPELESS_G8_UINT,
        XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS, XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT,
    };
    static const DXGI_FORMAT alpha[] =
    {
        DXGI_FORMAT_R32G32B32A32_TYPELESS, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_R16G16B16A16_TYPELESS, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_SINT,
        DXGI_FORMAT_R10G10B10A2_TYPELESS, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UINT,
        DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R8G8B8A8_SINT,
        DXGI_FORMAT_A8_UNORM,
        DXGI_FORMAT_BC1_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB,
        DXGI_FORMAT_BC2_TYPELESS, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB,
        DXGI_FORMAT_BC3_TYPELESS, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
        DXGI_FORMAT_BC7_TYPELESS, DXGI_FORMAT_BC7_UNORM, DXGI_FORMAT_BC7_UNORM_SRGB,
        DXGI_FORMAT_AYUV, DXGI_FORMAT_Y410, DXGI_FORMAT_Y416, DXGI_FORMAT_A8P8, DXGI_FORMAT_AI44, DXGI_FORMAT_IA44,
        DXGI_FORMAT_B4G4R4A4_UNORM,
        XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT, XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT, XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM,
        WIN11_DXGI_FORMAT_A4B4G4R4_UNORM,
    };
    static const DXGI_FORMAT video[] =
    {
        DXGI_FORMAT_AYUV, DXGI_FORMAT_Y410, DXGI_FORMAT_Y416,
        DXGI_FORMAT_YUY2, DXGI_FORMAT_Y210, DXGI_FORMAT_Y216,
        DXGI_FORMAT_NV12, DXGI_FORMAT_P010, DXGI_FORMAT_P016, DXGI_FORMAT_420_OPAQUE, DXGI_FORMAT_NV11,
        DXGI_FORMAT_AI44, DXGI_FORMAT_IA44, DXGI_FORMAT_P8, DXGI_FORMAT_A8P8,
        WIN10_DXGI_FORMAT_P208, WIN10_DXGI_FORMAT_V208, WIN10_DXGI_FORMAT_V408,
    };
    static const DXGI_FORMAT planar[] =
    {
        DXGI_FORMAT_NV12, DXGI_FORMAT_P010, DXGI_FORMAT_P016, DXGI_FORMAT_420_OPAQUE, DXGI_FORMAT_NV11,
        XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT, XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS, XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT,
        WIN10_DXGI_FORMAT_P208, WIN10_DXGI_FORMAT_V208, WIN10_DXGI_FORMAT_V408,
    };
    static const DXGI_FORMAT pal[] =
    {
        DXGI_FORMAT_AI44, DXGI_FORMAT_IA44, DXGI_FORMAT_P8, DXGI_FORMAT_A8P8,
    };
    static const DXGI_FORMAT depthStencil[] =
    {
        DXGI_FORMAT_R32G8X24_TYPELESS,
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
        DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_R24G8_TYPELESS,
        DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_X24_TYPELESS_G8_UINT,
        DXGI_FORMAT_D16_UNORM,
        XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT, XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS, XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT,
    };

    static const DXGI_FORMAT floatType[] =
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
        DXGI_FORMAT_R11G11B10_FLOAT,
        DXGI_FORMAT_R16G16_FLOAT,
        DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_FLOAT,
        DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
        DXGI_FORMAT_BC6H_UF16, DXGI_FORMAT_BC6H_SF16,
        XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT,XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT,
    };
    static const DXGI_FORMAT unormType[] =
    {
        DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        DXGI_FORMAT_R16G16_UNORM,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_A8_UNORM,
        DXGI_FORMAT_R1_UNORM,
        DXGI_FORMAT_R8G8_B8G8_UNORM, DXGI_FORMAT_G8R8_G8B8_UNORM,
        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB,
        DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB,
        DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB,
        DXGI_FORMAT_BC4_UNORM, DXGI_FORMAT_BC5_UNORM,
        DXGI_FORMAT_B5G6R5_UNORM, DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8X8_UNORM,
        DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
        DXGI_FORMAT_BC7_UNORM, DXGI_FORMAT_BC7_UNORM_SRGB,
        DXGI_FORMAT_B4G4R4A4_UNORM,
        DXGI_FORMAT_AYUV, DXGI_FORMAT_Y410, DXGI_FORMAT_Y416, DXGI_FORMAT_YUY2, DXGI_FORMAT_Y210, DXGI_FORMAT_Y216,
        XBOX_DXGI_FORMAT_R4G4_UNORM,
        WIN11_DXGI_FORMAT_A4B4G4R4_UNORM,
    };
    static const DXGI_FORMAT uintType[] =
    {
        DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R16G16B16A16_UINT,
        DXGI_FORMAT_R32G32_UINT,
        DXGI_FORMAT_R10G10B10A2_UINT,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R16G16_UINT,
        DXGI_FORMAT_R32_UINT,
        DXGI_FORMAT_R8G8_UINT,
        DXGI_FORMAT_R16_UINT,
        DXGI_FORMAT_R8_UINT,
    };
    static const DXGI_FORMAT snormType[] =
    {
        DXGI_FORMAT_R16G16B16A16_SNORM,
        DXGI_FORMAT_R8G8B8A8_SNORM,
        DXGI_FORMAT_R16G16_SNORM,
        DXGI_FORMAT_R8G8_SNORM,
        DXGI_FORMAT_R16_SNORM,
        DXGI_FORMAT_R8_SNORM,
        DXGI_FORMAT_BC4_SNORM, DXGI_FORMAT_BC5_SNORM,
        XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM,
    };
    static const DXGI_FORMAT sintType[] =
    {
        DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32_SINT,
        DXGI_FORMAT_R16G16B16A16_SINT,
        DXGI_FORMAT_R32G32_SINT,
        DXGI_FORMAT_R8G8B8A8_SINT,
        DXGI_FORMAT_R16G16_SINT,
        DXGI_FORMAT_R32_SINT,
        DXGI_FORMAT_R8G8_SINT,
        DXGI_FORMAT_R16_SINT,
        DXGI_FORMAT_R8_SINT,
    };
    bool success = true;

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
#pragma warning(suppress : 4389)
        bool isCompressed = std::find(std::begin(bcFmts), std::end(bcFmts), f) != std::end(bcFmts);

        bool isPAK = std::find(std::begin(packedFmts), std::end(packedFmts), f) != std::end(packedFmts);

        bool isSRGB = std::find(std::begin(srgb), std::end(srgb), f) != std::end(srgb);

        bool isBGR = std::find(std::begin(bgr), std::end(bgr), f) != std::end(bgr);

        bool isTypeless = std::find(std::begin(typeless), std::end(typeless), f) != std::end(typeless);
        bool isPartialTypeless = false;
        if (isTypeless)
        {
            isPartialTypeless = std::find(std::begin(partialTypeless), std::end(partialTypeless), f) == std::end(partialTypeless);
        }

        bool hasAlpha = std::find(std::begin(alpha), std::end(alpha), f) != std::end(alpha);

        bool isVideo = std::find(std::begin(video), std::end(video), f) != std::end(video);

        bool isPlanar = std::find(std::begin(planar), std::end(planar), f) != std::end(planar);

        bool isPal = std::find(std::begin(pal), std::end(pal), f) != std::end(pal);

        bool isDepthStencil = std::find(std::begin(depthStencil), std::end(depthStencil), f) != std::end(depthStencil);

        bool isFloatType = std::find(std::begin(floatType), std::end(floatType), f) != std::end(floatType);
        bool isUnormType = std::find(std::begin(unormType), std::end(unormType), f) != std::end(unormType);
        bool isSnormType = std::find(std::begin(snormType), std::end(snormType), f) != std::end(snormType);
        bool isUIntType = std::find(std::begin(uintType), std::end(uintType), f) != std::end(uintType);
        bool isSIntType = std::find(std::begin(sintType), std::end(sintType), f) != std::end(sintType);

        // Format should only appear in at most one of these data type lists
        assert(static_cast<int>(isFloatType) + static_cast<int>(isUnormType) + static_cast<int>(isSnormType) + static_cast<int>(isUIntType) + static_cast<int>(isSIntType) <= 1);

        if (!IsValid(static_cast<DXGI_FORMAT>(f)))
        {
            printe("ERROR: Isvalid failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsCompressed(static_cast<DXGI_FORMAT>(f)) != isCompressed)
        {
            printe("ERROR: IsCompressed failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsPacked(static_cast<DXGI_FORMAT>(f)) != isPAK)
        {
            printe("ERROR: IsPacked failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsSRGB(static_cast<DXGI_FORMAT>(f)) != isSRGB)
        {
            printe("ERROR: IsSRGB failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsBGR(static_cast<DXGI_FORMAT>(f)) != isBGR)
        {
            printe("ERROR: IsBGR failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if ((IsTypeless(static_cast<DXGI_FORMAT>(f)) != isTypeless)
            || (IsTypeless(static_cast<DXGI_FORMAT>(f), true) != isTypeless))
        {
            printe("ERROR: IsTypeless failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsTypeless(static_cast<DXGI_FORMAT>(f), false) != isPartialTypeless)
        {
            printe("ERROR: IsTypeless(false) failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (HasAlpha(static_cast<DXGI_FORMAT>(f)) != hasAlpha)
        {
            printe("ERROR: HasAlpha failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsVideo(static_cast<DXGI_FORMAT>(f)) != isVideo)
        {
            printe("ERROR: IsVideo failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsPlanar(static_cast<DXGI_FORMAT>(f)) != isPlanar)
        {
            printe("ERROR: IsPlanar failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsPalettized(static_cast<DXGI_FORMAT>(f)) != isPal)
        {
            printe("ERROR: IsPalettized failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (IsDepthStencil(static_cast<DXGI_FORMAT>(f)) != isDepthStencil)
        {
            printe("ERROR: IsDepthStencil failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (f >= 121 && f <= 129)
        {
            // Skip 'holes' in DXGI formats
            continue;
        }

        if ( f >= 133 && f <= 188 )
        {
            // Skip 'holes' in DXGI formats
            continue;
        }

        if (BitsPerPixel(static_cast<DXGI_FORMAT>(f)) == 0)
        {
            printe("ERROR: BitsPerPixel failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }

        if (isPal)
        {
            if (BitsPerColor(static_cast<DXGI_FORMAT>(f)) != 0)
            {
                printe("ERROR: BitsPerColor succeeded on DXGI Format %u when it should have returned 0 (%ls)\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
        }
        else
        {
            if (BitsPerColor(static_cast<DXGI_FORMAT>(f)) == 0)
            {
                printe("ERROR: BitsPerColor failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
        }

        if (isCompressed)
        {
            if (BytesPerBlock(static_cast<DXGI_FORMAT>(f)) == 0)
            {
                printe("ERROR: BytesPerBlock failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
        }
        else
        {
            if (BytesPerBlock(static_cast<DXGI_FORMAT>(f)) != 0)
            {
                printe("ERROR: BytesPerBlock succeeded on DXGI Format %u when it should have returned 0 (%ls)\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
        }

        switch (FormatDataType(static_cast<DXGI_FORMAT>(f)))
        {
        case FORMAT_TYPE_FLOAT:
            if (!isFloatType)
            {
                printe("ERROR: FormatDataType failed on DXGI Format %u (%ls) ... float\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
            break;

        case FORMAT_TYPE_UNORM:
            if (!isUnormType)
            {
                printe("ERROR: FormatDataType failed on DXGI Format %u (%ls) ... unorm\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
            break;

        case FORMAT_TYPE_SNORM:
            if (!isSnormType)
            {
                printe("ERROR: FormatDataType failed on DXGI Format %u (%ls) ... snorm\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
            break;

        case FORMAT_TYPE_UINT:
            if (!isUIntType)
            {
                printe("ERROR: FormatDataType failed on DXGI Format %u (%ls) ... uint\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
            break;

        case FORMAT_TYPE_SINT:
            if (!isSIntType)
            {
                printe("ERROR: FormatDataType failed on DXGI Format %u (%ls) ... sint\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
            break;

        default:
            if (isFloatType || isUnormType || isSnormType || isUIntType || isSIntType)
            {
                printe("ERROR: FormatDataType failed on DXGI Format %u (%ls) ... typeless\n", f, GetName(DXGI_FORMAT(f)));
                success = false;
            }
            break;
        }
    }

    // invalid args
    if ( IsValid( DXGI_FORMAT_UNKNOWN ) )
    {
        printe( "ERROR: IsValid failed on DXGI Unknown Format\n" );
        success = false;
    }

    if ( IsValid( DXGI_FORMAT(DXGI_END + 1) ) )
    {
        printe( "ERROR: IsValid failed on DXGI beyond end of range\n" );
        success = false;
    }

    if (IsCompressed(DXGI_FORMAT_UNKNOWN)
        || IsPacked(DXGI_FORMAT_UNKNOWN)
        || IsVideo(DXGI_FORMAT_UNKNOWN)
        || IsPlanar(DXGI_FORMAT_UNKNOWN)
        || IsPalettized(DXGI_FORMAT_UNKNOWN)
        || IsDepthStencil(DXGI_FORMAT_UNKNOWN)
        || IsSRGB(DXGI_FORMAT_UNKNOWN)
        || IsBGR(DXGI_FORMAT_UNKNOWN)
        || IsTypeless(DXGI_FORMAT_UNKNOWN)
        || BitsPerPixel(DXGI_FORMAT_UNKNOWN) != 0
        || BitsPerColor(DXGI_FORMAT_UNKNOWN) != 0
        || BytesPerBlock(DXGI_FORMAT_UNKNOWN) != 0
        || FormatDataType(DXGI_FORMAT_UNKNOWN) != FORMAT_TYPE_TYPELESS
        )
    {
        success = false;
        printe("Failed invalid DXGI arg test\n");
    }

    return success;
}


namespace
{
    //-------------------------------------------------------------------------------------
    // This function is used by DDSTextureLoader and ScreenGrab to do the same thing
    // as DirectXTex's ComputePitch, so we test it here...
    //
    // Also check against DirectX Tool Kit's LoaderHelpers.h
    //-------------------------------------------------------------------------------------
    HRESULT GetSurfaceInfo(
        _In_ size_t width,
        _In_ size_t height,
        _In_ DXGI_FORMAT fmt,
        _Out_opt_ size_t* outNumBytes,
        _Out_opt_ size_t* outRowBytes,
        _Out_opt_ size_t* outNumRows) noexcept
    {
        uint64_t numBytes = 0;
        uint64_t rowBytes = 0;
        uint64_t numRows = 0;

        bool bc = false;
        bool packed = false;
        bool planar = false;
        size_t bpe = 0;
        switch (static_cast<int>(fmt))
        {
        case DXGI_FORMAT_UNKNOWN:
            return E_INVALIDARG;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bc = true;
            bpe = 16;
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_YUY2:
            packed = true;
            bpe = 4;
            break;

        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            packed = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
            if ((height % 2) != 0)
            {
                // Requires a height alignment of 2.
                return E_INVALIDARG;
            }
            planar = true;
            bpe = 2;
            break;

        case WIN10_DXGI_FORMAT_P208:
            planar = true;
            bpe = 2;
            break;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            if ((height % 2) != 0)
            {
                // Requires a height alignment of 2.
                return E_INVALIDARG;
            }
            planar = true;
            bpe = 4;
            break;

        case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
        case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
        case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
            planar = true;
            bpe = 4;
            break;

        default:
            break;
        }

        if (bc)
        {
            uint64_t numBlocksWide = 0;
            if (width > 0)
            {
                numBlocksWide = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
            }
            uint64_t numBlocksHigh = 0;
            if (height > 0)
            {
                numBlocksHigh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
            }
            rowBytes = numBlocksWide * bpe;
            numRows = numBlocksHigh;
            numBytes = rowBytes * numBlocksHigh;
        }
        else if (packed)
        {
            rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
            numRows = uint64_t(height);
            numBytes = rowBytes * height;
        }
        else if (fmt == DXGI_FORMAT_NV11)
        {
            rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
            numRows = uint64_t(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
            numBytes = rowBytes * numRows;
        }
        else if (planar)
        {
            rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
            numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
            numRows = height + ((uint64_t(height) + 1u) >> 1);
        }
        else
        {
            const size_t bpp = BitsPerPixel(fmt);
            if (!bpp)
                return E_INVALIDARG;

            rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
            numRows = uint64_t(height);
            numBytes = rowBytes * height;
        }

    #if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
        static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
        if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
            return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    #else
        static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
    #endif

        if (outNumBytes)
        {
            *outNumBytes = static_cast<size_t>(numBytes);
        }
        if (outRowBytes)
        {
            *outRowBytes = static_cast<size_t>(rowBytes);
        }
        if (outNumRows)
        {
            *outNumRows = static_cast<size_t>(numRows);
        }

        return S_OK;
    }
}

//-------------------------------------------------------------------------------------
// ComputePitch
// ComputeScanlines
bool TEXTest::Test02()
{
    size_t rowPitch = 0;
    size_t slicePitch = 0;

    bool success = true;

    // DXGI_FORMAT_B5G6R5_UNORM
    HRESULT hr = ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 2 || slicePitch != 12 )
    {
        printe("ERROR: CP RGB565 1x1 failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_LEGACY_DWORD );
    if ( FAILED(hr) ||rowPitch != 4 || slicePitch != 24 )
    {
        printe("ERROR: CP RGB565 1x1 DWORD failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_PARAGRAPH );
    if ( FAILED(hr) ||rowPitch != 16 || slicePitch != 96 )
    {
        printe("ERROR: CP RGB565 1x1 PARAGRAPH failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_YMM );
    if ( FAILED(hr) ||rowPitch != 32 || slicePitch != 192 )
    {
        printe("ERROR: CP RGB565 1x1 YMM failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_ZMM );
    if ( FAILED(hr) ||rowPitch != 64 || slicePitch != 384 )
    {
        printe("ERROR: CP RGB565 1x1 ZMM failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_PAGE4K );
    if ( FAILED(hr) ||rowPitch != 4096 || slicePitch != 24576 )
    {
        printe("ERROR: CP RGB565 1x1 PAGE4K failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    size_t rows = ComputeScanlines(DXGI_FORMAT_B5G6R5_UNORM, 6);
    if (rows != 6)
    {
        printe("ERROR: CS RGB565 6 failed rows %zu\n", rows );
        success = false;
    }

    // DXGI_FORMAT_R8G8B8A8_UNORM
    hr = ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_NONE );
    if ( FAILED(hr) ||rowPitch != 8 || slicePitch != 8 )
    {
        printe("ERROR: CP R8G8B8A8_UNORM failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_24BPP );
    if ( FAILED(hr) ||rowPitch != 6 || slicePitch != 6 )
    {
        printe("ERROR: CP 24BPP failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_16BPP );
    if ( FAILED(hr) ||rowPitch != 4 || slicePitch != 4 )
    {
        printe("ERROR: CP 16BPP failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_8BPP );
    if ( FAILED(hr) ||rowPitch != 2 || slicePitch != 2 )
    {
        printe("ERROR: CP 8PP failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    rows = ComputeScanlines(DXGI_FORMAT_R8G8B8A8_UNORM, 1);
    if (rows != 1)
    {
        printe("ERROR: CS R8G8B8A8_UNORM failed rows %zu\n", rows );
        success = false;
    }

    // XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT
    hr = ComputePitch( XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT, 2, 1, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 8 || slicePitch != 8 )
    {
        printe("ERROR: R10G10B10_7E3_A2_FLOAT [Xbox] A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT, 128, 64, rowPitch, slicePitch );
    if ( FAILED(hr) ||rowPitch != 512 || slicePitch != 32768 )
    {
        printe("ERROR: R10G10B10_7E3_A2_FLOAT [Xbox] B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    rows = ComputeScanlines(XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT, 64);
    if (rows != 64)
    {
        printe("ERROR: CS R10G10B10_7E3_A2_FLOAT [Xbox] failed rows %zu\n", rows );
        success = false;
    }

    // XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT
    hr = ComputePitch( XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT, 2, 1, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 8 || slicePitch != 8 )
    {
        printe("ERROR: R10G10B10_6E4_A2_FLOAT [Xbox] A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT, 128, 64, rowPitch, slicePitch );
    if ( FAILED(hr) ||rowPitch != 512 || slicePitch != 32768 )
    {
        printe("ERROR: R10G10B10_6E4_A2_FLOAT [Xbox] B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    rows = ComputeScanlines(XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT, 64);
    if (rows != 64)
    {
        printe("ERROR: CS R10G10B10_6E4_A2_FLOAT [Xbox] failed rows %zu\n", rows );
        success = false;
    }

    for( unsigned int j = XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT; j <= XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT; ++j )
    {
        hr = ComputePitch( DXGI_FORMAT(j), 4, 2, rowPitch, slicePitch );
        if ( FAILED(hr) || rowPitch != 8 || slicePitch != 24 )
        {
            printe("ERROR: CP %ls A failed rowPitch %zu, slicePitch %zu (%08X)\n", GetName( DXGI_FORMAT(j) ), rowPitch, slicePitch, static_cast<unsigned int>(hr) );
            success = false;
        }

        hr = ComputePitch( DXGI_FORMAT(j), 128, 64, rowPitch, slicePitch );
        if ( FAILED(hr) ||rowPitch != 256 || slicePitch != 24576 )
        {
            printe("ERROR: CP %ls B failed rowPitch %zu, slicePitch %zu (%08X)\n", GetName( DXGI_FORMAT(j) ), rowPitch, slicePitch, static_cast<unsigned int>(hr) );
            success = false;
        }

        size_t numBytes = 0;
        size_t rowBytes = 0;
        size_t numRows = 0;
        hr = GetSurfaceInfo( 4, 2, DXGI_FORMAT(j), &numBytes, &rowBytes, &numRows );
        if ( FAILED(hr) || rowBytes != 8 || numBytes != 24 || numRows != 3 )
        {
            printe("ERROR: CP %ls A failed rowBytes %zu, numBytes %zu, numRows %zu (%08X)\n", GetName( DXGI_FORMAT(j) ), rowBytes, numBytes, numRows, static_cast<unsigned int>(hr) );
            success = false;
        }

        rows = ComputeScanlines(DXGI_FORMAT(j), 2);
        if ( rows != 3 )
        {
            printe("ERROR: CS %ls A failed rows %zu\n", GetName( DXGI_FORMAT(j) ), rows );
            success = false;
        }

        hr = GetSurfaceInfo( 128, 64, DXGI_FORMAT(j), &numBytes, &rowBytes, &numRows );
        if ( FAILED(hr) || rowBytes != 256 || numBytes != 24576 || numRows != (64+32) )
        {
            printe("ERROR: CP %ls B failed rowBytes %zu, numBytes %zu, numRows %zu (%08X)\n", GetName( DXGI_FORMAT(j) ), rowBytes, numBytes, numRows, static_cast<unsigned int>(hr) );
            success = false;
        }

        rows = ComputeScanlines(DXGI_FORMAT(j), 64);
        if ( rows != 96 )
        {
            printe("ERROR: CS %ls B failed rows %zu\n", GetName( DXGI_FORMAT(j) ), rows );
            success = false;
        }
    }

    // DXGI_FORMAT_P208
    hr = ComputePitch( WIN10_DXGI_FORMAT_P208, 2, 1, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 2 || slicePitch != 4 )
    {
        printe("ERROR: JPEG HW P208: CP A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( WIN10_DXGI_FORMAT_P208, 128, 64, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 128 || slicePitch != 16384 )
    {
        printe("ERROR: JPEG HW P208: CP B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( WIN10_DXGI_FORMAT_P208, 128, 65, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 128 || slicePitch != 16640 )
    {
        printe("ERROR: JPEG HW P208: CP C failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    rows = ComputeScanlines(WIN10_DXGI_FORMAT_P208, 64);
    if (rows != 128)
    {
        printe("ERROR: CS JPEG HW P208 failed rows %zu\n", rows );
        success = false;
    }

    // DXGI_FORMAT_V208
    hr = ComputePitch(WIN10_DXGI_FORMAT_V208, 2, 1, rowPitch, slicePitch);
    if (SUCCEEDED(hr) || hr != E_INVALIDARG)
    {
        printe("ERROR: JPEG HW V208: CP should have failed on a height of 1!\n");
        success = false;
    }

    hr = ComputePitch( WIN10_DXGI_FORMAT_V208, 2, 2, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 2 || slicePitch != 8 )
    {
        printe("ERROR: JPEG HW V208: CP A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( WIN10_DXGI_FORMAT_V208, 128, 64, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 128 || slicePitch != 16384 )
    {
        printe("ERROR: JPEG HW V208: CP B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    rows = ComputeScanlines(WIN10_DXGI_FORMAT_V208, 64);
    if (rows != 128)
    {
        printe("ERROR: CS JPEG HW V208 failed rows %zu\n", rows );
        success = false;
    }

    // DXGI_FORMAT_V408
    hr = ComputePitch( WIN10_DXGI_FORMAT_V408, 2, 1, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 2 || slicePitch != 2 )
    {
        printe("ERROR: JPEG HW V408: CP A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( WIN10_DXGI_FORMAT_V408, 128, 64, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 128 || slicePitch != 24576 )
    {
        printe("ERROR: JPEG HW V408: CP B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    hr = ComputePitch( WIN10_DXGI_FORMAT_V408, 128, 65, rowPitch, slicePitch );
    if ( FAILED(hr) || rowPitch != 128 || slicePitch != 24704 )
    {
        printe("ERROR: JPEG HW V408: CP C failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
        success = false;
    }

    rows = ComputeScanlines(WIN10_DXGI_FORMAT_V408, 64);
    if (rows != 192)
    {
        printe("ERROR: CS JPEG HW V408 failed rows %zu\n", rows );
        success = false;
    }

    // DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM
    {
        hr = ComputePitch( XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM, 2, 1, rowPitch, slicePitch );
        if ( FAILED(hr) || rowPitch != 8 || slicePitch != 8 )
        {
            printe("ERROR: R10G10B10_SNORM_A2_UNORM [Xbox] CP A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
            success = false;
        }

        hr = ComputePitch( XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM, 128, 64, rowPitch, slicePitch );
        if ( FAILED(hr) || rowPitch != 512 || slicePitch != 32768 )
        {
            printe("ERROR: R10G10B10_SNORM_A2_UNORM [Xbox] CP B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
            success = false;
        }

        size_t numBytes = 0;
        size_t rowBytes = 0;
        size_t numRows = 0;
        hr = GetSurfaceInfo( 4, 2, XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM, &numBytes, &rowBytes, &numRows );
        if ( FAILED(hr) || rowBytes != 16 || numBytes != 32 || numRows != 2 )
        {
            printe("ERROR: R10G10B10_SNORM_A2_UNORM [Xbox] CP A failed rowBytes %zu, numBytes %zu, numRows %zu (%08X)\n", rowBytes, numBytes, numRows, static_cast<unsigned int>(hr) );
            success = false;
        }

        hr = GetSurfaceInfo( 128, 64, XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM, &numBytes, &rowBytes, &numRows );
        if ( FAILED(hr) || rowBytes != 512 || numBytes != 32768 || numRows != 64 )
        {
            printe("ERROR: R10G10B10_SNORM_A2_UNORM [Xbox] CP B failed rowBytes %zu, numBytes %zu, numRows %zu (%08X)\n", rowBytes, numBytes, numRows, static_cast<unsigned int>(hr) );
            success = false;
        }

        rows = ComputeScanlines(XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM, 64);
        if (rows != 64)
        {
            printe("ERROR: CS R10G10B10_SNORM_A2_UNORM [Xbox] failed rows %zu\n", rows );
            success = false;
        }
    }

    // DXGI_FORMAT_R4G4_UNORM
    {
        hr = ComputePitch( XBOX_DXGI_FORMAT_R4G4_UNORM, 2, 1, rowPitch, slicePitch );
        if ( FAILED(hr) || rowPitch != 2 || slicePitch != 2 )
        {
            printe("ERROR: R4G4_UNORM [Xbox] CP A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
            success = false;
        }

        hr = ComputePitch( XBOX_DXGI_FORMAT_R4G4_UNORM, 128, 64, rowPitch, slicePitch );
        if ( FAILED(hr) || rowPitch != 128 || slicePitch != 8192 )
        {
            printe("ERROR: R4G4_UNORM [Xbox] CP B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr) );
            success = false;
        }

        size_t numBytes = 0;
        size_t rowBytes = 0;
        size_t numRows = 0;
        hr = GetSurfaceInfo( 4, 2, XBOX_DXGI_FORMAT_R4G4_UNORM, &numBytes, &rowBytes, &numRows );
        if ( FAILED(hr) || rowBytes != 4 || numBytes != 8 || numRows != 2 )
        {
            printe("ERROR: R4G4_UNORM [Xbox] CP A failed rowBytes %zu, numBytes %zu, numRows %zu (%08X)\n", rowBytes, numBytes, numRows, static_cast<unsigned int>(hr) );
            success = false;
        }

        hr = GetSurfaceInfo( 128, 64, XBOX_DXGI_FORMAT_R4G4_UNORM, &numBytes, &rowBytes, &numRows );
        if ( FAILED(hr) || rowBytes != 128 || numBytes != 8192 || numRows != 64 )
        {
            printe("ERROR: R4G4_UNORM [Xbox] CP B failed rowBytes %zu, numBytes %zu, numRows %zu (%08X)\n", rowBytes, numBytes, numRows, static_cast<unsigned int>(hr) );
            success = false;
        }

        rows = ComputeScanlines(XBOX_DXGI_FORMAT_R4G4_UNORM, 64);
        if (rows != 64)
        {
            printe("ERROR: CS R4G4_UNORM [Xbox] failed rows %zu\n", rows );
            success = false;
        }
    }

    // DXGI_FORMAT_NV12
    hr = ComputePitch(DXGI_FORMAT_NV12, 2, 1, rowPitch, slicePitch);
    if (SUCCEEDED(hr) || hr != E_INVALIDARG)
    {
        printe("ERROR: NV12: CP should have failed on a height of 1!\n");
        success = false;
    }

    hr = ComputePitch(DXGI_FORMAT_NV12, 2, 2, rowPitch, slicePitch);
    if (FAILED(hr) || rowPitch != 2 || slicePitch != 6)
    {
        printe("ERROR: NV12: CP A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputePitch(DXGI_FORMAT_NV12, 128, 64, rowPitch, slicePitch);
    if (FAILED(hr) || rowPitch != 128 || slicePitch != 12288)
    {
        printe("ERROR: NV12: CP B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr));
        success = false;
    }

    rows = ComputeScanlines(DXGI_FORMAT_NV12, 64);
    if (rows != 96)
    {
        printe("ERROR: NV12 failed rows %zu\n", rows);
        success = false;
    }

    // DXGI_FORMAT_P010
    hr = ComputePitch(DXGI_FORMAT_P010, 2, 1, rowPitch, slicePitch);
    if (SUCCEEDED(hr) || hr != E_INVALIDARG)
    {
        printe("ERROR: P010: CP should have failed on a height of 1!\n");
        success = false;
    }

    hr = ComputePitch(DXGI_FORMAT_P010, 2, 2, rowPitch, slicePitch);
    if (FAILED(hr) || rowPitch != 4 || slicePitch != 12)
    {
        printe("ERROR: P010: CP A failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputePitch(DXGI_FORMAT_P010, 128, 64, rowPitch, slicePitch);
    if (FAILED(hr) || rowPitch != 256 || slicePitch != 24576)
    {
        printe("ERROR: P010: CP B failed rowPitch %zu, slicePitch %zu (%08X)\n", rowPitch, slicePitch, static_cast<unsigned int>(hr));
        success = false;
    }

    rows = ComputeScanlines(DXGI_FORMAT_P010, 64);
    if (rows != 96)
    {
        printe("ERROR: P010 failed rows %zu\n", rows);
        success = false;
    }

    // invalid/zero arg
    hr = GetSurfaceInfo(128, 256, DXGI_FORMAT_UNKNOWN, nullptr, nullptr, nullptr);
    if (hr != E_INVALIDARG)
    {
        success = false;
        printe("GetSurfaceInfo: Failed invalid arg test\n");
    }

    hr = GetSurfaceInfo(128, 256, DXGI_FORMAT_R8G8B8A8_UNORM, nullptr, nullptr, nullptr);
    if (FAILED(hr))
    {
        success = false;
        printe("GetSurfaceInfo: Failed nullptr arg test\n");
    }

    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;
    hr = GetSurfaceInfo(0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, &numBytes, &rowBytes, &numRows);
    if (FAILED(hr) || rowBytes != 0 || numBytes != 0 || numRows != 0)
    {
        success = false;
        printe("GetSurfaceInfo: Failed zero arg test\n");
    }

    hr = ComputePitch(DXGI_FORMAT_UNKNOWN, 128, 64, rowPitch, slicePitch);
    if (hr != E_INVALIDARG)
    {
        success = false;
        printe("ComputePitch: Failed invalid arg test\n");
    }

    hr = ComputePitch(DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0, rowPitch, slicePitch);
    if (FAILED(hr) || rowPitch != 0|| slicePitch != 0)
    {
        success = false;
        printe("ComputePitch: Failed zero arg test\n");
    }

    rows = ComputeScanlines(DXGI_FORMAT_UNKNOWN, 256);
    if (rows != 0)
    {
        success = false;
        printe("ComputeScanlines: Failed invalid arg test\n");
    }

    rows = ComputeScanlines(DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (rows != 0)
    {
        success = false;
        printe("ComputeScanlines: Failed zero arg test\n");
    }

    //
    // Here we are comparing the DirectX Runtime vs. DirectXTex, so we only need a nullptr device.
    //
    ComPtr<ID3D11Device> device;
    D3D_FEATURE_LEVEL lvl;
    ComPtr<ID3D11DeviceContext> context;
    UINT createDeviceFlags = 0;
#ifdef D3D_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    hr = D3D11CreateDevice( nullptr,
                            D3D_DRIVER_TYPE_NULL,
                            nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, device.GetAddressOf(), &lvl, context.GetAddressOf() );
    if ( FAILED(hr) )
    {
        printe( "Failed creating Direct3D nullptr device (HRESULT %08X)\n", static_cast<unsigned int>(hr) );
        return false;
    }

#ifdef D3D_DEBUG
    //
    // Also want to ignore some common debug messages we don't care about...
    //
    ComPtr<ID3D11Debug> debugDevice;
    if ( SUCCEEDED( device.As( &debugDevice ) ) )
    {
        ComPtr<ID3D11InfoQueue> infoQ;
        if ( SUCCEEDED( debugDevice.As( &infoQ ) ) )
        {
            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS, D3D11_MESSAGE_ID_LIVE_DEVICE, D3D11_MESSAGE_ID_LIVE_OBJECT_SUMMARY,
                D3D11_MESSAGE_ID_CREATETEXTURE2D_OUTOFMEMORY_RETURN
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
            filter.DenyList.pIDList = hide;
            infoQ->AddStorageFilterEntries( &filter );
        }
    }
#endif

    size_t ncount = 0;
    size_t npass = 0;

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        switch ( static_cast<int>(f) )
        {
        case DXGI_FORMAT_R1_UNORM:
            continue;

        case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
        case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
        case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
        case XBOX_DXGI_FORMAT_R4G4_UNORM:
            // Only supported for Xbox One platform
            continue;

        case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
        case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
        case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
            // Special use case formats for Xbox One platform
            continue;

        case WIN10_DXGI_FORMAT_P208:
        case WIN10_DXGI_FORMAT_V208:
        case WIN10_DXGI_FORMAT_V408:
            // JPEG hardware decode formats
            continue;

        case WIN11_DXGI_FORMAT_A4B4G4R4_UNORM:
            // D3D11On12-only format
            continue;
        }

        if (f >= 121 && f <= 129)
        {
            // Skip 'holes' in DXGI formats
            continue;
        }

        if ( f >= 133 && f <= 188 )
        {
            // Skip 'holes' in DXGI formats
            continue;
        }

        static const UINT vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 32, 64, 100, 128, 131, 132, 200, 256, 768, 1024, 2048, 4096, 8192, 16384 };

        for(UINT i = 0; i < std::size(vals); ++i )
        {
            for(UINT j = 0; j < std::size(vals); ++j )
            {
                if ( IsCompressed( DXGI_FORMAT(f) )
                     && ( ( vals[i] % 4 ) != 0 || ( vals[j] % 4 ) != 0 ) )
                {
                    // Base level of BC must be multiple of 4
                    continue;
                }

                if ( IsPacked( DXGI_FORMAT(f) )
                     && ( vals[i] & 1 ) != 0 )
                {
                    // Packed formats require even width
                    continue;
                }
                else if ( f == DXGI_FORMAT_NV11 )
                {
                    if ( ( vals[i] & 3 ) != 0  )
                    {
                        // NV11 requires mulitple of 4 width
                        continue;
                    }
                }
                else if ( IsPlanar( DXGI_FORMAT(f) )
                          && ( ( vals[i] & 1 ) != 0 || ( vals[j] & 1 ) != 0 ) )
                {
                    // Planar formats require even width and height
                    continue;
                }

                CD3D11_TEXTURE2D_DESC desc( ( f == DXGI_FORMAT_420_OPAQUE ) ? DXGI_FORMAT_NV12 : DXGI_FORMAT(f), vals[i], vals[j], 1, 1, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_WRITE );

                ComPtr<ID3D11Texture2D> tex;
                hr = device->CreateTexture2D( &desc, nullptr, tex.GetAddressOf() );
                if ( ( hr == E_OUTOFMEMORY ) && ( vals[i] >= 8192 || vals[j] >= 8192 ) )
                {
                    continue;
                }

                if ( hr == E_INVALIDARG )
                {
                    switch( f )
                    {
                    case DXGI_FORMAT_B5G6R5_UNORM:
                    case DXGI_FORMAT_B5G5R5A1_UNORM:
                    case DXGI_FORMAT_AYUV:
                    case DXGI_FORMAT_Y410:
                    case DXGI_FORMAT_Y416:
                    case DXGI_FORMAT_NV12:
                    case DXGI_FORMAT_P010:
                    case DXGI_FORMAT_P016:
                    case DXGI_FORMAT_420_OPAQUE:
                    case DXGI_FORMAT_YUY2:
                    case DXGI_FORMAT_Y210:
                    case DXGI_FORMAT_Y216:
                    case DXGI_FORMAT_NV11:
                    case DXGI_FORMAT_AI44:
                    case DXGI_FORMAT_IA44:
                    case DXGI_FORMAT_P8:
                    case DXGI_FORMAT_A8P8:
                    case DXGI_FORMAT_B4G4R4A4_UNORM:
                        // Down-level doesn't support these even with KB 2670838 installed
                        continue;
                    }
                }

                ++ncount;

                if ( FAILED(hr) )
                {
                    printe("ERROR: Failed creating texture for format %ls (%u), %u by %u (%08X)\n", GetName( DXGI_FORMAT(f) ), f, vals[i], vals[j], static_cast<unsigned int>(hr) );
                    success = false;
                    continue;
                }

                D3D11_MAPPED_SUBRESOURCE res = {};
                hr = context->Map( tex.Get(), 0, D3D11_MAP_WRITE, 0, &res );
                if ( FAILED(hr) )
                {
                    printe("ERROR: Failed mapping texture for format %ls (%u), %u by %u (%08X)\n", GetName( DXGI_FORMAT(f) ), f, vals[i], vals[j], static_cast<unsigned int>(hr) );
                    success = false;
                    continue;
                }

                hr = ComputePitch( DXGI_FORMAT(f), vals[i], vals[j], rowPitch, slicePitch, CP_FLAGS_NONE );
                if ( FAILED(hr) || rowPitch != res.RowPitch || slicePitch != res.DepthPitch )
                {
                    printe("ERROR: %ls (%u) failed rowPitch %zu, slicePitch %zu (%u by %u)...check rowPitch %u slicePitch %u (%08X)\n",
                           GetName( DXGI_FORMAT(f) ), f, rowPitch, slicePitch, vals[i], vals[j], res.RowPitch, res.DepthPitch, static_cast<unsigned int>(hr) );
                    success = false;
                }
                else
                {
                    UINT targetRows = vals[j];
                    if ( f == DXGI_FORMAT_NV11 )
                    {
                        targetRows = vals[j] * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
                    }
                    else if ( f == XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT
                              || f == XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS
                              || f == XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT )
                    {
                        targetRows = 0;
                    }
                    else if ( IsPlanar( DXGI_FORMAT(f) ) )
                    {
                        targetRows = vals[j] + ( (vals[j] + 1 ) >> 1 );
                    }
                    else if ( IsCompressed( DXGI_FORMAT(f) ) )
                    {
                        targetRows = std::max<UINT>( 1, (vals[j] + 3) / 4 );
                    }

                    size_t rowCount = ComputeScanlines( DXGI_FORMAT(f), vals[j] );
                    if ( rowCount != targetRows )
                    {
                        printe("ERROR: %ls (%u) failed scanlines %zu (%u)...check scanlines %u\n",
                               GetName( DXGI_FORMAT(f) ), f, rowCount, vals[j], targetRows );
                        success = false;
                    }
                    else
                    {
                        numBytes = 0;
                        rowBytes = 0;
                        numRows = 0;
                        hr = GetSurfaceInfo( vals[i], vals[j], DXGI_FORMAT(f), &numBytes, &rowBytes, &numRows );

                        if ( FAILED(hr) || rowBytes != res.RowPitch || numBytes != res.DepthPitch || numRows != targetRows )
                        {
                            printe("ERROR: %ls (%u) failed rowBytes %zu, numBytes %zu, numRows %zu (%u by %u)...check rowBytes %u numBytes %u numRows %u (%08X)\n",
                                   GetName( DXGI_FORMAT(f) ), f, rowBytes, numBytes, numRows, vals[i], vals[j],
                                   res.RowPitch, res.DepthPitch, targetRows, static_cast<unsigned int>(hr) );
                            success = false;
                        }
                        else
                        {
                            ++npass;
                        }
                    }
                }
            }
        }
    }

    print("%zu formats/sizes tested vs. DX, %zu passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// MakeSRGB
// MakeLinear
// MakeTypeless
// MakeTypelessUNORM
// MakeTypelessFLOAT
bool TEXTest::Test12()
{
    bool success = true;

    //--- MakeSRGB
    static const DXGI_FORMAT hasSRGB[] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8X8_UNORM,
        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC7_UNORM,
    };

    for( size_t i = 0; i < std::size(hasSRGB); ++i )
    {
        DXGI_FORMAT f = MakeSRGB( hasSRGB[i] );

        if ( !IsSRGB( f ) )
        {
            DXGI_FORMAT fmt = hasSRGB[i];
            printe("ERROR: MakeSRGB failed on DXGI Format %u (%ls)\n", static_cast<unsigned int>(fmt), GetName(fmt));
            success = false;
        }
    }

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        size_t i = 0;
        for( ; i < std::size(hasSRGB); ++i )
        {
            if ( hasSRGB[i] == static_cast<DXGI_FORMAT>(f) )
                break;
        }

        if ( i < std::size(hasSRGB) )
            continue;

        if ( MakeSRGB( static_cast<DXGI_FORMAT>(f) ) != static_cast<DXGI_FORMAT>(f) )
        {
            printe( "ERROR: MakeSRGB failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }
    }

    if (MakeSRGB(DXGI_FORMAT_UNKNOWN) != DXGI_FORMAT_UNKNOWN)
    {
        printe("MakeSRGB: Failed invalid arg test\n");
    }

    //--- MakeLinear
    static const DXGI_FORMAT isSRGB[] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
        DXGI_FORMAT_BC1_UNORM_SRGB, DXGI_FORMAT_BC2_UNORM_SRGB, DXGI_FORMAT_BC3_UNORM_SRGB, DXGI_FORMAT_BC7_UNORM_SRGB,
    };

    for (size_t i = 0; i < std::size(isSRGB); ++i)
    {
        DXGI_FORMAT f = MakeLinear(isSRGB[i]);

        if (IsSRGB(f))
        {
            DXGI_FORMAT fmt = isSRGB[i];
            printe("ERROR: MakeLinear failed on DXGI Format %u (%ls)\n", static_cast<unsigned int>(fmt), GetName(fmt));
            success = false;
        }
    }

    for (UINT f = DXGI_START; f <= DXGI_END; ++f)
    {
        size_t i = 0;
        for (; i < std::size(isSRGB); ++i)
        {
            if (isSRGB[i] == static_cast<DXGI_FORMAT>(f))
                break;
        }

        if (i < std::size(isSRGB))
            continue;

        if (MakeLinear(static_cast<DXGI_FORMAT>(f)) != static_cast<DXGI_FORMAT>(f))
        {
            printe("ERROR: MakeLinear failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }
    }

    if (MakeLinear(DXGI_FORMAT_UNKNOWN) != DXGI_FORMAT_UNKNOWN)
    {
        printe("MakeLinear: Failed invalid arg test\n");
    }

    //--- MakeTypeless
    static const DXGI_FORMAT hasTypeless[] =
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32_SINT,
        DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_SINT,
        DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_SINT,
        DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UINT,
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R8G8B8A8_SINT,
        DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_SINT,
        DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_SINT,
        DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_R8G8_SNORM, DXGI_FORMAT_R8G8_SINT,
        DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_SINT,
        DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_SNORM, DXGI_FORMAT_R8_SINT,
        DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB,
        DXGI_FORMAT_BC4_UNORM, DXGI_FORMAT_BC4_SNORM, DXGI_FORMAT_BC5_UNORM, DXGI_FORMAT_BC5_SNORM,
        DXGI_FORMAT_BC6H_UF16, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM, DXGI_FORMAT_BC7_UNORM_SRGB,
        XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT,
        XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT,
        XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM,
        XBOX_DXGI_FORMAT_R4G4_UNORM,
    };

    for( size_t i = 0; i < std::size(hasTypeless); ++i )
    {
        DXGI_FORMAT f = MakeTypeless( hasTypeless[i] );

        if ( !IsTypeless( f ) )
        {
            DXGI_FORMAT fmt = hasTypeless[i];
            printe("ERROR: MakeTypeless failed on DXGI Format %u (%ls)\n", static_cast<unsigned int>(fmt), GetName(fmt));
            success = false;
        }
    }

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        size_t i = 0;
        for( ; i < std::size(hasTypeless); ++i )
        {
            if ( hasTypeless[i] == static_cast<DXGI_FORMAT>(f) )
                break;
        }

        if ( i < std::size(hasTypeless) )
            continue;

        if ( MakeTypeless( static_cast<DXGI_FORMAT>(f) ) != static_cast<DXGI_FORMAT>(f) )
        {
            printe("ERROR: MakeTypeless failed on DXGI Format %u (%ls)\n", f, GetName(DXGI_FORMAT(f)));
            success = false;
        }
    }

    if (MakeTypeless(DXGI_FORMAT_UNKNOWN) != DXGI_FORMAT_UNKNOWN)
    {
        printe("MakeTypeless: Failed invalid arg test\n");
    }

    //--- MakeTypelessUNORM
    //--- MakeTypelessFLOAT
    static const DXGI_FORMAT typeless[] =
    {
        DXGI_FORMAT_R32G32B32A32_TYPELESS, DXGI_FORMAT_R32G32B32_TYPELESS, DXGI_FORMAT_R16G16B16A16_TYPELESS, DXGI_FORMAT_R32G32_TYPELESS,
        DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, DXGI_FORMAT_R10G10B10A2_TYPELESS,
        DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R16G16_TYPELESS, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,         DXGI_FORMAT_X24_TYPELESS_G8_UINT, DXGI_FORMAT_R8G8_TYPELESS, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R8_TYPELESS,
        DXGI_FORMAT_BC1_TYPELESS, DXGI_FORMAT_BC2_TYPELESS, DXGI_FORMAT_BC3_TYPELESS, DXGI_FORMAT_BC4_TYPELESS, DXGI_FORMAT_BC5_TYPELESS,
        DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8X8_TYPELESS, DXGI_FORMAT_BC6H_TYPELESS, DXGI_FORMAT_BC7_TYPELESS
    };

    for( UINT t = 0; t < std::size(typeless); ++t )
    {
        DXGI_FORMAT fu = MakeTypelessUNORM( typeless[t] );
        if ( IsTypeless(fu) )
        {
            switch( fu )
            {
            case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            case DXGI_FORMAT_R32G32B32_TYPELESS:
            case DXGI_FORMAT_R32G32_TYPELESS:
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            case DXGI_FORMAT_BC6H_TYPELESS:
                // Know format that doesn't have a UNORM equiv
                break;

            default:
                {
                    DXGI_FORMAT fmt = typeless[t];
                    printe("ERROR: MakeTypelessUNORM failed on DXGI Format %d (%ls)\n", fmt, GetName(fmt));
                }
                success = false;
            }
        }
        else
        {
            if ( BitsPerPixel( typeless[t] ) != BitsPerPixel( fu ) )
            {
                printe( "ERROR: MakeTypelessUNORM %d -> %d changes bpp\n", typeless[t], fu );
                success = false;
            }
        }

        if (MakeTypelessUNORM(DXGI_FORMAT_UNKNOWN) != DXGI_FORMAT_UNKNOWN)
        {
            printe("MakeTypelessUNORM: Failed invalid arg test\n");
        }

        DXGI_FORMAT ff = MakeTypelessFLOAT( typeless[t] );
        if ( IsTypeless(ff) )
        {
            switch( ff )
            {
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            case DXGI_FORMAT_R8G8_TYPELESS:
            case DXGI_FORMAT_R8_TYPELESS:
            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC7_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
                // Know format that doesn't have a FLOAT equiv
                break;

            default:
                {
                    DXGI_FORMAT fmt = typeless[t];
                    printe("ERROR: MakeTypelessFLOAT failed on DXGI Format %d (%ls)\n", fmt, GetName(fmt));
                }
                success = false;
            }
        }
        else
        {
            if ( BitsPerPixel( typeless[t] ) != BitsPerPixel( ff ) )
            {
                printe( "ERROR: MakeTypelessFLOAT %d -> %d changes bpp\n", typeless[t], fu );
                success = false;
            }
        }

        if (MakeTypelessFLOAT(DXGI_FORMAT_UNKNOWN) != DXGI_FORMAT_UNKNOWN)
        {
            printe("MakeTypelessFLOAT: Failed invalid arg test\n");
        }

    }

    return success;
}


//-------------------------------------------------------------------------------------
// ComputeTileShape
bool TEXTest::Test20()
{
    bool success = true;

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        const size_t bpp = BitsPerPixel(static_cast<DXGI_FORMAT>(f));
        if (!bpp || bpp == 1 || bpp == 24 || bpp == 96)
            continue;

        if (IsVideo(static_cast<DXGI_FORMAT>(f)) || IsPacked(static_cast<DXGI_FORMAT>(f)))
            continue;

        if (!IsCompressed(static_cast<DXGI_FORMAT>(f)))
        {
            TileShape shape = {};
            HRESULT hr = ComputeTileShape(static_cast<DXGI_FORMAT>(f), TEX_DIMENSION_TEXTURE1D, shape);
            if (FAILED(hr))
            {
                printe("ERROR: Failed calling ComputeTileShape on DXGI Format %u (%ls) (%08X) for 1D\n", f, GetName(DXGI_FORMAT(f)), static_cast<unsigned int>(hr));
                success = false;
            }
            else if (shape.width == 0 || shape.height != 1 || shape.depth != 1)
            {
                printe("ERROR: Expected h=1 d=1 for tile shape on DXGI Format %u (%ls) (%08X) for 1D\n", f, GetName(DXGI_FORMAT(f)), static_cast<unsigned int>(hr));
                success = false;
            }
        }

        TileShape shape = {};
        HRESULT hr = ComputeTileShape(static_cast<DXGI_FORMAT>(f), TEX_DIMENSION_TEXTURE2D, shape);
        if (FAILED(hr))
        {
            printe("ERROR: Failed calling ComputeTileShape on DXGI Format %u (%ls) (%08X) for 2D\n", f, GetName(DXGI_FORMAT(f)), static_cast<unsigned int>(hr));
            success = false;
        }
        else if (shape.width == 0 || shape.height == 0 || shape.depth != 1)
        {
            printe("ERROR: Expected d=1 for tile shape on DXGI Format %u (%ls) (%08X) for 2D\n", f, GetName(DXGI_FORMAT(f)), static_cast<unsigned int>(hr));
            success = false;
        }

        shape = {};
        hr = ComputeTileShape(static_cast<DXGI_FORMAT>(f), TEX_DIMENSION_TEXTURE3D, shape);
        if (FAILED(hr))
        {
            printe("ERROR: Failed calling ComputeTileShape on DXGI Format %u (%ls) (%08X) for 3D\n", f, GetName(DXGI_FORMAT(f)), static_cast<unsigned int>(hr));
            success = false;
        }
        else if (shape.width == 0 || shape.height == 0 || shape.depth == 0)
        {
            printe("ERROR: Expected w, h, d for tile shape on DXGI Format %u (%ls) (%08X) for 3D\n", f, GetName(DXGI_FORMAT(f)), static_cast<unsigned int>(hr));
            success = false;
        }
    }

    // invalid args
    TileShape shape = {};
    HRESULT hr = ComputeTileShape(DXGI_FORMAT_BC5_UNORM, TEX_DIMENSION_TEXTURE1D, shape);
    if (SUCCEEDED(hr))
    {
        printe("ERROR: Expected failure for BC format for 1D (%08X)\n", static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputeTileShape(DXGI_FORMAT_R8G8_B8G8_UNORM, TEX_DIMENSION_TEXTURE1D, shape);
    if (SUCCEEDED(hr))
    {
        printe("ERROR: Expected failure for packed format for 1D (%08X)\n", static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputeTileShape(DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE1D, shape);
    if (SUCCEEDED(hr))
    {
        printe("ERROR: Expected failure for video format for 1D (%08X)\n", static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputeTileShape(DXGI_FORMAT_R8G8_B8G8_UNORM, TEX_DIMENSION_TEXTURE2D, shape);
    if (SUCCEEDED(hr))
    {
        printe("ERROR: Expected failure for packed format for 2D (%08X)\n", static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputeTileShape(DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE2D, shape);
    if (SUCCEEDED(hr))
    {
        printe("ERROR: Expected failure for video format for 2D (%08X)\n", static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputeTileShape(DXGI_FORMAT_R8G8_B8G8_UNORM, TEX_DIMENSION_TEXTURE3D, shape);
    if (SUCCEEDED(hr))
    {
        printe("ERROR: Expected failure for packed format for 3D (%08X)\n", static_cast<unsigned int>(hr));
        success = false;
    }

    hr = ComputeTileShape(DXGI_FORMAT_YUY2, TEX_DIMENSION_TEXTURE3D, shape);
    if (SUCCEEDED(hr))
    {
        printe("ERROR: Expected failure for video format for 3D (%08X)\n", static_cast<unsigned int>(hr));
        success = false;
    }

    return success;
}
