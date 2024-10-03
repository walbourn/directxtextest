//--------------------------------------------------------------------------------------
// File: fuzzloaders.cpp
//
// Simple command-line tool for fuzz-testing for image loaders
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable : 4005)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#include <Windows.h>

#include <cassert>
#include <cstdio>
#include <cwchar>
#include <memory>
#include <list>
#include <tuple>

#include "DirectXTex.h"

// See https://walbourn.github.io/modern-c++-bitmask-types/
#ifndef ENUM_FLAGS_CONSTEXPR
#if defined(NTDDI_WIN10_RS1) && !defined(__MINGW32__)
#define ENUM_FLAGS_CONSTEXPR constexpr
#else
#define ENUM_FLAGS_CONSTEXPR const
#endif
#endif

#define TOOL_VERSION DIRECTX_TEX_VERSION
#include "CmdLineHelpers.h"

using namespace Helpers;


namespace
{
    const wchar_t* g_ToolName = L"fuzzloaders";
    const wchar_t* g_Description = L"Microsoft (R) DirectX Texture File Fuzzing Harness";

    ENUM_FLAGS_CONSTEXPR DirectX::DDS_FLAGS c_ddsFlags = DirectX::DDS_FLAGS_ALLOW_LARGE_FILES | DirectX::DDS_FLAGS_PERMISSIVE;

#ifndef FUZZING_BUILD_MODE

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    enum OPTIONS : uint32_t
    {
        OPT_RECURSIVE = 1,
        OPT_DDS,
        OPT_HDR,
        OPT_PFM,
        OPT_PPM,
        OPT_TGA,
        OPT_WIC,
        OPT_MAX
    };

    static_assert(OPT_MAX <= 32, "dwOptions is a DWORD bitfield");

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const SValue<uint32_t> g_pOptions[] =
    {
        { L"r",         OPT_RECURSIVE },
        { L"dds",       OPT_DDS },
        { L"hdr",       OPT_HDR },
        { L"pfm",       OPT_PFM },
        { L"ppm",       OPT_PPM },
        { L"tga",       OPT_TGA },
        { L"wic",       OPT_WIC },
        { nullptr,      0 }
    };

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    //-------------------------------------------------------------------------------------
    HRESULT LoadBlobFromFile(_In_z_ const wchar_t* szFile, DirectX::Blob& blob)
    {
        if (szFile == nullptr)
            return E_INVALIDARG;

        ScopedHandle hFile(safe_handle(CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN, nullptr)));
        if (!hFile)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        // Get the file size
        LARGE_INTEGER fileSize = {};
        if (!GetFileSizeEx(hFile.get(), &fileSize))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        // File is too big for 32-bit allocation, so reject read (4 GB should be plenty large enough for our test images)
        if (fileSize.HighPart > 0)
        {
            return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
        }

        // Need at least 1 byte of data
        if (!fileSize.LowPart)
        {
            return E_FAIL;
        }

        // Create blob memory
        HRESULT hr = blob.Initialize(fileSize.LowPart);
        if (FAILED(hr))
            return hr;

        // Load entire file into blob memory
        DWORD bytesRead = 0;
        if (!ReadFile(hFile.get(), blob.GetBufferPointer(), static_cast<DWORD>(blob.GetBufferSize()), &bytesRead, nullptr))
        {
            blob.Release();
            return HRESULT_FROM_WIN32(GetLastError());
        }

        // Verify we got the whole blob loaded
        if (bytesRead != blob.GetBufferSize())
        {
            blob.Release();
            return E_FAIL;
        }

        return S_OK;
    }

    void PrintUsage()
    {
        PrintLogo(false, g_ToolName, g_Description);

        static const wchar_t* const s_usage =
            L"Usage: fuzzloaders <options> <files>\n"
            L"\n"
            L"   -r                  wildcard filename search is recursive\n"
            L"   -dds                force use of DDS loader\n"
            L"   -hdr                force use of HDR loader\n"
            L"   -pfm                force use of PFM loader\n"
            L"   -ppm                force use of PPM loader\n"
            L"   -tga                force use of TGA loader\n"
            L"   -wic                force use of WIC loader\n";

        wprintf(L"%ls", s_usage);
    }

#endif // !FUZZING_BUILD_MODE
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

HRESULT __cdecl LoadFromPortablePixMap(
    _In_z_ const wchar_t* szFile,
    _Out_opt_ DirectX::TexMetadata* metadata,
    _Out_ DirectX::ScratchImage& image) noexcept;

HRESULT __cdecl LoadFromPortablePixMapHDR(
    _In_z_ const wchar_t* szFile,
    _Out_opt_ DirectX::TexMetadata* metadata,
    _Out_ DirectX::ScratchImage& image) noexcept;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
