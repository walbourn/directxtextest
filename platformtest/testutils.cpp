//-------------------------------------------------------------------------------------
// testutils.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "pch.h"

#include "directxtest.h"

#include "scoped.h"
#include "DirectXTexP.h"

#include "DDS.h"

using namespace DirectX;

//#define VERBOSE

//-------------------------------------------------------------------------------------
HRESULT LoadBlobFromFile( _In_z_ const wchar_t* szFile, Blob& blob )
{
    if ( szFile == nullptr )
        return E_INVALIDARG;

    ScopedHandle hFile(safe_handle(CreateFile2(
        szFile,
        GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING,
        nullptr)));
    if ( !hFile )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    // Get the file size
    LARGE_INTEGER fileSize = {};
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
    if ( !ReadFile( hFile.get(), blob.GetBufferPointer(), static_cast<DWORD>( blob.GetBufferSize() ), &bytesRead, nullptr ) )
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
size_t DetermineFileSize( _In_z_ const wchar_t* szFile )
{
    if ( szFile == nullptr )
        return 0;

    ScopedHandle hFile(safe_handle(CreateFile2(
        szFile,
        GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING,
        nullptr)));
    if ( !hFile )
    {
        return 0;
    }

    // Get the file size
    LARGE_INTEGER fileSize = {};
    if ( !GetFileSizeEx( hFile.get(), &fileSize ) )
    {
        return 0;
    }

    return size_t( fileSize.QuadPart );
}


//-------------------------------------------------------------------------------------
#include <bcrypt.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status)          ((Status) >= 0)
#endif

struct bcrypthandle_closer { void operator()(BCRYPT_HASH_HANDLE h) { BCryptDestroyHash(h); } };

using ScopedHashHandle = std::unique_ptr<void, bcrypthandle_closer>;

#define MD5_DIGEST_LENGTH 16

HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest )
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

    for( size_t index=0; index < image.GetImageCount(); ++index )
    {
        const Image& img = images[ index ];

        const size_t pitch = img.rowPitch;
        const uint8_t *pix = img.pixels;
        if ( !pix )
            return E_POINTER;

        // TODO - For DXGI_FORMAT_R1_UNORM need to handle 'dirty edge' which changes between debugger and non-debugger runs

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

HRESULT MD5Checksum(_In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages = 0)
{
    if (image.GetImageCount() == 0 || !digest)
        return E_INVALIDARG;

    memset(digest, 0, MD5_DIGEST_LENGTH);

    NTSTATUS status;

    // Ensure have the MD5 algorithm ready
    static BCRYPT_ALG_HANDLE s_algid = nullptr;
    if (!s_algid)
    {
        status = BCryptOpenAlgorithmProvider(&s_algid, BCRYPT_MD5_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
        if (!NT_SUCCESS(status))
            return HRESULT_FROM_NT(status);

        DWORD len = 0, res = 0;
        status = BCryptGetProperty(s_algid, BCRYPT_HASH_LENGTH, (PBYTE)&len, sizeof(DWORD), &res, 0);
        if (!NT_SUCCESS(status) || res != sizeof(DWORD) || len != MD5_DIGEST_LENGTH)
        {
            return E_FAIL;
        }
    }

    // Create hash object
    BCRYPT_HASH_HANDLE hobj;
    status = BCryptCreateHash(s_algid, &hobj, nullptr, 0, nullptr, 0, 0);
    if (!NT_SUCCESS(status))
        return HRESULT_FROM_NT(status);

    ScopedHashHandle hash(hobj);

    const Image* images = image.GetImages();
    if (!images)
        return E_POINTER;

    if (!nimages || (nimages > image.GetImageCount()))
        nimages = image.GetImageCount();

    for (size_t index = 0; index < nimages; ++index)
    {
        const Image& img = images[index];

        const size_t pitch = img.rowPitch;
        const uint8_t *pix = img.pixels;
        if (!pix)
            return E_POINTER;

        const uint8_t *epix = img.pixels + img.slicePitch;

        for (; pix < epix; pix += pitch)
        {
            status = BCryptHashData(hash.get(), (PBYTE)pix, (ULONG)pitch, 0);
            if (!NT_SUCCESS(status))
                return HRESULT_FROM_NT(status);
        }
    }

    status = BCryptFinishHash(hash.get(), (PBYTE)digest, MD5_DIGEST_LENGTH, 0);
    if (!NT_SUCCESS(status))
        return HRESULT_FROM_NT(status);

#if defined(_DEBUG) && defined(VERBOSE)
    char buff[1024] = ", { ";
    char tmp[16];

    for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        sprintf_s(tmp, "0x%02x%s", digest[i], (i < (MD5_DIGEST_LENGTH - 1)) ? "," : " } ");
        strcat_s(buff, tmp);
    }

    OutputDebugStringA(buff);
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
    ScopedHandle hFile(safe_handle(CreateFile2(
        szFile,
        GENERIC_WRITE, 0, CREATE_ALWAYS,
        nullptr)));
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
HRESULT CopyViaLoadStoreScanline( const Image& srcImage, ScratchImage& image )
{
    using namespace DirectX::Internal;

    if ( srcImage.pixels == nullptr )
        return E_INVALIDARG;

    ScratchImage temp;
    HRESULT hr = temp.Initialize2D( DXGI_FORMAT_R32G32B32A32_FLOAT, srcImage.width, 1, 1, 1 );
    if ( FAILED(hr) )
        return hr;

    const Image *timg = temp.GetImage( 0, 0, 0 );
    if ( timg == nullptr )
        return E_POINTER;

    // Only need 1 scanline of temp memory (hence why temp has a height of 1 above)
    XMVECTOR* tscanline = const_cast<XMVECTOR*>( reinterpret_cast<const XMVECTOR*>( timg->pixels ) );
    if ( tscanline == nullptr )
        return E_POINTER;

    hr = image.Initialize2D( srcImage.format, srcImage.width, srcImage.height, 1, 1 );
    if ( FAILED(hr) )
        return hr;

    const Image *img = image.GetImage( 0, 0, 0 );
    if ( img == nullptr )
    {
        image.Release();
        return E_POINTER;
    }

    BYTE *pDest = img->pixels;
    if ( !pDest )
    {
        image.Release();
        return E_POINTER;
    }

    const BYTE *pSrc = srcImage.pixels;
    for( UINT h = 0; h < srcImage.height; ++h )
    {
        if ( !LoadScanline( tscanline, srcImage.width, pSrc, srcImage.rowPitch, srcImage.format ) )
        {
            image.Release();
            return E_FAIL;
        }

        if ( !StoreScanline( pDest, img->rowPitch, srcImage.format, tscanline, srcImage.width ) )
        {
            image.Release();
            return E_FAIL;
        }

        pSrc += srcImage.rowPitch;
        pDest += img->rowPitch;
    }

    return S_OK;
}
