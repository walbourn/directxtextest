//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl/client.h>

#define DIRECT3D_VERSION 0x900

#include <d3d9.h>

#define _XM_NO_XMVECTOR_OVERLOADS_
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

#define _WIN7_PLATFORM_UPDATE
#include <wincodec.h>

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }
}