// Entry-point
//--------------------------------------------------------------------------------------
#ifndef FUZZING_BUILD_MODE

#ifdef _PREFAST_
#pragma prefast(disable : 28198, "Command-line tool, frees all memory on exit")
#endif

int __cdecl wmain(_In_ int argc, _In_z_count_(argc) wchar_t* argv[])
{
    // Initialize COM (needed for WIC)
    HRESULT hr = hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        wprintf(L"Failed to initialize COM (%08X)\n", static_cast<unsigned int>(hr));
        return 1;
    }

    // Process command line
    DWORD dwOptions = 0;
    std::list<SConversion> conversion;

    for (int iArg = 1; iArg < argc; iArg++)
    {
        PWSTR pArg = argv[iArg];

        if (('-' == pArg[0]) || ('/' == pArg[0]))
        {
            pArg++;
            PWSTR pValue;

            for (pValue = pArg; *pValue && (':' != *pValue); pValue++);

            if (*pValue)
                *pValue++ = 0;

            DWORD dwOption = LookupByName(pArg, g_pOptions);

            if (!dwOption || (dwOptions & (1 << dwOption)))
            {
                PrintUsage();
                return 1;
            }

            dwOptions |= 1 << dwOption;

            switch (dwOption)
            {
            case OPT_DDS:
            case OPT_HDR:
            case OPT_PFM:
            case OPT_PPM:
            case OPT_TGA:
            case OPT_WIC:
                {
                    DWORD mask = (1 << OPT_DDS)
                        | (1 << OPT_HDR)
                        | (1 << OPT_PFM)
                        | (1 << OPT_PPM)
                        | (1 << OPT_TGA)
                        | (1 << OPT_WIC);
                    mask &= ~(1 << dwOption);
                    if (dwOptions & mask)
                    {
                        wprintf(L"-dds, -hdr, -pfm, -ppm, -tga, and -wic are mutually exclusive options\n");
                        return 1;
                    }
                }
                break;

            default:
                break;
            }
        }
        else if (wcspbrk(pArg, L"?*") != nullptr)
        {
            size_t count = conversion.size();
            SearchForFiles(pArg, conversion, (dwOptions & (1 << OPT_RECURSIVE)) != 0, nullptr);
            if (conversion.size() <= count)
            {
                wprintf(L"No matching files found for %ls\n", pArg);
                return 1;
            }
        }
        else
        {
            SConversion conv = {};
            conv.szSrc = pArg;

            conversion.push_back(conv);
        }
    }

    if (conversion.empty())
    {
        wprintf(L"ERROR: Need at least 1 image file to fuzz\n\n");
        PrintUsage();
        return 0;
    }

    for (auto& pConv : conversion)
    {
        wchar_t ext[_MAX_EXT];
        _wsplitpath_s(pConv.szSrc.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);
        bool isdds = (_wcsicmp(ext, L".dds") == 0);
        bool ishdr = (_wcsicmp(ext, L".hdr") == 0);
        bool ispfm = (_wcsicmp(ext, L".pfm") == 0);
        bool isppm = (_wcsicmp(ext, L".ppm") == 0);
        bool istga = (_wcsicmp(ext, L".tga") == 0);

        bool usedds = false;
        bool usehdr = false;
        bool usepfm = false;
        bool useppm = false;
        bool usetga = false;
        if (dwOptions & (1 << OPT_DDS))
        {
            usedds = true;
        }
        else if (dwOptions & (1 << OPT_TGA))
        {
            usetga = true;
        }
        else if (dwOptions & (1 << OPT_HDR))
        {
            usehdr = true;
        }
        else if (dwOptions & (1 << OPT_PFM))
        {
            usepfm = true;
        }
        else if (dwOptions & (1 << OPT_PPM))
        {
            useppm = true;
        }
        else if (!(dwOptions & (1 << OPT_WIC)))
        {
            usedds = isdds;
            usehdr = ishdr;
            usepfm = ispfm;
            useppm = isppm;
            usetga = istga;
        }

        // Load source image
#ifdef _DEBUG
        OutputDebugStringW(pConv.szSrc.c_str());
        OutputDebugStringA("\n");
#endif

        DirectX::ScratchImage result;
        if (usedds)
        {
            bool pass = false;

            hr = DirectX::LoadFromDDSFile(pConv.szSrc.c_str(), c_ddsFlags, nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: DDSTexture file not not found:\n%ls\n", pConv.szSrc.c_str());
                return 1;
            }
            else if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) && (hr != E_FAIL || (hr == E_FAIL && isdds)))
            {
#ifdef _DEBUG
                char buff[128] = {};
                sprintf_s(buff, "DDSTextureFromFile failed with %08X\n", static_cast<unsigned int>(hr));
                OutputDebugStringA(buff);
#endif
                wprintf(L"!");
            }
            else
            {
                pass = true;
            }

            // Validate memory version
            {
                DirectX::Blob blob;
                hr = LoadBlobFromFile(pConv.szSrc.c_str(), blob);

                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    wprintf(L"ERROR: DDSTexture file not not found:\n%ls\n", pConv.szSrc.c_str());
                    return 1;
                }
                if (FAILED(hr) && hr != E_OUTOFMEMORY)
                {
#ifdef _DEBUG
                    char buff[128] = {};
                    sprintf_s(buff, "LoadBlobFromFile failed with %08X\n", static_cast<unsigned int>(hr));
                    OutputDebugStringA(buff);
#endif
                    wprintf(L"!");
                }
                else
                {
                    hr = DirectX::LoadFromDDSMemory(blob.GetBufferPointer(), blob.GetBufferSize(), c_ddsFlags, nullptr, result);
                    if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) && (hr != E_FAIL || (hr == E_FAIL && isdds)))
                    {
#ifdef _DEBUG
                        char buff[128] = {};
                        sprintf_s(buff, "DDSTextureFromMemory failed with %08X\n", static_cast<unsigned int>(hr));
                        OutputDebugStringA(buff);
#endif
                        wprintf(L"!");
                    }
                    else if (pass)
                    {
                        wprintf(L"%ls", SUCCEEDED(hr) ? L"*" : L".");
                    }
                }
            }
        }
        else if (usehdr)
        {
            hr = DirectX::LoadFromHDRFile(pConv.szSrc.c_str(), nullptr, result); // LoadFromHDRFile exercises LoadFromHDRMemory
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: HDRTexture file not not found:\n%ls\n", pConv.szSrc.c_str());
                return 1;
            }
            else if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) && (hr != E_FAIL || (hr == E_FAIL && ishdr)))
            {
#ifdef _DEBUG
                char buff[128] = {};
                sprintf_s(buff, "HDRTexture failed with %08X\n", static_cast<unsigned int>(hr));
                OutputDebugStringA(buff);
#endif
                wprintf(L"!");
            }
            else
            {
                wprintf(L"%ls", SUCCEEDED(hr) ? L"*" : L".");
            }
        }
        else if (usepfm)
        {
            hr = LoadFromPortablePixMapHDR(pConv.szSrc.c_str(), nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: PFMTexture file not not found:\n%ls\n", pConv.szSrc.c_str());
                return 1;
            }
            else if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) && (hr != E_FAIL || (hr == E_FAIL && ispfm)))
            {
#ifdef _DEBUG
                char buff[128] = {};
                sprintf_s(buff, "PFMTexture failed with %08X\n", static_cast<unsigned int>(hr));
                OutputDebugStringA(buff);
#endif
                wprintf(L"!");
            }
            else
            {
                wprintf(L"%ls", SUCCEEDED(hr) ? L"*" : L".");
            }
        }
        else if (useppm)
        {
            hr = LoadFromPortablePixMap(pConv.szSrc.c_str(), nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: PPMTexture file not not found:\n%ls\n", pConv.szSrc.c_str());
                return 1;
            }
            else if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) && (hr != E_FAIL || (hr == E_FAIL && isppm)))
            {
#ifdef _DEBUG
                char buff[128] = {};
                sprintf_s(buff, "PPMTexture failed with %08X\n", static_cast<unsigned int>(hr));
                OutputDebugStringA(buff);
#endif
                wprintf(L"!");
            }
            else
            {
                wprintf(L"%ls", SUCCEEDED(hr) ? L"*" : L".");
            }
        }
        else if (usetga)
        {
            for (int j = 0; j < 2; ++j)
            {
                DirectX::TGA_FLAGS tgaFlags = !j ? DirectX::TGA_FLAGS_NONE : DirectX::TGA_FLAGS_BGR;
                bool pass = false;

                hr = DirectX::LoadFromTGAFile(pConv.szSrc.c_str(), tgaFlags, nullptr, result);
                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    wprintf(L"ERROR: TGATexture file not not found:\n%ls\n", pConv.szSrc.c_str());
                    return 1;
                }
                else if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) && (hr != E_FAIL || (hr == E_FAIL && istga)))
                {
#ifdef _DEBUG
                    char buff[128] = {};
                    sprintf_s(buff, "TGATextureFromFile case %d failed with %08X\n", j, static_cast<unsigned int>(hr));
                    OutputDebugStringA(buff);
#endif
                    wprintf(L"!");
                }
                else
                {
                    pass = true;
                }

                // Validate memory version
                {
                    DirectX::Blob blob;
                    hr = LoadBlobFromFile(pConv.szSrc.c_str(), blob);
                    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    {
                        wprintf(L"ERROR: TGATexture file not not found:\n%ls\n", pConv.szSrc.c_str());
                        return 1;
                    }
                    if (FAILED(hr) && hr != E_OUTOFMEMORY)
                    {
#ifdef _DEBUG
                        char buff[128] = {};
                        sprintf_s(buff, "LoadBlobFromFile failed with %08X\n", static_cast<unsigned int>(hr));
                        OutputDebugStringA(buff);
#endif
                        wprintf(L"!");
                    }
                    else
                    {
                        hr = DirectX::LoadFromTGAMemory(blob.GetBufferPointer(), blob.GetBufferSize(), tgaFlags, nullptr, result);
                        if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != E_OUTOFMEMORY && hr != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) && (hr != E_FAIL || (hr == E_FAIL && istga)))
                        {
#ifdef _DEBUG
                            char buff[128] = {};
                            sprintf_s(buff, "TGATextureFromMemory failed with %08X\n", static_cast<unsigned int>(hr));
                            OutputDebugStringA(buff);
#endif
                            wprintf(L"!");
                        }
                        else if (pass)
                        {
                            wprintf(L"%ls", SUCCEEDED(hr) ? L"*" : L".");
                        }
                    }
                }
            }
        }
        else
        {
            hr = DirectX::LoadFromWICFile(pConv.szSrc.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: WICTexture file not found:\n%ls\n", pConv.szSrc.c_str());
                return 1;
            }
            else if (FAILED(hr)
                     && hr != E_INVALIDARG
                     && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)
                     && hr != WINCODEC_ERR_COMPONENTNOTFOUND
                     && hr != WINCODEC_ERR_COMPONENTINITIALIZEFAILURE
                     && hr != WINCODEC_ERR_BADHEADER
                     && hr != E_OUTOFMEMORY)
            {
#ifdef _DEBUG
                char buff[128] = {};
                sprintf_s(buff, "WICTexture failed with %08X\n", static_cast<unsigned int>(hr));
                OutputDebugStringA(buff);
#endif
                wprintf(L"!");
            }
            else
            {
                wprintf(L"%ls", SUCCEEDED(hr) ? L"*" : L".");
            }
        }
        fflush(stdout);
    }

    wprintf(L"\n*** FUZZING COMPLETE ***\n");

    return 0;
}


