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

#include "DirectXTex.h"

namespace
{
    struct handle_closer { void operator()(HANDLE h) noexcept { if (h) CloseHandle(h); } };

    using ScopedHandle = std::unique_ptr<void, handle_closer>;

    inline HANDLE safe_handle(HANDLE h) noexcept { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }

    struct find_closer { void operator()(HANDLE h) noexcept { assert(h != INVALID_HANDLE_VALUE); if (h) FindClose(h); } };

    using ScopedFindHandle = std::unique_ptr<void, find_closer>;

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    enum OPTIONS
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

    struct SConversion
    {
        wchar_t szSrc[MAX_PATH] = {};
    };

    struct SValue
    {
        LPCWSTR pName;
        DWORD dwValue;
    };

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const SValue g_pOptions[] =
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

#ifdef _PREFAST_
#pragma prefast(disable : 26018, "Only used with static internal arrays")
#endif

    DWORD LookupByName(const wchar_t* pName, const SValue* pArray)
    {
        while (pArray->pName)
        {
            if (!_wcsicmp(pName, pArray->pName))
                return pArray->dwValue;

            pArray++;
        }

        return 0;
    }

    void SearchForFiles(const wchar_t* path, std::list<SConversion>& files, bool recursive)
    {
        // Process files
        WIN32_FIND_DATA findData = {};
        ScopedFindHandle hFile(safe_handle(FindFirstFileExW(path,
            FindExInfoBasic, &findData,
            FindExSearchNameMatch, nullptr,
            FIND_FIRST_EX_LARGE_FETCH)));
        if (hFile)
        {
            for (;;)
            {
                if (!(findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)))
                {
                    wchar_t drive[_MAX_DRIVE] = {};
                    wchar_t dir[_MAX_DIR] = {};
                    _wsplitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);

                    SConversion conv;
                    _wmakepath_s(conv.szSrc, drive, dir, findData.cFileName, nullptr);
                    files.push_back(conv);
                }

                if (!FindNextFile(hFile.get(), &findData))
                    break;
            }
        }

        // Process directories
        if (recursive)
        {
            wchar_t searchDir[MAX_PATH] = {};
            {
                wchar_t drive[_MAX_DRIVE] = {};
                wchar_t dir[_MAX_DIR] = {};
                _wsplitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);
                _wmakepath_s(searchDir, drive, dir, L"*", nullptr);
            }

            hFile.reset(safe_handle(FindFirstFileExW(searchDir,
                FindExInfoBasic, &findData,
                FindExSearchLimitToDirectories, nullptr,
                FIND_FIRST_EX_LARGE_FETCH)));
            if (!hFile)
                return;

            for (;;)
            {
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (findData.cFileName[0] != L'.')
                    {
                        wchar_t subdir[MAX_PATH] = {};

                        {
                            wchar_t drive[_MAX_DRIVE] = {};
                            wchar_t dir[_MAX_DIR] = {};
                            wchar_t fname[_MAX_FNAME] = {};
                            wchar_t ext[_MAX_FNAME] = {};
                            _wsplitpath_s(path, drive, dir, fname, ext);
                            wcscat_s(dir, findData.cFileName);
                            _wmakepath_s(subdir, drive, dir, fname, ext);
                        }

                        SearchForFiles(subdir, files, recursive);
                    }
                }

                if (!FindNextFile(hFile.get(), &findData))
                    break;
            }
        }
    }

    //-------------------------------------------------------------------------------------
    HRESULT LoadBlobFromFile(_In_z_ const wchar_t* szFile, DirectX::Blob& blob)
    {
        if (szFile == nullptr)
            return E_INVALIDARG;

        ScopedHandle hFile(safe_handle(CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
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
        wprintf(L"DirectXTex\n\n");
        wprintf(L"Usage: fuzzloaders <options> <files>\n");
        wprintf(L"\n");
        wprintf(L"   -r                  wildcard filename search is recursive\n");
        wprintf(L"   -dds                force use of DDS loader\n");
        wprintf(L"   -hdr                force use of HDR loader\n");
        wprintf(L"   -pfm                force use of PFM loader\n");
        wprintf(L"   -ppm                force use of PPM loader\n");
        wprintf(L"   -tga                force use of TGA loader\n");
        wprintf(L"   -wic                force use of WIC loader\n");
    }
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
            }
        }
        else if (wcspbrk(pArg, L"?*") != nullptr)
        {
            size_t count = conversion.size();
            SearchForFiles(pArg, conversion, (dwOptions & (1 << OPT_RECURSIVE)) != 0);
            if (conversion.size() <= count)
            {
                wprintf(L"No matching files found for %ls\n", pArg);
                return 1;
            }
        }
        else
        {
            SConversion conv;
            wcscpy_s(conv.szSrc, MAX_PATH, pArg);

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
        _wsplitpath_s(pConv.szSrc, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);
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
        OutputDebugStringW(pConv.szSrc);
        OutputDebugStringA("\n");
#endif

        DirectX::ScratchImage result;
        if (usedds)
        {
            constexpr DirectX::DDS_FLAGS ddsFlags = DirectX::DDS_FLAGS_ALLOW_LARGE_FILES;
            bool pass = false;

            hr = DirectX::LoadFromDDSFile(pConv.szSrc, ddsFlags, nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: DDSTexture file not not found:\n%ls\n", pConv.szSrc);
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
                hr = LoadBlobFromFile(pConv.szSrc, blob);
                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    wprintf(L"ERROR: DDSTexture file not not found:\n%ls\n", pConv.szSrc);
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
                    hr = DirectX::LoadFromDDSMemory(blob.GetBufferPointer(), blob.GetBufferSize(), ddsFlags, nullptr, result);
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
            hr = DirectX::LoadFromHDRFile(pConv.szSrc, nullptr, result); // LoadFromHDRFile exercises LoadFromHDRMemory
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: HDRTexture file not not found:\n%ls\n", pConv.szSrc);
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
            hr = LoadFromPortablePixMapHDR(pConv.szSrc, nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: PFMTexture file not not found:\n%ls\n", pConv.szSrc);
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
            hr = LoadFromPortablePixMap(pConv.szSrc, nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: PPMTexture file not not found:\n%ls\n", pConv.szSrc);
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

                hr = DirectX::LoadFromTGAFile(pConv.szSrc, tgaFlags, nullptr, result);
                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    wprintf(L"ERROR: TGATexture file not not found:\n%ls\n", pConv.szSrc);
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
                    hr = LoadBlobFromFile(pConv.szSrc, blob);
                    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    {
                        wprintf(L"ERROR: TGATexture file not not found:\n%ls\n", pConv.szSrc);
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
            hr = DirectX::LoadFromWICFile(pConv.szSrc, DirectX::WIC_FLAGS_NONE, nullptr, result);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                wprintf(L"ERROR: WICTexture file not found:\n%ls\n", pConv.szSrc);
                return 1;
            }
            else if (FAILED(hr) && hr != E_INVALIDARG && hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) && hr != WINCODEC_ERR_COMPONENTNOTFOUND && hr != E_OUTOFMEMORY && hr != WINCODEC_ERR_BADHEADER)
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
