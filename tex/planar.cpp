//-------------------------------------------------------------------------------------
// planar.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "tex.h"

#include "DirectXTexP.h"

#include "getname.h"

using namespace DirectX;

namespace
{
    struct TestMedia
    {
        DXGI_FORMAT tformat;
        TexMetadata metadata;
        const wchar_t *fname;
    };

    const TestMedia g_TestMedia[] =
    {
        //  resultFormat | width height depth arraySize mipLevels miscFlags miscFlags2 format dimension | filename
        { DXGI_FORMAT_YUY2, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv12.dds" },

#ifndef BUILD_BVT_ONLY
        { DXGI_FORMAT_YUY2, { 3264, 2448, 1, 1, 1, 0, 0, DXGI_FORMAT_NV12, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"imagenv12.dds" },
        { DXGI_FORMAT_Y210, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P010, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP010.dds" },
        { DXGI_FORMAT_Y216, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_P016, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenaP016.dds" },
        { DXGI_FORMAT_YUY2, { 200, 200, 1, 1, 1, 0, 0, DXGI_FORMAT_NV11, TEX_DIMENSION_TEXTURE2D }, MEDIA_PATH L"lenanv11.dds" },
#endif // !BUILD_BVT_ONLY

        // TODO: need mips or array cases
    };
}

//-------------------------------------------------------------------------------------

extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// ConvertToSinglePlane
bool TEXTest::Test15()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_TestMedia); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

#if defined(_DEBUG) && defined(VERBOSE)
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        wchar_t tempDir[MAX_PATH] = {};
        ret = ExpandEnvironmentStringsW(TEMP_PATH L"plane", tempDir, MAX_PATH);
        if ( !ret || ret > MAX_PATH )
        {
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );
            return false;
        }

        CreateDirectoryW( tempDir, nullptr );

        TexMetadata metadata;
        ScratchImage srcimage;
        HRESULT hr = LoadFromDDSFile( szPath, DDS_FLAGS_NONE, &metadata, srcimage );

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe( "Failed getting DDS data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe( "Metadata error in DDS:\n%ls\n", szPath );
            printmeta( &metadata );
            printmetachk( check );
        }
        else
        {
            bool pass = true;

            //--- Simple convert --------------------------------------------------
            ScratchImage image;
            hr = ConvertToSinglePlane( *srcimage.GetImage(0,0,0), image );
            if ( FAILED(hr) )
            {
                success = false;
                pass = false;
                printe( "Failed single plane conversion (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
            }
            else if ( image.GetMetadata().format != g_TestMedia[index].tformat
                      || image.GetMetadata().width != srcimage.GetMetadata().width || image.GetMetadata().height != srcimage.GetMetadata().height )
            {
                success = false;
                pass = false;
                printe( "Failed conversion result is %zu x %zu (format %ls):\n",
                        image.GetMetadata().width, image.GetMetadata().height, GetName( image.GetMetadata().format ) );
                printe( "\n...(check) %zu x %zu (format %ls):\n%ls\n",
                        srcimage.GetMetadata().width, srcimage.GetMetadata().height, GetName( g_TestMedia[index].tformat ), szPath );
            }
            else
            {
                // TODO - Verify the image data (perhaps MD5 checksum)

                wchar_t tname[MAX_PATH] = {};
                wcscpy_s( tname, fname );
                wcscat_s( tname, L"_" );
                wcscat_s( tname, GetName(g_TestMedia[index].tformat) );

                wchar_t szDestPath[MAX_PATH] = {};
                _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );

                SaveScratchImage( szDestPath, DDS_FLAGS_NONE, image );

                if ( g_TestMedia[index].tformat != DXGI_FORMAT_YUY2 )
                {
                    wcscpy_s( tname, fname );
                    wcscat_s( tname, L"_DXGI_FORMAT_YUY2" );

                    ScratchImage yuy2;
                    hr = Convert( *image.GetImage(0,0,0), DXGI_FORMAT_YUY2, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, yuy2 );
                    if ( SUCCEEDED(hr) )
                    {
                        _wmakepath_s( szDestPath, MAX_PATH, nullptr, tempDir, tname, L".dds" );
                        SaveScratchImage( szDestPath, DDS_FLAGS_NONE, yuy2 );
                    }
                }
            }

            //--- Complex convert -------------------------------------------------
            // TODO: For now, we call with only a single image until we have more test data
            // if ( srcimage.GetImageCount() > 1 )
            {
                hr = ConvertToSinglePlane( srcimage.GetImages(), srcimage.GetImageCount(), srcimage.GetMetadata(), image );
                if ( FAILED(hr) )
                {
                    success = false;
                    pass = false;
                    printe( "Failed single plane conversion [complex] (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                }
                else if ( image.GetMetadata().format != g_TestMedia[index].tformat
                        || image.GetMetadata().width != srcimage.GetMetadata().width || image.GetMetadata().height != srcimage.GetMetadata().height
                        || image.GetImageCount() != srcimage.GetImageCount() )
                {
                    success = false;
                    pass = false;
                    printe( "Failed complex conversion result is %zu x %zu with %zu images (format %ls):\n",
                            image.GetMetadata().width, image.GetMetadata().height, image.GetImageCount(), GetName( image.GetMetadata().format ) );
                    printe( "\n...(check) %zu x %zu  %zu   (format %ls):\n%ls\n",
                            srcimage.GetMetadata().width, srcimage.GetMetadata().height, srcimage.GetImageCount(), GetName( g_TestMedia[index].tformat ), szPath );
                }
            }

            if ( pass )
                ++npass;

            // invalid args
            if (!index)
            {
            #pragma warning(push)
            #pragma warning(disable:6385 6387)
                auto img = *srcimage.GetImage(0,0,0);
                img.format = DXGI_FORMAT_UNKNOWN;
                hr = ConvertToSinglePlane(img, image);
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    printe("Failed invalid format arg test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                img.format = DXGI_FORMAT_R8G8B8A8_UNORM;
                hr = ConvertToSinglePlane(img, image);
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    printe("Failed non-planar format arg test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                img.format = DXGI_FORMAT_420_OPAQUE;
                hr = ConvertToSinglePlane(img, image);
                if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                {
                    success = false;
                    printe("Failed unsupported format arg test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                auto mdata = srcimage.GetMetadata();
                mdata.format = DXGI_FORMAT_UNKNOWN;
                hr = ConvertToSinglePlane( srcimage.GetImages(), srcimage.GetImageCount(), mdata, image );
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    printe("Failed invalid format arg complex test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                mdata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
                hr = ConvertToSinglePlane( srcimage.GetImages(), srcimage.GetImageCount(), mdata, image );
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    printe("Failed non-planar format arg complex test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                mdata.format = DXGI_FORMAT_420_OPAQUE;
                hr = ConvertToSinglePlane( srcimage.GetImages(), srcimage.GetImageCount(), mdata, image );
                if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                {
                    success = false;
                    printe("Failed unsupported format arg complex test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                mdata.format = DXGI_FORMAT_NV11;
                mdata.dimension = TEX_DIMENSION_TEXTURE3D;
                hr = ConvertToSinglePlane(srcimage.GetImages(), srcimage.GetImageCount(), mdata, image);
                if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                {
                    success = false;
                    printe("Failed invalid dimension complex arg test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                hr = ConvertToSinglePlane(srcimage.GetImages(), 0, srcimage.GetMetadata(), image);
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    printe("Failed zero images complex test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

            #if defined(_M_X64) || defined(_M_ARM64)
                img = *srcimage.GetImage(0,0,0);
                img.width = INT64_MAX;
                hr = ConvertToSinglePlane(img, image);
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    printe("Failed size too large arg test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }

                mdata = srcimage.GetMetadata();
                mdata.width = INT64_MAX;
                hr = ConvertToSinglePlane(srcimage.GetImages(), srcimage.GetImageCount(), mdata, image);
                if (hr != E_INVALIDARG)
                {
                    success = false;
                    printe("Failed size too large complex arg test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
                }
            #endif
            #pragma warning(pop)
            }

            ++ncount;
        }
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    // invalid args
    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        ScratchImage image;
        Image nullin = {};
        nullin.width = nullin.height = 256;
        nullin.format = DXGI_FORMAT_NV11;
        HRESULT hr = ConvertToSinglePlane(nullin, image);
        if (hr != E_POINTER)
        {
            success = false;
            printe("Failed invalid arg test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
        }

        TexMetadata metadata = {};
        metadata.width = metadata.height = 256;
        metadata.format = DXGI_FORMAT_NV11;
        metadata.depth = metadata.arraySize = metadata.mipLevels = 1;
        metadata.dimension = TEX_DIMENSION_TEXTURE2D;
        hr = ConvertToSinglePlane(nullptr, 0,  metadata, image);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid arg complex test (HRESULT: %08X)\n", static_cast<unsigned int>(hr));
        }
    #pragma warning(pop)
    }

    return success;
}
