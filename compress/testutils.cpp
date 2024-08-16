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
    BYTE header[DDS_DX10_HEADER_SIZE];
    size_t required;
    HRESULT hr = EncodeDDSHeader( image.GetMetadata(), flags, header, DDS_DX10_HEADER_SIZE, required );
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

    void* pErrorOffset = nullptr;
#if defined(_M_IX86)
    void* pIP = reinterpret_cast<void*>(pData->ContextRecord->Eip);
    pErrorOffset = reinterpret_cast<void*>(pData->ContextRecord->FloatSave.ErrorOffset);
#elif defined(_M_X64)
    void* pIP = reinterpret_cast<void*>(pData->ContextRecord->Rip);
#elif defined(_M_ARM64)
    void* pIP = reinterpret_cast<void*>(pData->ContextRecord->Pc);
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
