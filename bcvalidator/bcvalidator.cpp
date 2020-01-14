//--------------------------------------------------------------------------------------
// File: bcvalidator.cpp
//
// Simple command-line tool for regression testing BC compression codecs
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
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

#include <windows.h>

#include <memory>
#include <list>

#include <wrl\client.h>

#include <d3d11.h>
#include <dxgi.h>
#include <dxgiformat.h>

#include "DirectXTex.h"

namespace
{
    struct handle_closer { void operator()(HANDLE h) { if (h) CloseHandle(h); } };

    typedef std::unique_ptr<void, handle_closer> ScopedHandle;

    inline HANDLE safe_handle(HANDLE h) { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }

    struct find_closer { void operator()(HANDLE h) { assert(h != INVALID_HANDLE_VALUE); if (h) FindClose(h); } };

    typedef std::unique_ptr<void, find_closer> ScopedFindHandle;
}

using namespace DirectX;
using Microsoft::WRL::ComPtr;

enum OPTIONS
{
    OPT_RECURSIVE = 1,
    OPT_CODEC,
    OPT_WRITE,
    OPT_FORCE_SINGLEPROC,
    OPT_GPU,
    OPT_NOGPU,
    OPT_COMPRESS_UNIFORM,
    OPT_COMPRESS_MAX,
    OPT_COMPRESS_QUICK,
    OPT_COMPRESS_DITHER,
    OPT_MAX
};

static_assert(OPT_MAX <= 32, "dwOptions is a DWORD bitfield");

struct SConversion
{
    wchar_t szSrc[MAX_PATH];
};

