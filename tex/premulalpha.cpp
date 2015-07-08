//-------------------------------------------------------------------------------------
// premulalpha.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------

extern HRESULT MD5Checksum( _In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages=0 );
extern HRESULT SaveScratchImage( _In_z_ LPCWSTR szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------

inline bool IsEqual( float f1, float f2 )
{
    return ( fabs(f1 - f2) < 0.000001 ) != 0;
}

//-------------------------------------------------------------------------------------
// PremultiplyAlpha
bool Test13()
{
    bool success = true;

    // Alpha image
    WCHAR szPath[MAX_PATH];
    ExpandEnvironmentStringsW( MEDIA_PATH L"tree02S.dds", szPath, MAX_PATH );

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    // Form dest path
    WCHAR ext[_MAX_EXT];
    WCHAR fname[_MAX_FNAME];
    _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

    WCHAR tempDir[MAX_PATH];
    ExpandEnvironmentStringsW( TEMP_PATH L"premul", tempDir, MAX_PATH );

    CreateDirectoryW( tempDir, NULL );

    const TexMetadata checkai = { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    TexMetadata metadata;
    ScratchImage imagealpha;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, imagealpha );
    if ( FAILED(hr) )
    {
        success = false;
        printe( "Failed loading dds (HRESULT %08X):\n%S\n", hr, szPath );
    }
    else if ( memcmp( &metadata, &checkai, sizeof(TexMetadata) ) != 0 )
    {
        success = false;
        printe( "Metadata error in DDS:\n%S\n", szPath );
        printmeta( &metadata );
        printmetachk( &checkai );
    }

    uint8_t srcdigest1[16];
    hr = MD5Checksum( imagealpha, srcdigest1, 1 );
    if ( FAILED(hr) )
    {
        success = false;
        printe( "Failed computing MD5 checksum (HRESULT %08X):\n%S\n", hr, szPath );
    }

    uint8_t srcdigestall[16];
    hr = MD5Checksum( imagealpha, srcdigestall );
    if ( FAILED(hr) )
    {
        success = false;
        printe( "Failed computing MD5 checksum (HRESULT %08X):\n%S\n", hr, szPath );
    }

    // PremultiplyAlpha (single)
    {
        ScratchImage pmAlpha;
        hr = PremultiplyAlpha( *imagealpha.GetImage(0,0,0), TEX_PMALPHA_DEFAULT, pmAlpha );
        uint8_t digestDefault[16];
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed computing premultiply alpha [single] (HRESULT %08X)\n", hr );
        }
        else
        {
            // Verfy the result
            hr = MD5Checksum( pmAlpha, digestDefault, 1 );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing premultiply alpha [single] of image data (HRESULT %08X)\n", hr );
            }
            else if ( memcmp( digestDefault, srcdigest1, 16 ) == 0 )
            {
                success = false;
                printe( "Premultiply image [single] is identical to original!\n" );
            }
            else
            {
                float mse, mseV[4];
                hr = ComputeMSE( *imagealpha.GetImage(0,0,0), *pmAlpha.GetImage(0,0,0), mse, mseV );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed comparing premultiply alpha [single] of image data (HRESULT %08X)\n", hr );
                }

                if ( !IsEqual( mse, 0.005031f ) )
                {
                    success = false;
                    printe( "Failed comparing premultiply alpha [single] of image data (%f)\n", mse );
                }
            }

            WCHAR szDestPath[MAX_PATH];
            _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, fname, L".dds" );

            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, pmAlpha );
        }

        // TEX_PMALPHA_SRGB
        ScratchImage pmAlphaSRGB;
        hr = PremultiplyAlpha( *imagealpha.GetImage(0,0,0), TEX_PMALPHA_SRGB, pmAlphaSRGB );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed computing premultiply alpha [single sRGB] (HRESULT %08X)\n", hr );
        }
        else
        {
            // Verfy the result
            uint8_t digest[16];
            hr = MD5Checksum( pmAlphaSRGB, digest, 1 );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing premultiply alpha [single sRGB] of image data (HRESULT %08X)\n", hr );
            }
            else if ( memcmp( digest, srcdigest1, 16 ) == 0 )
            {
                success = false;
                printe( "Premultiply image [single sRGB] is identical to original!\n" );
            }
            else if ( memcmp( digest, digestDefault, 16 ) == 0 )
            {
                success = false;
                printe( "Premultiply image [single sRGB] is identical to non-sRGB!\n" );
            }
            else
            {
                float mse, mseV[4];
                hr = ComputeMSE( *imagealpha.GetImage(0,0,0), *pmAlphaSRGB.GetImage(0,0,0), mse, mseV );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed comparing premultiply alpha [single sRGB] of image data (HRESULT %08X)\n", hr );
                }

                if ( !IsEqual( mse, 0.002190f ) )
                {
                    success = false;
                    printe( "Failed comparing premultiply alpha [single sRGB] of image data (%f)\n", mse );
                }
            }

            WCHAR tname[MAX_PATH];
            wcscpy_s( tname, fname );
            wcscat_s( tname, L"_sRGB" );

            WCHAR szDestPath[MAX_PATH];
            _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, pmAlphaSRGB );
        }

        // TEX_PMALPHA_IGNORE_SRGB
        ScratchImage pmAlphaNoSRGB;
        hr = PremultiplyAlpha( *imagealpha.GetImage(0,0,0), TEX_PMALPHA_IGNORE_SRGB | TEX_PMALPHA_SRGB, pmAlphaNoSRGB );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed computing premultiply alpha [single ignore sRGB] (HRESULT %08X)\n", hr );
        }
        else
        {
            // Verfy the result
            uint8_t digest[16];
            hr = MD5Checksum( pmAlphaNoSRGB, digest, 1 );
            if ( FAILED(hr) )
            {
                success = false;
                printe( "Failed computing premultiply alpha [single ignore sRGB] of image data (HRESULT %08X)\n", hr );
            }
            else if ( memcmp( digest, srcdigest1, 16 ) == 0 )
            {
                success = false;
                printe( "Premultiply image [single ignore sRGB] is identical to original!\n" );
            }
            else if ( memcmp( digest, digestDefault, 16 ) != 0 )
            {
                success = false;
                printe( "Premultiply image [single ignore sRGB] is *not* identical to non-sRGB!\n" );
            }
        }
    }

    // PremultiplyAlpha (complex)
    {
        ScratchImage pmAlpha;
        hr = PremultiplyAlpha( imagealpha.GetImages(), imagealpha.GetImageCount(), imagealpha.GetMetadata(), TEX_PMALPHA_DEFAULT, pmAlpha );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed computing premultiply alpha [complex] (HRESULT %08X)\n", hr );
        }
        else
        {
            const TexMetadata& metadata2 = pmAlpha.GetMetadata();

            TexMetadata chk = metadata;
            chk.SetAlphaMode( TEX_ALPHA_MODE_PREMULTIPLIED );

            if ( memcmp( &metadata2, &chk, sizeof(TexMetadata) ) != 0 )
            {
                success = false;
                printe( "Metadata error in premultiply alpha [complex] \n" );
                printmeta( &metadata2 );
                printmetachk( &chk );
            }
            else
            {
                // Verfy the result
                uint8_t digest[16];
                hr = MD5Checksum( pmAlpha, digest );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed computing premultiply alpha [complex] of image data (HRESULT %08X)\n", hr );
                }
                else if ( memcmp( digest, srcdigestall, 16 ) == 0 )
                {
                    success = false;
                    printe( "Premultiply image [complex] is identical to original!\n" );
                }
                else
                {
                    static float result[9] = { 0.005031f, 0.005441f, 0.005821f, 0.006450f, 0.007194f, 0.008633f, 0.012355f, 0.018454f, 0.020023f };
                    for( size_t j = 0; j < metadata.mipLevels; ++j )
                    {
                        float mse, mseV[4];
                        hr = ComputeMSE( *imagealpha.GetImage(j,0,0), *pmAlpha.GetImage(j,0,0), mse, mseV );
                        if ( FAILED(hr) )
                        {
                            success = false;
                            printe( "Failed comparing premultiply alpha [complex] of image data (HRESULT %08X, %Iu)\n", hr, j );
                        }

                        if ( !IsEqual( mse, result[j] ) )
                        {
                            success = false;
                            printe( "Failed comparing premultiply alpha [complex] of image data (%f, %Iu)\n", mse, j );
                        }
                    }
                }

                WCHAR tname[MAX_PATH];
                wcscpy_s( tname, fname );
                wcscat_s( tname, L"_c" );

                WCHAR szDestPath[MAX_PATH];
                _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

                SaveScratchImage( szDestPath, DDS_FLAGS_NONE, pmAlpha );
            }
        }
    }

    return success;
}