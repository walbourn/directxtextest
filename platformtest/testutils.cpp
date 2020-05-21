//-------------------------------------------------------------------------------------
// testutils.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "pch.h"

#include "directxtest.h"

#include "scoped.h"
#include "directxtexp.h"

#include "dds.h"

using namespace DirectX;

//#define VERBOSE

//-------------------------------------------------------------------------------------
HRESULT LoadBlobFromFile( _In_z_ const wchar_t* szFile, Blob& blob )
{
    if ( szFile == nullptr )
        return E_INVALIDARG;

    ScopedHandle hFile( safe_handle( CreateFile2( szFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr ) ) );
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

    ScopedHandle hFile(safe_handle(CreateFile2(szFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr)));
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
#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
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
    static BCRYPT_ALG_HANDLE s_algid = 0;
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

        size_t pitch = img.rowPitch;
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
HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DWORD flags, _In_ const ScratchImage& image )
{
    if ( szFile == nullptr || image.GetPixels() == nullptr || image.GetPixelsSize() == 0 )
        return E_INVALIDARG;

    // Create DDS Header
    const size_t MAX_HEADER_SIZE = sizeof(DWORD) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
    BYTE header[MAX_HEADER_SIZE];
    size_t required;
    HRESULT hr = _EncodeDDSHeader( image.GetMetadata(), flags, header, MAX_HEADER_SIZE, required );
    if ( FAILED(hr) )
        return hr;

    // Create file and write header
    ScopedHandle hFile( safe_handle( CreateFile2( szFile, GENERIC_WRITE, 0, CREATE_ALWAYS, nullptr ) ) );
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
        if ( !_LoadScanline( tscanline, srcImage.width, pSrc, srcImage.rowPitch, srcImage.format ) )
        {
            image.Release();
            return E_FAIL;
        }

        if ( !_StoreScanline( pDest, img->rowPitch, srcImage.format, tscanline, srcImage.width ) )
        {
            image.Release();
            return E_FAIL;
        }

        pSrc += srcImage.rowPitch;
        pDest += img->rowPitch;
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
