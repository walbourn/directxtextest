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
#include <bcrypt.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status)          ((Status) >= 0)
#endif

struct bcrypthandle_closer { void operator()(BCRYPT_HASH_HANDLE h) { BCryptDestroyHash(h); } };

using ScopedHashHandle = std::unique_ptr<void, bcrypthandle_closer>;

#define MD5_DIGEST_LENGTH 16

HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages=0 )
{
    if ( image.GetImageCount() == 0 || !digest )
        return E_INVALIDARG;

    memset( digest, 0, MD5_DIGEST_LENGTH );

    NTSTATUS status;

    // Ensure have the MD5 algorithm ready
    static BCRYPT_ALG_HANDLE s_algid = nullptr;
    if ( !s_algid )
    {
        status = BCryptOpenAlgorithmProvider( &s_algid, BCRYPT_MD5_ALGORITHM, MS_PRIMITIVE_PROVIDER,  0 );
        if ( !NT_SUCCESS(status) )
            return HRESULT_FROM_NT(status);

        DWORD len = 0, res = 0;
        status = BCryptGetProperty( s_algid, BCRYPT_HASH_LENGTH, (PBYTE)&len, sizeof(DWORD), &res, 0 );
        if ( !NT_SUCCESS(status) || res != sizeof(DWORD) || len != MD5_DIGEST_LENGTH )
        {
            return E_FAIL;
        }
    }

    // Create hash object
    BCRYPT_HASH_HANDLE hobj;
    status = BCryptCreateHash( s_algid, &hobj, nullptr, 0, nullptr, 0, 0 );
    if ( !NT_SUCCESS(status) )
        return HRESULT_FROM_NT(status);

    ScopedHashHandle hash( hobj );

    const Image* images = image.GetImages();
    if ( !images )
        return E_POINTER;

    if ( !nimages || (nimages > image.GetImageCount()) )
        nimages = image.GetImageCount();

    for( size_t index=0; index < nimages; ++index )
    {
        const Image& img = images[ index ];

        size_t pitch = img.rowPitch;
        const uint8_t *pix = img.pixels;
        if ( !pix )
            return E_POINTER;

        const uint8_t *epix = img.pixels + img.slicePitch;

        for( ; pix < epix; pix += pitch )
        {
            status = BCryptHashData( hash.get(), (PBYTE)pix, (ULONG)pitch, 0 );
            if ( !NT_SUCCESS(status) )
                return HRESULT_FROM_NT(status);
        }
    }

    status = BCryptFinishHash( hash.get(), (PBYTE)digest, MD5_DIGEST_LENGTH, 0 );
    if ( !NT_SUCCESS(status) )
        return HRESULT_FROM_NT(status);

#if defined(_DEBUG) && defined(VERBOSE)
    char buff[1024] = ", { ";
    char tmp[16];

    for( size_t i=0; i < MD5_DIGEST_LENGTH; ++i )
    {
        sprintf_s( tmp, "0x%02x%s", digest[i], (i < (MD5_DIGEST_LENGTH-1)) ? "," : " } " );
        strcat_s( buff, tmp );
    }

    OutputDebugStringA( buff );
    OutputDebugStringA("\n");
#endif

    return S_OK;
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
