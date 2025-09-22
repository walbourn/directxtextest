//-------------------------------------------------------------------------------------
// DirectXTest.h
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#pragma once

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#pragma warning(push)
#pragma warning(disable : 4005)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#define NODRAWTEXT
#define NOGDI
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#ifndef __MINGW32__
#define _CRTDBG_MAP_ALLOC
#endif

#include <crtdbg.h>

#include <Windows.h>
#endif

#ifdef __MINGW32__
#include <unknwn.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cwchar>
#include <iterator>
#include <tuple>

#define _XM_NO_XMVECTOR_OVERLOADS_
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#define _DIRECTX_TEST_NAME_ "DirectXTex"

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#define print printf
#define printe printf
#else
inline void DebugPrint(_In_z_ _Printf_format_string_ const char* format, ...) noexcept
{
    va_list args;
    va_start(args, format);

    char buff[1024] = {};
    vsprintf_s(buff, format, args);
    OutputDebugStringA(buff);
    va_end(args);
}

#define print DebugPrint
#define printe DebugPrint
#endif

#define printxmv(v) print("%s: %f,%f,%f,%f\n", #v, XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v), XMVectorGetW(v))

#define printmeta(md) print( "(actual) w: %zu x h: %zu x d: %zu\narraySize: %zu  mips: %zu\nmiscFlags: %08X miscFlags2: %08X format: %d dimension: %u\n", \
                              (md)->width, (md)->height, (md)->depth, (md)->arraySize, (md)->mipLevels, (md)->miscFlags, (md)->miscFlags2, (md)->format, (md)->dimension );

#define printdds(md) print( " size: %u  flags: %08X   fourCC %08X\nbitCount %u   %08X/%08X/%08X/%08X\n", \
                              md.size, md.flags, md.fourCC, md.RGBBitCount, md.RBitMask, md.GBitMask, md.BBitMask, md.ABitMask);

#define printmetachk(md) print( "...\n(check) w: %zu x h: %zu x d: %zu\narraySize: %zu  mips: %zu\nmiscFlags: %08X miscFlags2: %08X format: %d dimension: %u\n", \
                              (md)->width, (md)->height, (md)->depth, (md)->arraySize, (md)->mipLevels, (md)->miscFlags, (md)->miscFlags2, (md)->format, (md)->dimension );

#define printdigest(str,digest) print( "%s:\n0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\n", str, \
                                       digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7], \
                                       digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15] );

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#define MEDIA_PATH L"%DIRECTXTEX_MEDIA_PATH%\\"
#define TEMP_PATH L"%TEMP%\\"
#elif (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#define MEDIA_PATH L"Assets\\"
#define TEMP_PATH L"%TEMP%\\"
#elif defined(_GAMING_XBOX)
#define MEDIA_PATH L"Assets\\"
#define TEMP_PATH L"S:\\dxtmp"
#else
#define MEDIA_PATH L"Assets\\"
#define TEMP_PATH L"T:\\"
#endif

// See https://walbourn.github.io/modern-c++-bitmask-types/
#ifndef ENUM_FLAGS_CONSTEXPR
#if defined(NTDDI_WIN10_RS1) && !defined(__MINGW32__)
#define ENUM_FLAGS_CONSTEXPR constexpr
#else
#define ENUM_FLAGS_CONSTEXPR const
#endif
#endif

//#define VERBOSE
