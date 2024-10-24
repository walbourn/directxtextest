// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifdef _MSC_VER
#pragma warning(disable : 4619 4616 4430)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning(push)
#pragma warning(disable : 4005)
#define NOMINMAX 1
#define NODRAWTEXT
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#include <Windows.h>

#ifdef __MINGW32__
#include <unknwn.h>
#endif

#include <cstdint>

// Need to define once to avoid ODR warnings.

namespace DirectX
{
#define WIC_LOADER_FLAGS_DEFINED
    enum WIC_LOADER_FLAGS : uint32_t
    {
        WIC_LOADER_DEFAULT = 0,
        WIC_LOADER_FORCE_SRGB = 0x1,
        WIC_LOADER_IGNORE_SRGB = 0x2,
        WIC_LOADER_SRGB_DEFAULT = 0x4,
        WIC_LOADER_MIP_AUTOGEN = 0x8,
        WIC_LOADER_MIP_RESERVE = 0x10,
        WIC_LOADER_FIT_POW2 = 0x20,
        WIC_LOADER_MAKE_SQUARE = 0x40,
        WIC_LOADER_FORCE_RGBA32 = 0x80,
    };

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#endif

    DEFINE_ENUM_FLAG_OPERATORS(WIC_LOADER_FLAGS)

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}