struct SValue
{
    LPCWSTR pName;
    DWORD dwValue;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

const SValue g_pOptions [] =
{
    { L"r",             OPT_RECURSIVE },
    { L"c",             OPT_CODEC },
    { L"w",             OPT_WRITE },
    { L"singleproc",    OPT_FORCE_SINGLEPROC },
    { L"gpu",           OPT_GPU },
    { L"nogpu",         OPT_NOGPU },
    { L"bcuniform",     OPT_COMPRESS_UNIFORM },
    { L"bcmax",         OPT_COMPRESS_MAX },
    { L"bcquick",       OPT_COMPRESS_QUICK },
    { L"bcdither",      OPT_COMPRESS_DITHER },
    { nullptr,          0 }
};

const SValue g_pCodecs[] =
{
    { L"BC1",   DXGI_FORMAT_BC1_UNORM },
    { L"BC2",   DXGI_FORMAT_BC2_UNORM },
    { L"BC3",   DXGI_FORMAT_BC3_UNORM },
    { L"BC4S",  DXGI_FORMAT_BC4_SNORM },
    { L"BC5S",  DXGI_FORMAT_BC5_SNORM },
    { L"BC4U",  DXGI_FORMAT_BC4_UNORM },
    { L"BC5U",  DXGI_FORMAT_BC5_UNORM },
    { L"BC6U",  DXGI_FORMAT_BC6H_UF16 },
    { L"BC6S",  DXGI_FORMAT_BC6H_SF16 },
    { L"BC7",   DXGI_FORMAT_BC7_UNORM },
    { nullptr,  0 }
};

const SValue g_pDirectCompute[] =
{
    { L"4.0", D3D_FEATURE_LEVEL_10_0 },
    { L"4.1", D3D_FEATURE_LEVEL_10_1 },
    { L"5.0", D3D_FEATURE_LEVEL_11_0 },
    { nullptr,  0 }
};

#define CODEC_DDS 0xFFFF0001 
#define CODEC_TGA 0xFFFF0002
#define CODEC_HDP 0xFFFF0003
#define CODEC_JXR 0xFFFF0004
#define CODEC_HDR 0xFFFF0005

const SValue g_pExtFileTypes[] =
{
    { L".BMP",  WIC_CODEC_BMP },
    { L".JPG",  WIC_CODEC_JPEG },
    { L".JPEG", WIC_CODEC_JPEG },
    { L".PNG",  WIC_CODEC_PNG },
    { L".DDS",  CODEC_DDS },
    { L".TGA",  CODEC_TGA },
    { L".HDR",  CODEC_HDR },
    { L".TIF",  WIC_CODEC_TIFF },
    { L".TIFF", WIC_CODEC_TIFF },
    { L".WDP",  WIC_CODEC_WMP },
    { L".HDP",  WIC_CODEC_WMP },
    { L".JXR",  WIC_CODEC_WMP },
    { nullptr,  CODEC_DDS }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

namespace
{
#pragma prefast(disable : 26018, "Only used with static internal arrays")

    DWORD LookupByName(const wchar_t *pName, const SValue *pArray)
    {
        while (pArray->pName)
        {
            if (!_wcsicmp(pName, pArray->pName))
                return pArray->dwValue;

            pArray++;
        }

        return 0;
    }

    const wchar_t* LookupByValue(DWORD pValue, const SValue *pArray)
    {
        while (pArray->pName)
        {
            if (pValue == pArray->dwValue)
                return pArray->pName;

            pArray++;
        }

        return L"";
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


    void PrintList(size_t cch, const SValue *pValue)
    {
        while (pValue->pName)
        {
            size_t cchName = wcslen(pValue->pName);

            if (cch + cchName + 2 >= 80)
            {
                wprintf(L"\n      ");
                cch = 6;
            }

            wprintf(L"%ls ", pValue->pName);
            cch += cchName + 2;
            pValue++;
        }

        wprintf(L"\n");
    }


    _Success_(return != false)
        bool GetDXGIFactory(_Outptr_ IDXGIFactory1** pFactory)
    {
        if (!pFactory)
            return false;

        *pFactory = nullptr;

        typedef HRESULT(WINAPI* pfn_CreateDXGIFactory1)(REFIID riid, _Out_ void **ppFactory);

        static pfn_CreateDXGIFactory1 s_CreateDXGIFactory1 = nullptr;

        if (!s_CreateDXGIFactory1)
        {
            HMODULE hModDXGI = LoadLibrary(L"dxgi.dll");
            if (!hModDXGI)
                return false;

            s_CreateDXGIFactory1 = reinterpret_cast<pfn_CreateDXGIFactory1>(reinterpret_cast<void*>(GetProcAddress(hModDXGI, "CreateDXGIFactory1")));
            if (!s_CreateDXGIFactory1)
                return false;
        }

        return SUCCEEDED(s_CreateDXGIFactory1(IID_PPV_ARGS(pFactory)));
    }

    void PrintUsage()
    {
        wprintf(L"Usage: bcvalidator <options> <files>\n");
        wprintf(L"\n");
        wprintf(L"   -r                  wildcard filename search is recursive\n");
        wprintf(L"   -c <codec>          Which codec to validate\n");
#ifdef _OPENMP
        wprintf(L"   -singleproc         Do not use multi-threaded compression\n");
#endif
        wprintf(L"   -gpu <adapter>      Select GPU for DirectCompute-based codecs (0 is default)\n");
        wprintf(L"   -nogpu              Do not use DirectCompute-based codecs\n");
        wprintf(L"   -bcuniform          Use uniform rather than perceptual weighting for BC1-3\n");
        wprintf(L"   -bcdither           Use dithering for BC1-3\n");
        wprintf(L"   -bcmax              Use exhaustive compression (BC7 only)\n");
        wprintf(L"   -bcquick            Use quick compression (BC7 only)\n");

        wprintf(L"\n   <codec>: ");
        PrintList(13, g_pCodecs);

        ComPtr<IDXGIFactory1> dxgiFactory;
        if (GetDXGIFactory(dxgiFactory.GetAddressOf()))
        {
            wprintf(L"\n   <adapter>:\n");

            ComPtr<IDXGIAdapter> adapter;
            for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC desc;
                if (SUCCEEDED(adapter->GetDesc(&desc)))
                {
                    wprintf(L"      %u: VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                }
            }
        }
    }

    _Success_(return != false)
        bool CreateDevice(int adapter, _Outptr_ ID3D11Device** pDevice)
    {
        if (!pDevice)
            return false;

        *pDevice = nullptr;

        static PFN_D3D11_CREATE_DEVICE s_DynamicD3D11CreateDevice = nullptr;

        if (!s_DynamicD3D11CreateDevice)
        {
            HMODULE hModD3D11 = LoadLibrary(L"d3d11.dll");
            if (!hModD3D11)
                return false;

            s_DynamicD3D11CreateDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(reinterpret_cast<void*>(GetProcAddress(hModD3D11, "D3D11CreateDevice")));
            if (!s_DynamicD3D11CreateDevice)
                return false;
        }

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        ComPtr<IDXGIAdapter> pAdapter;
        if (adapter >= 0)
        {
            ComPtr<IDXGIFactory1> dxgiFactory;
            if (GetDXGIFactory(dxgiFactory.GetAddressOf()))
            {
                if (FAILED(dxgiFactory->EnumAdapters(adapter, pAdapter.GetAddressOf())))
                {
                    wprintf(L"\nERROR: Invalid GPU adapter index (%d)!\n", adapter);
                    return false;
                }
            }
        }

        D3D_FEATURE_LEVEL fl;
        HRESULT hr = s_DynamicD3D11CreateDevice(pAdapter.Get(),
            (pAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
            nullptr, createDeviceFlags, featureLevels, _countof(featureLevels),
            D3D11_SDK_VERSION, pDevice, &fl, nullptr);
        if (SUCCEEDED(hr))
        {
            if (fl < D3D_FEATURE_LEVEL_11_0)
            {
                D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts;
                hr = (*pDevice)->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
                if (FAILED(hr))
                    memset(&hwopts, 0, sizeof(hwopts));

                if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
                {
                    if (*pDevice)
                    {
                        (*pDevice)->Release();
                        *pDevice = nullptr;
                    }
                    hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = (*pDevice)->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));
            if (SUCCEEDED(hr))
            {
                hr = dxgiDevice->GetAdapter(pAdapter.ReleaseAndGetAddressOf());
                if (SUCCEEDED(hr))
                {
                    DXGI_ADAPTER_DESC desc;
                    hr = pAdapter->GetDesc(&desc);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"\n[Using DirectCompute %ls on \"%ls\"]\n", LookupByValue(fl, g_pDirectCompute), desc.Description );
                    }
                }
            }

            return true;
        }
        else
            return false;
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
// Entry-point
//--------------------------------------------------------------------------------------
#pragma prefast(disable : 28198, "Command-line tool, frees all memory on exit")

int __cdecl wmain(_In_ int argc, _In_z_count_(argc) wchar_t* argv[])
{
    // Parameters and defaults
    DXGI_FORMAT format = DXGI_FORMAT_BC3_UNORM;
    DWORD dwCompress = TEX_COMPRESS_DEFAULT;
    int adapter = -1;

    // Initialize COM (needed for WIC)
    HRESULT hr = hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        wprintf(L"Failed to initialize COM (%08X)\n", hr);
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

            // Handle options with additional value parameter
            switch (dwOption)
            {
            case OPT_CODEC:
            case OPT_GPU:
                if (!*pValue)
                {
                    if ((iArg + 1 >= argc))
                    {
                        PrintUsage();
                        return 1;
                    }

                    iArg++;
                    pValue = argv[iArg];
                }
                break;
            }

            switch (dwOption)
            {
            case OPT_CODEC:
                format = static_cast<DXGI_FORMAT>(LookupByName(pValue, g_pCodecs));
                break;

            case OPT_GPU:
                if (swscanf_s(pValue, L"%d", &adapter) != 1)
                {
                    wprintf(L"Invalid value specified with -gpu (%ls)\n\n", pValue);
                    PrintUsage();
                    return 1;
                }
                else if (adapter < 0)
                {
                    wprintf(L"Adapter index (%ls)\n\n", pValue);
                    PrintUsage();
                    return 1;
                }
                break;

            case OPT_COMPRESS_UNIFORM:
                dwCompress |= TEX_COMPRESS_UNIFORM;
                break;

            case OPT_COMPRESS_MAX:
                if (dwCompress & TEX_COMPRESS_BC7_QUICK)
                {
                    wprintf(L"Can't use -bcmax and -bcquick at same time\n\n");
                    PrintUsage();
                    return 1;
                }
                dwCompress |= TEX_COMPRESS_BC7_USE_3SUBSETS;
                break;

            case OPT_COMPRESS_QUICK:
                if (dwCompress & TEX_COMPRESS_BC7_USE_3SUBSETS)
                {
                    wprintf(L"Can't use -bcmax and -bcquick at same time\n\n");
                    PrintUsage();
                    return 1;
                }
                dwCompress |= TEX_COMPRESS_BC7_QUICK;
                break;

            case OPT_COMPRESS_DITHER:
                dwCompress |= TEX_COMPRESS_DITHER;
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
        wprintf(L"ERROR: Need at least 1 image file to validate\n\n");
        PrintUsage();
        return 0;
    }

    LARGE_INTEGER qpcFreq;
    if (!QueryPerformanceFrequency(&qpcFreq))
    {
        qpcFreq.QuadPart = 0;
    }

    double qpcInvFreq = 1000.0 / double(qpcFreq.QuadPart);

    ComPtr<ID3D11Device> pDevice;

    for (auto pConv = conversion.begin(); pConv != conversion.end(); ++pConv)
    {
        wchar_t ext[_MAX_EXT];
        wchar_t fname[_MAX_FNAME];
        _wsplitpath_s(pConv->szSrc, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

        // Load source image
#ifdef _DEBUG
        OutputDebugStringW(pConv->szSrc);
        OutputDebugStringA("\n");
#endif

        TexMetadata info;
        std::unique_ptr<ScratchImage> image(new (std::nothrow) ScratchImage);

        if (!image)
        {
            wprintf(L"\nERROR: Memory allocation failed\n");
            return 1;
        }

        if (_wcsicmp(ext, L".dds") == 0)
        {
            hr = LoadFromDDSFile(pConv->szSrc, DDS_FLAGS_NONE, &info, *image);
            if (FAILED(hr))
            {
                wprintf(L" FAILED (%x)\n", hr);
                return 1;
            }

            if (IsCompressed(info.format))
            {
                wprintf(L"ERROR: Input images must be uncompressed!\n");
                return 1;
            }
        }
        else if (_wcsicmp(ext, L".tga") == 0)
        {
            hr = LoadFromTGAFile(pConv->szSrc, &info, *image);
            if (FAILED(hr))
            {
                wprintf(L" FAILED (%x)\n", hr);
                return 1;
            }
        }
        else if (_wcsicmp(ext, L".hdr") == 0)
        {
            hr = LoadFromHDRFile(pConv->szSrc, &info, *image);
            if (FAILED(hr))
            {
                wprintf(L" FAILED (%x)\n", hr);
                return 1;
            }
        }
        else
        {
            hr = LoadFromWICFile(pConv->szSrc, WIC_FLAGS_NONE, &info, *image);
            if (FAILED(hr))
            {
                wprintf(L" FAILED (%x)\n", hr);
                return 1;
            }
        }

        // Handle sRGB input
        DXGI_FORMAT bcformat = format;
        if (IsSRGB(info.format))
            bcformat = MakeSRGB(format);

        // Perform compression
        bool bc6hbc7 = false;
        switch (bcformat)
        {
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bc6hbc7 = true;

            {
                static bool s_tryonce = false;

                if (!s_tryonce)
                {
                    s_tryonce = true;

                    if (!(dwOptions & (DWORD64(1) << OPT_NOGPU)))
                    {
                        if (!CreateDevice(adapter, pDevice.GetAddressOf()))
                            wprintf(L"\nWARNING: DirectCompute is not available, using BC6H / BC7 CPU codec\n");
                    }
                    else
                    {
                        wprintf(L"\nWARNING: using BC6H / BC7 CPU codec\n");
                    }
                }
            }
            break;
        }

        DWORD cflags = dwCompress;
#ifdef _OPENMP
        if (!(dwOptions & (DWORD64(1) << OPT_FORCE_SINGLEPROC)))
        {
            cflags |= TEX_COMPRESS_PARALLEL;
        }
#endif

        std::unique_ptr<ScratchImage> cimage(new (std::nothrow) ScratchImage);
        if (!cimage)
        {
            wprintf(L"\nERROR: Memory allocation failed\n");
            return 1;
        }

        LONGLONG timeDelta = 0;
        {
            LARGE_INTEGER qpcStart;
            if (!QueryPerformanceCounter(&qpcStart))
            {
                qpcStart.QuadPart = 0;
            }

            if (bc6hbc7 && pDevice)
            {
                hr = Compress(pDevice.Get(), image->GetImage(0, 0, 0), image->GetImageCount(), info, bcformat, dwCompress, 1.0f, *cimage);
            }
            else
            {
                hr = Compress(image->GetImage(0, 0, 0), image->GetImageCount(), info, bcformat, cflags, TEX_THRESHOLD_DEFAULT, *cimage);
            }
            if (FAILED(hr))
            {
                wprintf(L" FAILED [compress] (%x)\n", hr);
                return 1;
            }

            LARGE_INTEGER qpcEnd;
            if (QueryPerformanceCounter(&qpcEnd))
            {
                timeDelta = qpcEnd.QuadPart - qpcStart.QuadPart;
            }

            auto cinfo = cimage->GetMetadata();
            if (cinfo.width != info.width
                || cinfo.height != info.height)
            {
                wprintf(L"ERROR: Compression metadata invalid!\n");
                return 1;
            }
        }

        // Decompress and compare

        float mse = 0, mseV[4] = {};
        std::unique_ptr<ScratchImage> image2(new (std::nothrow) ScratchImage);
        {
            hr = Decompress(cimage->GetImage(0, 0, 0), cimage->GetImageCount(), cimage->GetMetadata(), DXGI_FORMAT_UNKNOWN /* picks good default */, *image2);
            if (FAILED(hr))
            {
                wprintf(L" FAILED [decompress] (%x)\n", hr);
                return 1;
            }

            auto info2 = image2->GetMetadata();
            if (info2.width != info.width
                || info2.height != info.height)
            {
                wprintf(L"ERROR: Decompression metadata invalid!\n");
                return 1;
            }

            // Note we are only comparing the first image in a complex image!
            hr = ComputeMSE(*image->GetImage(0, 0, 0), *image2->GetImage(0, 0, 0), mse, mseV);
            if (FAILED(hr))
            {
                wprintf(L"ERROR: Failed comparing input image with decompressed image\n");
                return 1;
            }
        }

        wprintf(L"\nImage: %ls%ls\n", fname, ext);

        wprintf(L"\tCompression: %ls (%ls)\n", LookupByValue(format, g_pCodecs), (bc6hbc7 && pDevice) ? L"GPU" : L"CPU");

        if (timeDelta > 0)
        {
            wprintf(L"\tCompression time: %.0f ms\n", double(timeDelta) * qpcInvFreq);
        }

        wprintf(L"\tMSE: %f (%f %f %f %f)\n\tPSNR %f dB\n", mse, mseV[0], mseV[1], mseV[2], mseV[3],
            10.0 * log10(3.0 / (double(mseV[0]) + double(mseV[1]) + double(mseV[2]))));

        // Consider adding SSIM metric...
        // Zhou, W., A. C. Bovik, H. R. Sheikh, and E. P. Simoncelli.
        // "Image Qualifty Assessment: From Error Visibility to Structural Similarity."
        // IEEE Transactions on Image Processing. Vol. 13, Issue 4, April 2004, pp. 600-612.

        if (dwOptions & (1 << OPT_WRITE))
        {
            DWORD fileType = LookupByName(ext, g_pExtFileTypes);
            if (!fileType)
            {
                wprintf(L"ERROR: Unknown file extension %ls\n", ext);
                return 1;
            }

            wchar_t fname2[_MAX_FNAME];
            wcscpy_s(fname2, fname);
            wcscat_s(fname2, L"_result");

            wchar_t destPath[_MAX_PATH];
            _wmakepath_s(destPath, nullptr, nullptr, fname2, ext);

            switch (fileType)
            {
            case CODEC_DDS:
                hr = SaveToDDSFile(image2->GetImage(0, 0, 0), image2->GetImageCount(), image2->GetMetadata(), DDS_FLAGS_NONE, destPath);
                break;

            case CODEC_TGA:
                hr = SaveToTGAFile(*image2->GetImage(0, 0, 0), destPath);
                break;

            case CODEC_HDR:
                hr = SaveToHDRFile(*image2->GetImage(0, 0, 0), destPath);
                break;

            default:
            {
                WICCodecs codec = (fileType == CODEC_HDP || fileType == CODEC_JXR) ? WIC_CODEC_WMP : static_cast<WICCodecs>(fileType);
                hr = SaveToWICFile(image2->GetImage(0, 0, 0), image2->GetImageCount(), WIC_FLAGS_NONE, GetWICCodec(codec), destPath);
                break;
            }
            }

            if (FAILED(hr))
            {
                wprintf(L"ERROR: Failed to write result file\n:%ls\n", destPath);
                return 1;
            }
        }
    }

    wprintf(L"\n");

    return 0;
}
