//-------------------------------------------------------------------------------------
// utils.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

// VS 2010's stdint.h conflicts with intsafe.h
#pragma warning(push)
#pragma warning(disable : 4005)
#include <wrl.h>
#pragma warning(pop)

#include <algorithm>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

static const UINT DXGI_START = 1;
static const UINT DXGI_END = 120; // as of DXGI 1.2 + Xbox One

//-------------------------------------------------------------------------------------

extern const wchar_t* GetName( DXGI_FORMAT fmt );


//-------------------------------------------------------------------------------------
// IsValid
// IsCompressed
// IsPacked
// IsVideo
// IsPlanar
// IsPalettized
// IsDepthStencil
// IsSRGB
// IsTypeless
// BitsPerPixel
// BitsPerColor
bool Test01()
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
    static const DXGI_FORMAT typeless[] = 
    {
        DXGI_FORMAT_R32G32B32A32_TYPELESS, DXGI_FORMAT_R32G32B32_TYPELESS, DXGI_FORMAT_R16G16B16A16_TYPELESS, DXGI_FORMAT_R32G32_TYPELESS,
        DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, DXGI_FORMAT_R10G10B10A2_TYPELESS,
        DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R16G16_TYPELESS, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT_X24_TYPELESS_G8_UINT, DXGI_FORMAT_R8G8_TYPELESS, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R8_TYPELESS,
        DXGI_FORMAT_BC1_TYPELESS, DXGI_FORMAT_BC2_TYPELESS, DXGI_FORMAT_BC3_TYPELESS, DXGI_FORMAT_BC4_TYPELESS, DXGI_FORMAT_BC5_TYPELESS, 
        DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8X8_TYPELESS, DXGI_FORMAT_BC6H_TYPELESS, DXGI_FORMAT_BC7_TYPELESS,
        DXGI_FORMAT(119) /* DXGI_FORMAT_R16_UNORM_X8_TYPELESS */,
        DXGI_FORMAT(120) /* DXGI_FORMAT_X16_TYPELESS_G8_UINT */,
    };
    static const DXGI_FORMAT partialTypeless[] = 
    {
        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, 
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_X24_TYPELESS_G8_UINT, 
        DXGI_FORMAT(119) /* DXGI_FORMAT_R16_UNORM_X8_TYPELESS */,
        DXGI_FORMAT(120) /* DXGI_FORMAT_X16_TYPELESS_G8_UINT */,
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
        DXGI_FORMAT(116) /* DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT */,
        DXGI_FORMAT(117) /* DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT */,
    };
    static const DXGI_FORMAT video[] = 
    {
        DXGI_FORMAT_AYUV, DXGI_FORMAT_Y410, DXGI_FORMAT_Y416,
        DXGI_FORMAT_YUY2, DXGI_FORMAT_Y210, DXGI_FORMAT_Y216,
        DXGI_FORMAT_NV12, DXGI_FORMAT_P010, DXGI_FORMAT_P016, DXGI_FORMAT_420_OPAQUE, DXGI_FORMAT_NV11,
        DXGI_FORMAT_AI44, DXGI_FORMAT_IA44, DXGI_FORMAT_P8, DXGI_FORMAT_A8P8,
    };
    static const DXGI_FORMAT planar[] = 
    {
        DXGI_FORMAT_NV12, DXGI_FORMAT_P010, DXGI_FORMAT_P016, DXGI_FORMAT_420_OPAQUE, DXGI_FORMAT_NV11,
        DXGI_FORMAT(118) /* DXGI_FORMAT_D16_UNORM_S8_UINT */,
        DXGI_FORMAT(119) /* DXGI_FORMAT_R16_UNORM_X8_TYPELESS */,
        DXGI_FORMAT(120) /* DXGI_FORMAT_X16_TYPELESS_G8_UINT */,
    };
    static const DXGI_FORMAT pal[] =
    {
        DXGI_FORMAT_AI44, DXGI_FORMAT_IA44, DXGI_FORMAT_P8, DXGI_FORMAT_A8P8,
    };
    static const DXGI_FORMAT depthStencil[] =
    {
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, 
        DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_X24_TYPELESS_G8_UINT, 
        DXGI_FORMAT_D16_UNORM,
        DXGI_FORMAT(118) /* DXGI_FORMAT_D16_UNORM_S8_UINT */,
        DXGI_FORMAT(119) /* DXGI_FORMAT_R16_UNORM_X8_TYPELESS */,
        DXGI_FORMAT(120) /* DXGI_FORMAT_X16_TYPELESS_G8_UINT */,
    };
    bool success = true;

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        bool isCompressed = std::find( std::begin(bcFmts), std::end(bcFmts), f ) != std::end(bcFmts);

        bool isPAK = std::find( std::begin(packedFmts), std::end(packedFmts), f ) != std::end(packedFmts);

        bool isSRGB = std::find( std::begin(srgb), std::end(srgb), f ) != std::end(srgb);

        bool isTypeless = std::find( std::begin(typeless), std::end(typeless), f ) != std::end(typeless);
        bool isPartialTypeless = false;
        if ( isTypeless )
        {
            isPartialTypeless = std::find( std::begin(partialTypeless), std::end(partialTypeless), f ) == std::end(partialTypeless);
        }

        bool hasAlpha = std::find( std::begin(alpha), std::end(alpha), f ) != std::end(alpha);

        bool isVideo = std::find( std::begin(video), std::end(video), f ) != std::end(video);

        bool isPlanar = std::find( std::begin(planar), std::end(planar), f ) != std::end(planar);

        bool isPal = std::find( std::begin(pal), std::end(pal), f ) != std::end(pal);

        bool isDepthStencil = std::find( std::begin(depthStencil), std::end(depthStencil), f ) != std::end(depthStencil);

        if ( !IsValid( (DXGI_FORMAT)f ) )
        {
            printe( "ERROR: Isvalid failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) ) );
            success = false;
        }

        if ( IsCompressed( (DXGI_FORMAT)f ) != isCompressed )
        {
            printe( "ERROR: IsCompressed failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( IsPacked( (DXGI_FORMAT)f ) != isPAK )
        {
            printe( "ERROR: IsPacked failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( IsSRGB( (DXGI_FORMAT)f ) != isSRGB )
        {
            printe( "ERROR: IsSRGB failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( (IsTypeless( (DXGI_FORMAT)f ) != isTypeless)
             || (IsTypeless( (DXGI_FORMAT)f, true ) != isTypeless) )
        {
            printe( "ERROR: IsTypeless failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( IsTypeless( (DXGI_FORMAT)f, false ) != isPartialTypeless )
        {
            printe( "ERROR: IsTypeless(false) failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( HasAlpha( (DXGI_FORMAT)f ) != hasAlpha )
        {
            printe( "ERROR: HasAlpha failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( IsVideo( (DXGI_FORMAT)f) != isVideo )
        {
            printe( "ERROR: IsVideo failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( IsPlanar( (DXGI_FORMAT)f ) != isPlanar )
        {
            printe( "ERROR: IsPlanar failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( IsPalettized( (DXGI_FORMAT)f ) != isPal )
        {
            printe( "ERROR: IsPalettized failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( IsDepthStencil( (DXGI_FORMAT)f ) != isDepthStencil )
        {
            printe( "ERROR: IsDepthStencil failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( BitsPerPixel( (DXGI_FORMAT)f ) == 0 )
        {
            printe( "ERROR: BitsPerPixel failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
            success = false;
        }

        if ( isPal )
        {
            if ( BitsPerColor( (DXGI_FORMAT)f ) != 0 )
            {
                printe( "ERROR: BitsPerColor succeeded on DXGI Format %u when it should have returned 0 (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
                success = false;
            }
        }
        else
        {
            if ( BitsPerColor( (DXGI_FORMAT)f ) == 0 )
            {
                printe( "ERROR: BitsPerColor failed on DXGI Format %u (%S)\n", f, GetName( DXGI_FORMAT(f) )  );
                success = false;
            }
        }
    }

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

    return success;
}


namespace
{
//-------------------------------------------------------------------------------------
// This function is used by DDSTextureLoader and ScreenGrab to do the same thing
// as DirectXTex's ComputePitch, so we test it here...
//-------------------------------------------------------------------------------------
static void GetSurfaceInfo( _In_ size_t width,
                            _In_ size_t height,
                            _In_ DXGI_FORMAT fmt,
                            _Out_opt_ size_t* outNumBytes,
                            _Out_opt_ size_t* outRowBytes,
                            _Out_opt_ size_t* outNumRows )
{
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;
    switch (static_cast<int>(fmt))
    {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        bc=true;
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
        planar = true;
        bpe = 2;
        break;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
        planar = true;
        bpe = 4;
        break;

    case 118 /* DXGI_FORMAT_D16_UNORM_S8_UINT */:
    case 119 /* DXGI_FORMAT_R16_UNORM_X8_TYPELESS */:
    case 120 /* DXGI_FORMAT_X16_TYPELESS_G8_UINT */:
        planar = true;
        bpe = 4;
        break;
    }

    if (bc)
    {
        size_t numBlocksWide = 0;
        if (width > 0)
        {
            numBlocksWide = std::max<size_t>( 1, (width + 3) / 4 );
        }
        size_t numBlocksHigh = 0;
        if (height > 0)
        {
            numBlocksHigh = std::max<size_t>( 1, (height + 3) / 4 );
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    }
    else if (packed)
    {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numRows = height;
        numBytes = rowBytes * height;
    }
    else if ( fmt == DXGI_FORMAT_NV11 )
    {
        rowBytes = ( ( width + 3 ) >> 2 ) * 4;
        numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
    }
    else if (planar)
    {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numBytes = ( rowBytes * height ) + ( ( rowBytes * height + 1 ) >> 1 );
        numRows = height + ( ( height + 1 ) >> 1 );
    }
    else
    {
        size_t bpp = BitsPerPixel( fmt );
        rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
        numRows = height;
        numBytes = rowBytes * height;
    }

    if (outNumBytes)
    {
        *outNumBytes = numBytes;
    }
    if (outRowBytes)
    {
        *outRowBytes = rowBytes;
    }
    if (outNumRows)
    {
        *outNumRows = numRows;
    }
}
};

//-------------------------------------------------------------------------------------
// ComputePitch
// ComputeScanlines
bool Test02()
{
    size_t rowPitch = 0;
    size_t slicePitch = 0;

    bool success = true;

    ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch );
    if ( rowPitch != 2 || slicePitch != 12 )
    {
        printf("ERROR: CP RGB565 1x1 failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_LEGACY_DWORD );
    if ( rowPitch != 4 || slicePitch != 24 )
    {
        printf("ERROR: CP RGB565 1x1 DWORD failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_PARAGRAPH );
    if ( rowPitch != 16 || slicePitch != 96 )
    {
        printf("ERROR: CP RGB565 1x1 PARAGRAPH failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_YMM );
    if ( rowPitch != 32 || slicePitch != 192 )
    {
        printf("ERROR: CP RGB565 1x1 YMM failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_ZMM );
    if ( rowPitch != 64 || slicePitch != 384 )
    {
        printf("ERROR: CP RGB565 1x1 ZMM failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_B5G6R5_UNORM, 1, 6, rowPitch, slicePitch, CP_FLAGS_PAGE4K );
    if ( rowPitch != 4096 || slicePitch != 24576 )
    {
        printf("ERROR: CP RGB565 1x1 PAGE4K failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_NONE );
    if ( rowPitch != 8 || slicePitch != 8 )
    {
        printf("ERROR: CP R8G8B8A8_UNORM failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_24BPP );
    if ( rowPitch != 6 || slicePitch != 6 )
    {
        printf("ERROR: CP 24BPP failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_16BPP );
    if ( rowPitch != 4 || slicePitch != 4 )
    {
        printf("ERROR: CP 16BPP failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, rowPitch, slicePitch, CP_FLAGS_8BPP );
    if ( rowPitch != 2 || slicePitch != 2 )
    {
        printf("ERROR: CP 8PP failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT(116), 2, 1, rowPitch, slicePitch );
    if ( rowPitch != 8 || slicePitch != 8 )
    {
        printf("ERROR: R10G10B10_7E3_A2_FLOAT [Xbox] A failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT(116), 128, 64, rowPitch, slicePitch );
    if ( rowPitch != 512 || slicePitch != 32768 )
    {
        printf("ERROR: R10G10B10_7E3_A2_FLOAT [Xbox] B failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT(117), 2, 1, rowPitch, slicePitch );
    if ( rowPitch != 8 || slicePitch != 8 )
    {
        printf("ERROR: R10G10B10_6E4_A2_FLOAT [Xbox] A failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    ComputePitch( DXGI_FORMAT(117), 128, 64, rowPitch, slicePitch );
    if ( rowPitch != 512 || slicePitch != 32768 )
    {
        printf("ERROR: R10G10B10_6E4_A2_FLOAT [Xbox] B failed rowPitch %Iu, slicePitch %Iu\n", rowPitch, slicePitch );
        success = false;
    }

    for( unsigned int j = 118; j <= 120; ++j )
    {
        ComputePitch( DXGI_FORMAT(j), 4, 2, rowPitch, slicePitch );
        if ( rowPitch != 8 || slicePitch != 24 )
        {
            printf("ERROR: %S A failed rowPitch %Iu, slicePitch %Iu\n", GetName( DXGI_FORMAT(j) ), rowPitch, slicePitch );
            success = false;
        }

        ComputePitch( DXGI_FORMAT(j), 128, 64, rowPitch, slicePitch );
        if ( rowPitch != 256 || slicePitch != 24576 )
        {
            printf("ERROR: %S B failed rowPitch %Iu, slicePitch %Iu\n", GetName( DXGI_FORMAT(j) ), rowPitch, slicePitch );
            success = false;
        }

        size_t numBytes = 0;
        size_t rowBytes = 0;
        size_t numRows = 0;
        GetSurfaceInfo( 4, 2, DXGI_FORMAT(j), &numBytes, &rowBytes, &numRows );
        if ( rowBytes != 8 || numBytes != 24 || numRows != 3 )
        {
            printf("ERROR: %S A failed rowBytes %Iu, numBytes %Iu, numRows %Iu\n", GetName( DXGI_FORMAT(j) ), rowBytes, numBytes, numRows );
            success = false;
        }

        GetSurfaceInfo( 128, 64, DXGI_FORMAT(j), &numBytes, &rowBytes, &numRows );
        if ( rowBytes != 256 || numBytes != 24576 || numRows != (64+32) )
        {
            printf("ERROR: %S B failed rowBytes %Iu, numBytes %Iu, numRows %Iu\n", GetName( DXGI_FORMAT(j) ), rowBytes, numBytes, numRows );
            success = false;
        }
    }

    //
    // Here we are comparing the DirectX Runtime vs. DirectXTex, so we only need a NULL device. 
    //
    ComPtr<ID3D11Device> device;
    D3D_FEATURE_LEVEL lvl;
    ComPtr<ID3D11DeviceContext> context;
    UINT createDeviceFlags = 0;
#ifdef D3D_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDevice( nullptr,
                                    D3D_DRIVER_TYPE_NULL,
                                    nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, device.GetAddressOf(), &lvl, context.GetAddressOf() );
    if ( FAILED(hr) )
    {
        printe( "Failed creating Direct3D NULL device (HRESULT %08X)\n", hr );
        return false;
    }

#if defined(D3D_DEBUG)
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
            D3D11_INFO_QUEUE_FILTER filter = {0};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            infoQ->AddStorageFilterEntries( &filter );
        }
    }
#endif

    size_t ncount = 0;
    size_t npass = 0;

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        switch ( f )
        {
        case DXGI_FORMAT_R1_UNORM:
            continue;

        case DXGI_FORMAT(116) /* DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT */:
        case DXGI_FORMAT(117) /* DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT */:
            // Only supported for Xbox One platform
            continue;

        case DXGI_FORMAT(118) /* DXGI_FORMAT_D16_UNORM_S8_UINT */:
        case DXGI_FORMAT(119) /* DXGI_FORMAT_R16_UNORM_X8_TYPELESS */:
        case DXGI_FORMAT(120) /* DXGI_FORMAT_X16_TYPELESS_G8_UINT */:
            // Special use case formats for Xbox One platform
            continue;
        }

        static const UINT vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 32, 64, 100, 128, 131, 132, 200, 256, 768, 1024, 2048, 4096, 8192, 16384 };

        for(UINT i = 0; i < _countof(vals); ++i )
        {
            for(UINT j = 0; j < _countof(vals); ++j )
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
                    printe("ERROR: Failed creating texture for format %S (%u), %u by %u (%08X)\n", GetName( DXGI_FORMAT(f) ), f, vals[i], vals[j], hr );
                    success = false;
                    continue;
                }

                D3D11_MAPPED_SUBRESOURCE res = {0};
                hr = context->Map( tex.Get(), 0, D3D11_MAP_WRITE, 0, &res );
                if ( FAILED(hr) )
                {
                    printe("ERROR: Failed mapping texture for format %S (%u), %u by %u\n", GetName( DXGI_FORMAT(f) ), f, vals[i], vals[j] );
                    success = false;
                    continue;
                }

                ComputePitch( DXGI_FORMAT(f), vals[i], vals[j], rowPitch, slicePitch, CP_FLAGS_NONE );
                if ( rowPitch != res.RowPitch || slicePitch != res.DepthPitch )
                {
                    printe("ERROR: %S (%u) failed rowPitch %Iu, slicePitch %Iu (%u by %u)...check rowPitch %u slicePitch %u\n",
                           GetName( DXGI_FORMAT(f) ), f, rowPitch, slicePitch, vals[i], vals[j], res.RowPitch, res.DepthPitch );
                    success = false;
                }
                else
                {
                    UINT targetRows = vals[j];
                    if ( f == DXGI_FORMAT_NV11 )
                    {
                        targetRows = vals[j] * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
                    }
                    else if ( f == 118 /* DXGI_FORMAT_D16_UNORM_S8_UINT */
                              || f == 119 /* DXGI_FORMAT_R16_UNORM_X8_TYPELESS */
                              || f == 120 /* DXGI_FORMAT_X16_TYPELESS_G8_UINT */ )
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
                        printe("ERROR: %S (%u) failed scanlines %Iu (%u)...check scanlines %u\n",
                               GetName( DXGI_FORMAT(f) ), f, rowCount, vals[j], targetRows );
                        success = false;
                    }
                    else
                    {
                        size_t numBytes = 0;
                        size_t rowBytes = 0;
                        size_t numRows = 0;
                        GetSurfaceInfo( vals[i], vals[j], DXGI_FORMAT(f), &numBytes, &rowBytes, &numRows );

                        if ( rowBytes != res.RowPitch || numBytes != res.DepthPitch || numRows != targetRows )
                        {
                            printe("ERROR: %S (%u) failed rowBytes %Iu, numBytes %Iu, numRows %Iu (%u by %u)...check rowBytes %u numBytes %u numRows %u\n",
                                   GetName( DXGI_FORMAT(f) ), f, rowBytes, numBytes, numRows, vals[i], vals[j],
                                   res.RowPitch, res.DepthPitch, targetRows );
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

    print("%Iu formats/sizes tested vs. DX, %Iu passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// MakeSRGB
// MakeTypeless
// MakeTypelessUNORM
// MakeTypelessFLOAT
bool Test12()
{
    bool success = true;

    //--- MakeSRGB
    static const DXGI_FORMAT hasSRGB[] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8X8_UNORM,
        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC7_UNORM, 
    };

    for( size_t i = 0; i < _countof(hasSRGB); ++i )
    {
        DXGI_FORMAT f = MakeSRGB( hasSRGB[i] );
        
        if ( !IsSRGB( f ) )
        {
            printe( "ERROR: MakeSRGB failed on DXGI Format %u\n", hasSRGB[i] );
            success = false;
        }
    }

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        size_t i = 0;
        for( ; i < _countof(hasSRGB); ++i )
        {
            if ( hasSRGB[i] == (DXGI_FORMAT)f )
                break;
        }

        if ( i < _countof(hasSRGB) )
            continue;

        if ( MakeSRGB( (DXGI_FORMAT)f ) != (DXGI_FORMAT)f )
        {
            printe( "ERROR: MakeSRGB failed on DXGI Format %u\n", f );
            success = false;
        }
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
    };

    for( size_t i = 0; i < _countof(hasTypeless); ++i )
    {
        DXGI_FORMAT f = MakeTypeless( hasTypeless[i] );
        
        if ( !IsTypeless( f ) )
        {
            printe( "ERROR: MakeTypeless failed on DXGI Format %u\n", hasTypeless[i] );
            success = false;
        }
    }

    for (UINT f = DXGI_START; f <= DXGI_END; ++f )
    {
        size_t i = 0;
        for( ; i < _countof(hasTypeless); ++i )
        {
            if ( hasTypeless[i] == (DXGI_FORMAT)f )
                break;
        }

        if ( i < _countof(hasTypeless) )
            continue;

        if ( MakeTypeless( (DXGI_FORMAT)f ) != (DXGI_FORMAT)f )
        {
            printe( "ERROR: MakeTypeless failed on DXGI Format %u\n", f );
            success = false;
        }
    }

    //--- MakeTypelessUNORM
    //--- MakeTypelessFLOAT
    static const DXGI_FORMAT typeless[] = 
    {
        DXGI_FORMAT_R32G32B32A32_TYPELESS, DXGI_FORMAT_R32G32B32_TYPELESS, DXGI_FORMAT_R16G16B16A16_TYPELESS, DXGI_FORMAT_R32G32_TYPELESS,
        DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, DXGI_FORMAT_R10G10B10A2_TYPELESS,
        DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R16G16_TYPELESS, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT_X24_TYPELESS_G8_UINT, DXGI_FORMAT_R8G8_TYPELESS, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R8_TYPELESS,
        DXGI_FORMAT_BC1_TYPELESS, DXGI_FORMAT_BC2_TYPELESS, DXGI_FORMAT_BC3_TYPELESS, DXGI_FORMAT_BC4_TYPELESS, DXGI_FORMAT_BC5_TYPELESS, 
        DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8X8_TYPELESS, DXGI_FORMAT_BC6H_TYPELESS, DXGI_FORMAT_BC7_TYPELESS
    };

    for( UINT t = 0; t < _countof(typeless); ++t )
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
                printe( "ERROR: MakeTypelessUNORM failed on DXGI Format %u\n", typeless[t] );
                success = false;
            }
        }
        else
        {
            if ( BitsPerPixel( typeless[t] ) != BitsPerPixel( fu ) )
            {
                printe( "ERROR: MakeTypelessUNORM %u -> %u changes bpp\n", typeless[t], fu );
                success = false;
            }
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
                printe( "ERROR: MakeTypelessFLOAT failed on DXGI Format %u\n", typeless[t] );
                success = false;
            }
        }
        else
        {
            if ( BitsPerPixel( typeless[t] ) != BitsPerPixel( ff ) )
            {
                printe( "ERROR: MakeTypelessFLOAT %u -> %u changes bpp\n", typeless[t], fu );
                success = false;
            }
        }
    }

    return success;
}