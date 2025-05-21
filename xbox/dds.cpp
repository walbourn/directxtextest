//-------------------------------------------------------------------------------------
// dds.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"
#include "DirectXTexXbox.h"

using namespace DirectX;

struct TestMedia
{
    TexMetadata metadata;
    bool isXbox;
    const wchar_t *fname;
};

static const TestMedia g_TestMedia[] = 
{
{ { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, true, L"Media\\xboxseafloor.dds" },
{ { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8X8_UNORM, TEX_DIMENSION_TEXTURE2D }, true, L"Media\\xboxtiny_skin.dds" },
{ { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, true, L"Media\\xboxwindowslogo.dds" },
{ { 256, 256, 1, 1, 9, 0, 0x3, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, true, L"Media\\xboxwindowslogoBC1.dds" },

{ { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"reftexture.dds" },
{ { 200, 150, 1, 1, 1, 0, TEX_ALPHA_MODE_OPAQUE, DXGI_FORMAT_B5G5R5A1_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"test555.dds" },
{ { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"test8888.dds" },
{ { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"test8888mip.dds" },
{ { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"testcube8888.dds" },
{ { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"testcube8888mip.dds" },
{ { 32, 32, 1, 6, 1, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"testcubedxt5.dds" },
{ { 32, 32, 1, 6, 6, TEX_MISC_TEXTURECUBE, 0, DXGI_FORMAT_BC3_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"testcubedxt5mip.dds" },
{ { 32, 32, 1, 1, 1, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"testdxt1.dds" },
{ { 32, 32, 1, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE2D }, false, MEDIA_PATH L"testdxt1mip.dds" },
{ { 32, 32, 4, 1, 1, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, false, MEDIA_PATH L"testvol8888.dds" },
{ { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE3D }, false, MEDIA_PATH L"testvol8888mip.dds" },
{  { 32, 32, 4, 1, 6, 0, 0, DXGI_FORMAT_BC1_UNORM, TEX_DIMENSION_TEXTURE3D }, false, MEDIA_PATH L"testvoldxt1mip.dds" },
};

static_assert(!std::is_copy_assignable<Xbox::XboxImage>::value, "Copy Assign.");
static_assert(!std::is_copy_constructible<Xbox::XboxImage>::value, "Copy Ctor.");
static_assert(std::is_nothrow_move_constructible<Xbox::XboxImage>::value, "Move Ctor.");
static_assert(std::is_nothrow_move_assignable<Xbox::XboxImage>::value, "Move Assign.");

//-------------------------------------------------------------------------------------

extern HRESULT LoadBlobFromFile( _In_z_ LPCWSTR szFile, Blob& blob );
extern HRESULT SaveScratchImage(_In_z_ LPCWSTR szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image);

//-------------------------------------------------------------------------------------
// GetMetadataFromDDSMemory
bool Test01()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_TestMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        ExpandEnvironmentStringsW( g_TestMedia[index].fname, szPath, MAX_PATH );

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        Blob blob;
        HRESULT hr = LoadBlobFromFile( szPath, blob );
        if ( FAILED(hr) )
        {
            success = false;
            printe("Failed getting raw file data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else
        {
            TexMetadata metadata;
            bool isXbox;
            hr = Xbox::GetMetadataFromDDSMemory(blob.GetBufferPointer(), blob.GetBufferSize(), metadata, isXbox);
#ifdef _USE_SCARLETT
            if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                continue;
#endif

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
            {
                success = false;
                printe("Metadata error in:\n%ls\n", szPath);
                printmeta( &metadata );
                printmetachk( check );
            }
            else if ( g_TestMedia[index].isXbox != isXbox )
            {
                success = false;
                printe("Mismatch in expected 'isXbox' status:\n%ls\n", szPath);
            }
            else
            {
                ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// GetMetadataFromDDSFile
bool Test02()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_TestMedia); ++index )
    {
        WCHAR szPath[MAX_PATH];
        ExpandEnvironmentStringsW( g_TestMedia[index].fname, szPath, MAX_PATH );

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        bool isXbox;
        HRESULT hr = Xbox::GetMetadataFromDDSFile( szPath, metadata, isXbox );
#ifdef _USE_SCARLETT
        if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            continue;
#endif
        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe("Failed getting data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta( &metadata );
            printmetachk( check );
        }
        else if ( g_TestMedia[index].isXbox != isXbox )
        {
            success = false;
            printe("Mismatch in expected 'isXbox' status:\n%ls\n", szPath);
        }
        else
        {
            ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromDDSMemory
bool Test03()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_TestMedia); ++index )
    {
        if ( !g_TestMedia[index].isXbox )
            continue;

        WCHAR szPath[MAX_PATH];
        ExpandEnvironmentStringsW( g_TestMedia[index].fname, szPath, MAX_PATH );

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        Blob blob;
        HRESULT hr = LoadBlobFromFile( szPath, blob );
        if ( FAILED(hr) )
        {
            success = false;
            printe("Failed getting raw file data from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else
        {
            TexMetadata metadata;
            Xbox::XboxImage image;
            hr = Xbox::LoadFromDDSMemory( blob.GetBufferPointer(), blob.GetBufferSize(), &metadata, image );
#ifdef _USE_SCARLETT
            if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                continue;
#endif

            const TexMetadata* check = &g_TestMedia[index].metadata;
            if ( FAILED(hr) )
            {
                success = false;
                printe("Failed loading dds from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
            {
                success = false;
                printe("Metadata error in:\n%ls\n", szPath);
                printmeta( &metadata );
                printmetachk( check );
            }
            else if ( !image.GetSize() || !image.GetAlignment() || image.GetTileMode() == -1 )
            {
                success = false;
                printe("Size, Alignment, and/or TileMode error:\n%ls\n", szPath);
            }
            else
            {
                ++npass;
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// LoadFromDDSFile
bool Test04()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_TestMedia); ++index )
    {
        if ( !g_TestMedia[index].isXbox )
            continue;

        WCHAR szPath[MAX_PATH];
        ExpandEnvironmentStringsW( g_TestMedia[index].fname, szPath, MAX_PATH );

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        Xbox::XboxImage image;
        HRESULT hr = Xbox::LoadFromDDSFile( szPath, &metadata, image );
#ifdef _USE_SCARLETT
        if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            continue;
#endif

        const TexMetadata* check = &g_TestMedia[index].metadata;
        if ( FAILED(hr) )
        {
            success = false;
            printe("Failed loading dds from memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else if ( memcmp( &metadata, check, sizeof(TexMetadata) ) != 0 )
        {
            success = false;
            printe("Metadata error in:\n%ls\n", szPath);
            printmeta( &metadata );
            printmetachk( check );
        }
        else if ( !image.GetSize() || !image.GetAlignment() || image.GetTileMode() == -1 )
        {
            success = false;
            printe("Size, Alignment, and/or TileMode error:\n%ls\n", szPath);
        }
        else
        {
            ++npass;
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveDDSToMemory
bool Test05()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < std::size(g_TestMedia); ++index )
    {
        if ( !g_TestMedia[index].isXbox )
            continue;

        WCHAR szPath[MAX_PATH];
        ExpandEnvironmentStringsW( g_TestMedia[index].fname, szPath, MAX_PATH );

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        TexMetadata metadata;
        Xbox::XboxImage image;
        HRESULT hr = Xbox::LoadFromDDSFile(szPath, &metadata, image);
#ifdef _USE_SCARLETT
        if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            continue;
#endif

        if ( FAILED(hr) )
        {
            success = false;
            printe("Failed loading DDS from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else
        {
            Blob blob;
            hr = Xbox::SaveToDDSMemory( image, blob );
            if ( FAILED(hr) )
            {
                success = false;
                printe("Failed writing DDS to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
            }
            else
            {
                TexMetadata metadata2;
                Xbox::XboxImage image2;
                hr = Xbox::LoadFromDDSMemory( blob.GetBufferPointer(), blob.GetBufferSize(), &metadata2, image2 );
                if ( FAILED(hr) )
                {
                    success = false;
                    printe( "Failed reading back written DDS to memory (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath );
                }
                else
                {
                    if ( memcmp( &metadata, &metadata2, sizeof(TexMetadata) ) != 0 )
                    {
                        success = false;
                        printe( "Metadata error in dds memory readback:\n%ls\n", szPath );
                        printmeta( &metadata2 );
                        printmetachk( &metadata );
                    }
                    else if ( image.GetSize() != image2.GetSize()
                              || image.GetTileMode() != image2.GetTileMode()
                              || image.GetAlignment() != image2.GetAlignment() )
                    {
                        success = false;
                        printe( "Size, tilemode, and/or alignment disagree:\n%ls\n", szPath );
                    }
                    else if ( memcmp( image.GetPointer(), image2.GetPointer(), image.GetSize() ) != 0 )
                    {
                        success = false;
                        printe( "Data mismatch on readback:\n%ls\n", szPath );
                    }
                    else
                    {
                        ++npass;
                    }
                }
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass );

    return success;
}


//-------------------------------------------------------------------------------------
// SaveDDSToFile
bool Test06()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        if (!g_TestMedia[index].isXbox)
            continue;

        WCHAR szPath[MAX_PATH];
        ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);

#ifdef _DEBUG
        OutputDebugString(szPath);
        OutputDebugStringA("\n");
#endif

        // Form dest path
        WCHAR ext[_MAX_EXT];
        WCHAR fname[_MAX_FNAME];
        _wsplitpath_s(szPath, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

        WCHAR tempDir[MAX_PATH];
        ExpandEnvironmentStringsW(L"%TEMP%\\xbox", tempDir, MAX_PATH);

        CreateDirectoryW(tempDir, NULL);

        WCHAR szDestPath[MAX_PATH];
        _wmakepath_s(szDestPath, MAX_PATH, NULL, tempDir, fname, L".dds");

        TexMetadata metadata;
        Xbox::XboxImage image;
        HRESULT hr = Xbox::LoadFromDDSFile(szPath, &metadata, image);
#ifdef _USE_SCARLETT
        if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            continue;
#endif

        if (FAILED(hr))
        {
            success = false;
            printe("Failed loading DDS from (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else
        {
            hr = Xbox::SaveToDDSFile(image, szDestPath);
            if (FAILED(hr))
            {
                success = false;
                printe("Failed writing DDS to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath);
            }
            else
            {
                TexMetadata metadata2;
                Xbox::XboxImage image2;
                hr = Xbox::LoadFromDDSFile(szDestPath, &metadata2, image2);
                if (FAILED(hr))
                {
                    success = false;
                    printe("Failed reading back written DDS to (HRESULT %08X):\n%ls\n", static_cast<unsigned int>(hr), szDestPath);
                }
                else
                {
                    if (memcmp(&metadata, &metadata2, sizeof(TexMetadata)) != 0)
                    {
                        success = false;
                        printe("Metadata error in dds readback:\n%ls\n", szDestPath);
                        printmeta(&metadata2);
                        printmetachk(&metadata);
                    }
                    else if (image.GetSize() != image2.GetSize()
                        || image.GetTileMode() != image2.GetTileMode()
                        || image.GetAlignment() != image2.GetAlignment())
                    {
                        success = false;
                        printe("Size, tilemode, and/or alignment disagree:\n%ls\n", szPath);
                    }
                    else if (memcmp(image.GetPointer(), image2.GetPointer(), image.GetSize()) != 0)
                    {
                        success = false;
                        printe("Data mismatch on readback:\n%ls\n", szPath);
                    }
                    else
                    {
                        ++npass;
                    }
                }
            }
        }

        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);

    return success;
}