#else // FUZZING_BUILD_MODE


//--------------------------------------------------------------------------------------
// Libfuzzer entry-point
//--------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    DirectX::ScratchImage result;

    // Validate memory version
    std::ignore = DirectX::LoadFromDDSMemory(data, size, c_ddsFlags, nullptr, result);
    std::ignore = DirectX::LoadFromTGAMemory(data, size, DirectX::TGA_FLAGS_NONE, nullptr, result);
    std::ignore = DirectX::LoadFromTGAMemory(data, size, DirectX::TGA_FLAGS_BGR, nullptr, result);
    std::ignore = LoadFromHDRMemory(data, size, nullptr, result);

    // Disk version
    wchar_t tempFileName[MAX_PATH] = {};
    wchar_t tempPath[MAX_PATH] = {};

    if (!GetTempPathW(MAX_PATH, tempPath))
        return 0;

    if (!GetTempFileNameW(tempPath, L"fuzz", 0, tempFileName))
        return 0;

    {
        ScopedHandle hFile(safe_handle(CreateFileW(tempFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL, nullptr)));
        if (!hFile)
            return 0;

        DWORD bytesWritten = 0;
        if (!WriteFile(hFile.get(), data, static_cast<DWORD>(size), &bytesWritten, nullptr))
            return 0;
    }

    std::ignore = DirectX::LoadFromDDSFile(tempFileName, c_ddsFlags, nullptr, result);
    std::ignore = DirectX::LoadFromHDRFile(tempFileName, nullptr, result);
    std::ignore = DirectX::LoadFromTGAFile(tempFileName, DirectX::TGA_FLAGS_NONE, nullptr, result);
    std::ignore = DirectX::LoadFromTGAFile(tempFileName, DirectX::TGA_FLAGS_BGR, nullptr, result);
    std::ignore = LoadFromPortablePixMap(tempFileName, nullptr, result);
    std::ignore = LoadFromPortablePixMapHDR(tempFileName, nullptr, result);

    return 0;
}

#endif // FUZZING_BUILD_MODE
