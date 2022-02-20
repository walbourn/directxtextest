//-------------------------------------------------------------------------------------
// testutils.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "scoped.h"
#include "DirectXTexP.h"

#include "DDS.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------
HRESULT CreateDevice( ID3D11Device** pDev, ID3D11DeviceContext** pContext )
{
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };
    constexpr UINT numDriverTypes = static_cast<UINT>(std::size(driverTypes));

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    constexpr UINT numFeatureLevels = static_cast<UINT>(std::size(featureLevels));

    HRESULT hr = E_FAIL;

    UINT createDeviceFlags = 0;
#ifdef D3D_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        hr = D3D11CreateDevice( nullptr, driverTypes[driverTypeIndex], nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, pDev, nullptr, pContext );
        if( SUCCEEDED( hr ) )
            break;
    }

    return hr;
}

//-------------------------------------------------------------------------------------
HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image )
{
    if ( szFile == nullptr || image.GetPixels() == nullptr || image.GetPixelsSize() == 0 )
        return E_INVALIDARG;

    // Create DDS Header
    const size_t MAX_HEADER_SIZE = sizeof(DWORD) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
    BYTE header[MAX_HEADER_SIZE];
    size_t required;
    HRESULT hr = EncodeDDSHeader( image.GetMetadata(), flags, header, MAX_HEADER_SIZE, required );
    if ( FAILED(hr) )
        return hr;

    // Create file and write header
    ScopedHandle hFile(safe_handle(CreateFile(szFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)));
    if ( !hFile )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    DWORD bytesWritten;
    if ( !WriteFile( hFile.get(), header, static_cast<DWORD>( required ), &bytesWritten, nullptr ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    if ( bytesWritten != required )
    {
        return E_FAIL;
    }

    if (image.GetPixelsSize() > UINT32_MAX)
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

    // Scratch image is already formatted in memory how the DDS file is laid out, so write all pixel data...
    if ( !WriteFile( hFile.get(), image.GetPixels(), static_cast<DWORD>( image.GetPixelsSize() ), &bytesWritten, nullptr ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    if ( bytesWritten != image.GetPixelsSize() )
    {
        return E_FAIL;
    }

    return S_OK;
}


//-------------------------------------------------------------------------------------
HRESULT CreateWideImage( _In_count_(nimages) const Image* images, size_t nimages, _In_ const TexMetadata& metadata, _Out_ ScratchImage& result )
{
    if ( !images || !nimages )
        return E_INVALIDARG;

    if ( IsCompressed(metadata.format) )
    {
        // Trick to create the image as a larger size than we need for BC
        TexMetadata mdata2 = metadata;
        mdata2.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        HRESULT hr = result.Initialize( mdata2, CP_FLAGS_PAGE4K );
        if ( FAILED(hr) )
            return hr;

        // Hack it back to our original BC
        result.OverrideFormat( metadata.format );
    }
    else
    {
        HRESULT hr = result.Initialize( metadata, CP_FLAGS_PAGE4K );
        if ( FAILED(hr) )
            return hr;
    }

    for( size_t index=0; index < nimages; ++index )
    {
        auto sptr = reinterpret_cast<const uint8_t*>( images[index].pixels );
        if ( !sptr )
            return E_POINTER;

        const Image& img = result.GetImages()[ index ];

        auto dptr = reinterpret_cast<uint8_t*>( img.pixels );
        if ( !dptr )
            return E_POINTER;

        size_t rowCount = ComputeScanlines( img.format, img.height );
        if ( !rowCount )
            return E_UNEXPECTED;

        size_t spitch = images[index].rowPitch;
        size_t dpitch = img.rowPitch;

        size_t csize = std::min<size_t>( dpitch, spitch );

        for( size_t y = 0; y < rowCount; ++y )
        {
            memcpy_s( dptr, dpitch, sptr, csize );
            sptr += spitch;
            dptr += dpitch;
        }
    }

    return S_OK;
}


//-------------------------------------------------------------------------------------
#define ENUM_CASE(x) case x: return L## #x;

const wchar_t* GetName( DXGI_FORMAT fmt )
{
    switch( static_cast<int>(fmt) )
    {
    ENUM_CASE( DXGI_FORMAT_R32G32B32A32_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R32G32B32A32_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R32G32B32A32_UINT )
    ENUM_CASE( DXGI_FORMAT_R32G32B32A32_SINT )
    ENUM_CASE( DXGI_FORMAT_R32G32B32_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R32G32B32_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R32G32B32_UINT )
    ENUM_CASE( DXGI_FORMAT_R32G32B32_SINT )
    ENUM_CASE( DXGI_FORMAT_R16G16B16A16_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R16G16B16A16_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R16G16B16A16_UNORM )
    ENUM_CASE( DXGI_FORMAT_R16G16B16A16_UINT )
    ENUM_CASE( DXGI_FORMAT_R16G16B16A16_SNORM )
    ENUM_CASE( DXGI_FORMAT_R16G16B16A16_SINT )
    ENUM_CASE( DXGI_FORMAT_R32G32_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R32G32_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R32G32_UINT )
    ENUM_CASE( DXGI_FORMAT_R32G32_SINT )
    ENUM_CASE( DXGI_FORMAT_R32G8X24_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_D32_FLOAT_S8X24_UINT )
    ENUM_CASE( DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_X32_TYPELESS_G8X24_UINT )
    ENUM_CASE( DXGI_FORMAT_R10G10B10A2_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R10G10B10A2_UNORM )
    ENUM_CASE( DXGI_FORMAT_R10G10B10A2_UINT )
    ENUM_CASE( DXGI_FORMAT_R11G11B10_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R8G8B8A8_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R8G8B8A8_UNORM )
    ENUM_CASE( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB )
    ENUM_CASE( DXGI_FORMAT_R8G8B8A8_UINT )
    ENUM_CASE( DXGI_FORMAT_R8G8B8A8_SNORM )
    ENUM_CASE( DXGI_FORMAT_R8G8B8A8_SINT )
    ENUM_CASE( DXGI_FORMAT_R16G16_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R16G16_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R16G16_UNORM )
    ENUM_CASE( DXGI_FORMAT_R16G16_UINT )
    ENUM_CASE( DXGI_FORMAT_R16G16_SNORM )
    ENUM_CASE( DXGI_FORMAT_R16G16_SINT )
    ENUM_CASE( DXGI_FORMAT_R32_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_D32_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R32_FLOAT )
    ENUM_CASE( DXGI_FORMAT_R32_UINT )
    ENUM_CASE( DXGI_FORMAT_R32_SINT )
    ENUM_CASE( DXGI_FORMAT_R24G8_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_D24_UNORM_S8_UINT )
    ENUM_CASE( DXGI_FORMAT_R24_UNORM_X8_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_X24_TYPELESS_G8_UINT )
    ENUM_CASE( DXGI_FORMAT_R8G8_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R8G8_UNORM )
    ENUM_CASE( DXGI_FORMAT_R8G8_UINT )
    ENUM_CASE( DXGI_FORMAT_R8G8_SNORM )
    ENUM_CASE( DXGI_FORMAT_R8G8_SINT )
    ENUM_CASE( DXGI_FORMAT_R16_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R16_FLOAT )
    ENUM_CASE( DXGI_FORMAT_D16_UNORM )
    ENUM_CASE( DXGI_FORMAT_R16_UNORM )
    ENUM_CASE( DXGI_FORMAT_R16_UINT )
    ENUM_CASE( DXGI_FORMAT_R16_SNORM )
    ENUM_CASE( DXGI_FORMAT_R16_SINT )
    ENUM_CASE( DXGI_FORMAT_R8_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_R8_UNORM )
    ENUM_CASE( DXGI_FORMAT_R8_UINT )
    ENUM_CASE( DXGI_FORMAT_R8_SNORM )
    ENUM_CASE( DXGI_FORMAT_R8_SINT )
    ENUM_CASE( DXGI_FORMAT_A8_UNORM )
    ENUM_CASE( DXGI_FORMAT_R1_UNORM )
    ENUM_CASE( DXGI_FORMAT_R9G9B9E5_SHAREDEXP )
    ENUM_CASE( DXGI_FORMAT_R8G8_B8G8_UNORM )
    ENUM_CASE( DXGI_FORMAT_G8R8_G8B8_UNORM )
    ENUM_CASE( DXGI_FORMAT_BC1_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_BC1_UNORM )
    ENUM_CASE( DXGI_FORMAT_BC1_UNORM_SRGB )
    ENUM_CASE( DXGI_FORMAT_BC2_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_BC2_UNORM )
    ENUM_CASE( DXGI_FORMAT_BC2_UNORM_SRGB )
    ENUM_CASE( DXGI_FORMAT_BC3_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_BC3_UNORM )
    ENUM_CASE( DXGI_FORMAT_BC3_UNORM_SRGB )
    ENUM_CASE( DXGI_FORMAT_BC4_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_BC4_UNORM )
    ENUM_CASE( DXGI_FORMAT_BC4_SNORM )
    ENUM_CASE( DXGI_FORMAT_BC5_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_BC5_UNORM )
    ENUM_CASE( DXGI_FORMAT_BC5_SNORM )
    ENUM_CASE( DXGI_FORMAT_B5G6R5_UNORM )
    ENUM_CASE( DXGI_FORMAT_B5G5R5A1_UNORM )
    ENUM_CASE( DXGI_FORMAT_B8G8R8A8_UNORM )
    ENUM_CASE( DXGI_FORMAT_B8G8R8X8_UNORM )
    ENUM_CASE( DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM )
    ENUM_CASE( DXGI_FORMAT_B8G8R8A8_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_B8G8R8A8_UNORM_SRGB )
    ENUM_CASE( DXGI_FORMAT_B8G8R8X8_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_B8G8R8X8_UNORM_SRGB )
    ENUM_CASE( DXGI_FORMAT_BC6H_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_BC6H_UF16 )
    ENUM_CASE( DXGI_FORMAT_BC6H_SF16 )
    ENUM_CASE( DXGI_FORMAT_BC7_TYPELESS )
    ENUM_CASE( DXGI_FORMAT_BC7_UNORM )
    ENUM_CASE( DXGI_FORMAT_BC7_UNORM_SRGB )
    ENUM_CASE( DXGI_FORMAT_AYUV )
    ENUM_CASE( DXGI_FORMAT_Y410 )
    ENUM_CASE( DXGI_FORMAT_Y416 )
    ENUM_CASE( DXGI_FORMAT_NV12 )
    ENUM_CASE( DXGI_FORMAT_P010 )
    ENUM_CASE( DXGI_FORMAT_P016 )
    ENUM_CASE( DXGI_FORMAT_420_OPAQUE )
    ENUM_CASE( DXGI_FORMAT_YUY2 )
    ENUM_CASE( DXGI_FORMAT_Y210 )
    ENUM_CASE( DXGI_FORMAT_Y216 )
    ENUM_CASE( DXGI_FORMAT_NV11 )
    ENUM_CASE( DXGI_FORMAT_AI44 )
    ENUM_CASE( DXGI_FORMAT_IA44 )
    ENUM_CASE( DXGI_FORMAT_P8 )
    ENUM_CASE( DXGI_FORMAT_A8P8 )
    ENUM_CASE( DXGI_FORMAT_B4G4R4A4_UNORM )
    case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT: return L"DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT";
    case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT: return L"DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT";
    case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT: return L"DXGI_FORMAT_D16_UNORM_S8_UINT";
    case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS: return L"DXGI_FORMAT_R16_UNORM_X8_TYPELESS";
    case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT: return L"DXGI_FORMAT_X16_TYPELESS_G8_UINT";
    case WIN10_DXGI_FORMAT_P208: return L"DXGI_FORMAT_P208";
    case WIN10_DXGI_FORMAT_V208: return L"DXGI_FORMAT_V208";
    case WIN10_DXGI_FORMAT_V408: return L"DXGI_FORMAT_V408";
    case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM: return L"DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM";
    case XBOX_DXGI_FORMAT_R4G4_UNORM: return L"DXGI_FORMAT_R4G4_UNORM";

    default:
        return L"UNKNOWN";
    }
}


//-------------------------------------------------------------------------------------
// See https://www.gamasutra.com/view/news/169203/Exceptional_floating_point.php
int __cdecl DescribeException(PEXCEPTION_POINTERS pData)

{
    // Clear the exception or else every FP instruction will
    // trigger it again.
    _clearfp();

    DWORD exceptionCode = pData->ExceptionRecord->ExceptionCode;

    const char* pDescription = nullptr;
    switch (exceptionCode)
    {
        case STATUS_FLOAT_INVALID_OPERATION:
            pDescription = "float invalid operation";
            break;

        case STATUS_FLOAT_DIVIDE_BY_ZERO:
            pDescription = "float divide by zero";
            break;

        case STATUS_FLOAT_OVERFLOW:
            pDescription = "float overflow";
            break;

        case STATUS_FLOAT_UNDERFLOW:
            pDescription = "float underflow";
            break;

        case STATUS_FLOAT_INEXACT_RESULT:
            pDescription = "float inexact result";
            break;

        case STATUS_FLOAT_MULTIPLE_TRAPS:
            // This seems to occur with SSE code.
            pDescription = "float multiple traps";
            break;

        case STATUS_ACCESS_VIOLATION:
            pDescription = "access violation";
            break;

        default:
            pDescription = "unknown exception";
            break;
    }

    void* pErrorOffset = 0;
#if defined(_M_IX86)
    void* pIP = (void*)pData->ContextRecord->Eip;
    pErrorOffset = (void*)pData->ContextRecord->FloatSave.ErrorOffset;
#elif defined(_M_X64)
    void* pIP = (void*)pData->ContextRecord->Rip;
#else
#error Unknown processor
#endif

    printf("ERROR: Crash with exception %lx (%s) at %p!n",
        exceptionCode, pDescription, pIP);

    if (pErrorOffset)
    {
        // Float exceptions may be reported in a delayed manner -- report the
        // actual instruction as well.

        printf("Faulting instruction may actually be at %p.n", pErrorOffset);
    }

    // Return this value to execute the __except block and continue as if
    // all was fine, which is a terrible idea in shipping code.
    return EXCEPTION_EXECUTE_HANDLER;

    // Return this value to let the normal exception handling process
    // continue after printing diagnostics/saving crash dumps/etc.
    //return EXCEPTION_CONTINUE_SEARCH;
}
