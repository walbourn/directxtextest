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
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DWORD flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------

inline bool IsEqual( float f1, float f2 )
{
    return ( fabs(f1 - f2) < 0.000001 ) != 0;
}

inline bool IsErrorTooLarge(float f, float threshold)
{
    return (fabs(f) > threshold) != 0;
}

//-------------------------------------------------------------------------------------
// PremultiplyAlpha
bool Test13()
{
    bool success = true;

    // Alpha image
    wchar_t szPath[MAX_PATH];
    DWORD ret = ExpandEnvironmentStringsW(MEDIA_PATH L"tree02S.dds", szPath, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

#ifdef DEBUG
    OutputDebugString(szPath);
    OutputDebugStringA("\n");
#endif

    // Form dest path
    wchar_t ext[_MAX_EXT];
    wchar_t fname[_MAX_FNAME];
    _wsplitpath_s( szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

    wchar_t tempDir[MAX_PATH];
    ret = ExpandEnvironmentStringsW(TEMP_PATH L"premul", tempDir, MAX_PATH);
    if ( !ret || ret > MAX_PATH )
    {
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
        return false;
    }

    CreateDirectoryW( tempDir, NULL );

    const TexMetadata checkai = { 304, 268, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D };

    TexMetadata metadata;
    ScratchImage imagealpha;
    HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, imagealpha );
    if ( FAILED(hr) )
    {
        printe( "Failed loading dds (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }
    else if ( memcmp( &metadata, &checkai, sizeof(TexMetadata) ) != 0 )
    {
        printe( "Metadata error in DDS:\n%ls\n", szPath );
        printmeta( &metadata );
        printmetachk( &checkai );
        return false;
    }

    uint8_t srcdigest1[16];
    hr = MD5Checksum( imagealpha, srcdigest1, 1 );
    if ( FAILED(hr) )
    {
        printe( "Failed computing MD5 checksum (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }

    uint8_t srcdigestall[16];
    hr = MD5Checksum( imagealpha, srcdigestall );
    if ( FAILED(hr) )
    {
        printe( "Failed computing MD5 checksum (HRESULT %08X):\n%ls\n", hr, szPath );
        return false;
    }

    // PremultiplyAlpha (single)
    {
        ScratchImage pmAlpha;
        hr = PremultiplyAlpha( *imagealpha.GetImage(0,0,0), TEX_PMALPHA_DEFAULT, pmAlpha );
        uint8_t digestDefault[16] = { 0 };
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

            wchar_t szDestPath[MAX_PATH] = {};
            _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, fname, L".dds" );

            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, pmAlpha );

            // Reverse
            ScratchImage alpha;
            hr = PremultiplyAlpha(*pmAlpha.GetImage(0, 0, 0), TEX_PMALPHA_DEFAULT, alpha, true);
            if (FAILED(hr))
            {
                success = false;
                printe("Failed computing demul alpha [single] (HRESULT %08X)\n", hr);
            }
            else
            {
                float mse, mseV[4];
                hr = ComputeMSE(*imagealpha.GetImage(0, 0, 0), *alpha.GetImage(0, 0, 0), mse, mseV);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed comparing demul alpha [single] of image data (HRESULT %08X)\n", hr);
                }

                if (IsErrorTooLarge(mse, 0.0001f))
                {
                    success = false;
                    printe("Failed comparing demul alpha [single] of image data (%f)\n", mse);
                }

                wchar_t fname2[_MAX_FNAME];
                wcscpy_s(fname2, fname);
                wcscat_s(fname2, L"_reverse");

                wchar_t szDestPath2[MAX_PATH] = {};
                _wmakepath_s(szDestPath2, MAX_PATH, NULL, tempDir, fname2, L".dds");

                SaveScratchImage(szDestPath2, DDS_FLAGS_NONE, alpha);
            }
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
                else if ( !IsEqual( mse, 0.002190f ) )
                {
                    success = false;
                    printe( "Failed comparing premultiply alpha [single sRGB] of image data (%f)\n", mse );
                }
            }

            wchar_t tname[MAX_PATH] = { 0 };
            wcscpy_s( tname, fname );
            wcscat_s( tname, L"_sRGB" );

            wchar_t szDestPath[MAX_PATH];
            _wmakepath_s( szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds" );

            SaveScratchImage( szDestPath, DDS_FLAGS_NONE, pmAlphaSRGB );

            // Reverse
            ScratchImage alphaSRGB;
            hr = PremultiplyAlpha(*pmAlphaSRGB.GetImage(0, 0, 0), TEX_PMALPHA_SRGB, alphaSRGB, true);
            if (FAILED(hr))
            {
                success = false;
                printe("Failed computing demul alpha [single sRGB] (HRESULT %08X)\n", hr);
            }
            else
            {
                float mse, mseV[4];
                hr = ComputeMSE(*imagealpha.GetImage(0, 0, 0), *alphaSRGB.GetImage(0, 0, 0), mse, mseV);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed comparing demul alpha [single sRGB] of image data (HRESULT %08X)\n", hr);
                }

                if (IsErrorTooLarge(mse, 0.0001f))
                {
                    success = false;
                    printe("Failed comparing demul alpha [single sRGB] of image data (%f)\n", mse);
                }

                wchar_t fname2[_MAX_FNAME];
                wcscpy_s(fname2, fname);
                wcscat_s(fname2, L"_sRGB_reverse");

                wchar_t szDestPath2[MAX_PATH] = {};
                _wmakepath_s(szDestPath2, MAX_PATH, NULL, tempDir, fname2, L".dds");

                SaveScratchImage(szDestPath2, DDS_FLAGS_NONE, alphaSRGB);
            }
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

            // Reverse
            ScratchImage alphaNoSRGB;
            hr = PremultiplyAlpha(*pmAlphaNoSRGB.GetImage(0, 0, 0), TEX_PMALPHA_IGNORE_SRGB | TEX_PMALPHA_SRGB, alphaNoSRGB, true);
            if (FAILED(hr))
            {
                success = false;
                printe("Failed computing demul alpha [single sRGB] (HRESULT %08X)\n", hr);
            }
            else
            {
                float mse, mseV[4];
                hr = ComputeMSE(*imagealpha.GetImage(0, 0, 0), *alphaNoSRGB.GetImage(0, 0, 0), mse, mseV);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed comparing demul alpha [single sRGB] of image data (HRESULT %08X)\n", hr);
                }

                if (IsErrorTooLarge(mse, 0.0001f))
                {
                    success = false;
                    printe("Failed comparing demul alpha [single sRGB] of image data (%f)\n", mse);
                }
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
                hr = MD5Checksum(pmAlpha, digest);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed computing premultiply alpha [complex] of image data (HRESULT %08X)\n", hr);
                }
                else if (memcmp(digest, srcdigestall, 16) == 0)
                {
                    success = false;
                    printe("Premultiply image [complex] is identical to original!\n");
                }
                else
                {
                    static float result[9] = { 0.005031f, 0.005441f, 0.005821f, 0.006450f, 0.007194f, 0.008633f, 0.012355f, 0.018454f, 0.020023f };
                    for (size_t j = 0; j < metadata.mipLevels; ++j)
                    {
                        float mse, mseV[4];
                        hr = ComputeMSE(*imagealpha.GetImage(j, 0, 0), *pmAlpha.GetImage(j, 0, 0), mse, mseV);
                        if (FAILED(hr))
                        {
                            success = false;
                            printe("Failed comparing premultiply alpha [complex] of image data (HRESULT %08X, %Iu)\n", hr, j);
                        }
                        else if (!IsEqual(mse, result[j]))
                        {
                            success = false;
                            printe("Failed comparing premultiply alpha [complex] of image data (%f, %Iu)\n", mse, j);
                        }
                    }
                }

                wchar_t tname[MAX_PATH] = { 0 };
                wcscpy_s(tname, fname);
                wcscat_s(tname, L"_c");

                wchar_t szDestPath[MAX_PATH];
                _wmakepath_s(szDestPath, MAX_PATH, NULL, tempDir, tname, L".dds");

                SaveScratchImage(szDestPath, DDS_FLAGS_NONE, pmAlpha);

                // Reverse
                ScratchImage alpha;
                hr = PremultiplyAlpha(pmAlpha.GetImages(), pmAlpha.GetImageCount(), pmAlpha.GetMetadata(), TEX_PMALPHA_DEFAULT, alpha, true);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed computing delmul alpha [complex] (HRESULT %08X)\n", hr);
                }
                else
                {
                    const TexMetadata& metadata2 = alpha.GetMetadata();

                    TexMetadata chk = metadata;
                    chk.SetAlphaMode(TEX_ALPHA_MODE_STRAIGHT);

                    if (memcmp(&metadata2, &chk, sizeof(TexMetadata)) != 0)
                    {
                        success = false;
                        printe("Metadata error in delmul alpha [complex] \n");
                        printmeta(&metadata2);
                        printmetachk(&chk);
                    }
                    else
                    {
                        float mse, mseV[4];
                        hr = ComputeMSE(*imagealpha.GetImage(0, 0, 0), *alpha.GetImage(0, 0, 0), mse, mseV);
                        if (FAILED(hr))
                        {
                            success = false;
                            printe("Failed comparing demul alpha [complex] of image data (HRESULT %08X)\n", hr);
                        }

                        if (IsErrorTooLarge(mse, 0.0001f))
                        {
                            success = false;
                            printe("Failed comparing demul alpha [complex] of image data (%f)\n", mse);
                        }
                    }
                }
            }
        }
    }

    return success;
}