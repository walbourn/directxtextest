//-------------------------------------------------------------------------------------
// testutils.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"

#include "scoped.h"
#include "directxtexp.h"

#include "dds.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------
HRESULT LoadBlobFromFile( _In_z_ LPCWSTR szFile, Blob& blob )
{
    if ( szFile == NULL )
        return E_INVALIDARG;

    ScopedHandle hFile( safe_handle( CreateFile( szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                                 FILE_FLAG_SEQUENTIAL_SCAN, NULL ) ) );
    if ( !hFile )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    // Get the file size
    LARGE_INTEGER fileSize = {0};
    if ( !GetFileSizeEx( hFile.get(), &fileSize ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    // File is too big for 32-bit allocation, so reject read (4 GB should be plenty large enough for our test images)
    if ( fileSize.HighPart > 0 )
    {
        return HRESULT_FROM_WIN32( ERROR_FILE_TOO_LARGE );
    }

    // Need at least 1 byte of data
    if ( !fileSize.LowPart  )
    {
        return E_FAIL;
    }

    // Create blob memory
    HRESULT hr = blob.Initialize( fileSize.LowPart );
    if ( FAILED(hr) )
        return hr;
  
    // Load entire file into blob memory
    DWORD bytesRead = 0;
    if ( !ReadFile( hFile.get(), blob.GetBufferPointer(), static_cast<DWORD>( blob.GetBufferSize() ), &bytesRead, NULL ) )
    {
        blob.Release();
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    // Verify we got the whole blob loaded
    if ( bytesRead != blob.GetBufferSize() )
    {
        blob.Release();
        return E_FAIL;
    }

    return S_OK;
}


//-------------------------------------------------------------------------------------
#include <bcrypt.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#endif

struct bcrypthandle_closer { void operator()(BCRYPT_HASH_HANDLE h) { BCryptDestroyHash(h); } };

typedef std::unique_ptr<void, bcrypthandle_closer> ScopedHashHandle;

#define MD5_DIGEST_LENGTH 16

HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest )
{
    if ( image.GetImageCount() == 0 || !digest )
        return E_INVALIDARG;

    memset( digest, 0, MD5_DIGEST_LENGTH );

    NTSTATUS status;

    // Ensure have the MD5 algorithm ready
    static BCRYPT_ALG_HANDLE s_algid = 0;
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

    for( size_t index=0; index < image.GetImageCount(); ++index )
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

#ifdef DEBUG
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
HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image )
{
    if ( szFile == NULL || image.GetPixels() == NULL || image.GetPixelsSize() == 0 )
        return E_INVALIDARG;

    // Create DDS Header
    const size_t MAX_HEADER_SIZE = sizeof(DWORD) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
    BYTE header[MAX_HEADER_SIZE];
    size_t required;
    HRESULT hr = _EncodeDDSHeader( image.GetMetadata(), flags, header, MAX_HEADER_SIZE, required );
    if ( FAILED(hr) )
        return hr;

    // Create file and write header
    ScopedHandle hFile( safe_handle( CreateFile( szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL ) ) );
    if ( !hFile )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    DWORD bytesWritten;
    if ( !WriteFile( hFile.get(), header, static_cast<DWORD>( required ), &bytesWritten, NULL ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    if ( bytesWritten != required )
    {
        return E_FAIL;
    }

    // Scratch image is already formatted in memory how the DDS file is laid out, so write all pixel data...
    if ( !WriteFile( hFile.get(), image.GetPixels(), static_cast<DWORD>( image.GetPixelsSize() ), &bytesWritten, NULL ) )
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
HRESULT CreateWideImage( _In_count_(nimages) const Image* images, _In_ size_t nimages, _In_ const TexMetadata& metadata, _Out_ ScratchImage& result )
{
    if ( !images || !nimages )
        return E_INVALIDARG;

    HRESULT hr = result.Initialize( metadata, CP_FLAGS_PAGE4K );
    if ( FAILED(hr) )
        return hr;

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
HRESULT IsDDSHeaderPresent( _In_bytecount_(size) LPCVOID pSource, _In_ size_t size )
{
    if ( !pSource )
        return E_INVALIDARG;

    if ( size < (sizeof(DDS_HEADER) + sizeof(DWORD)) )
    {
        return HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
    }

    // DDS files always start with the same magic number ("DDS ")
    DWORD dwMagicNumber = *reinterpret_cast<const DWORD*>(pSource);
    if ( dwMagicNumber != DDS_MAGIC )
    {
        return E_FAIL;
    }

    const DDS_HEADER* pHeader = reinterpret_cast<const DDS_HEADER*>( (const BYTE*)pSource + sizeof( DWORD ) );
    assert( pHeader != NULL );

    // Verify header to validate DDS file
    if ( pHeader->dwSize != sizeof(DDS_HEADER)
         || pHeader->ddspf.dwSize != sizeof(DDS_PIXELFORMAT) )
    {
        return E_FAIL;
    }

    // Check for DX10 extension
    if ( (pHeader->ddspf.dwFlags & DDS_FOURCC)
         && (MAKEFOURCC( 'D', 'X', '1', '0' ) == pHeader->ddspf.dwFourCC) )
    {
        // Buffer must be big enough for both headers and magic value
        if ( size < (sizeof(DDS_HEADER)+sizeof(DWORD)+sizeof(DDS_HEADER_DXT10)) )
        {
            return E_FAIL;
        }

        return S_OK;
    }

    return S_FALSE;
}

HRESULT IsDDSHeaderPresent( _In_z_ LPCWSTR szFile )
{
    if ( szFile == NULL )
        return E_INVALIDARG;

    ScopedHandle hFile( safe_handle( CreateFile( szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                                 FILE_FLAG_SEQUENTIAL_SCAN, NULL ) ) );
    if ( !hFile )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    // Get the file size
    LARGE_INTEGER fileSize = {0};
    if ( !GetFileSizeEx( hFile.get(), &fileSize ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    // File is too big for 32-bit allocation, so reject read (4 GB should be plenty large enough for our test images)
    if ( fileSize.HighPart > 0 )
    {
        return HRESULT_FROM_WIN32( ERROR_FILE_TOO_LARGE );
    }

    // Need enough data for the MAGIC value and DDS standard header
    if ( fileSize.LowPart < sizeof(DDS_HEADER) + sizeof(DWORD) )
    {
        return E_FAIL;
    }

    // Read the header in (including extended header if present)
    const size_t MAX_HEADER_SIZE = sizeof(DWORD) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
    BYTE header[MAX_HEADER_SIZE];

    DWORD bytesRead = 0;
    if ( !ReadFile( hFile.get(), header, MAX_HEADER_SIZE, &bytesRead, NULL ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    return IsDDSHeaderPresent( header, bytesRead );
}