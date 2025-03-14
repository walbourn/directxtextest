//-------------------------------------------------------------------------------------
// swizzle.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include <assert.h>

#include "directxtest.h"
#include "tex.h"

#include "DirectXTexP.h"
#include "scoped.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------

extern HRESULT MD5Checksum(_In_ const ScratchImage& image, _Out_bytecap_x_(16) uint8_t *digest, size_t nimages = 0);

//-------------------------------------------------------------------------------------

namespace
{
    template <DXGI_FORMAT F, typename T> HRESULT FillTexture(size_t w, size_t h, size_t d, ScratchImage& result)
    {
        if (d != 1)
            return E_NOTIMPL;

        HRESULT hr = result.Initialize2D(F, w, h, 1, 1);
        if (FAILED(hr))
            return hr;

        auto img = result.GetImage(0, 0, 0);
        uint8_t val = 1;
        uint8_t* ptr = img->pixels;
        if (!ptr)
            return E_POINTER;

        for (size_t y = 0; y < h; ++y)
        {
            T* p = reinterpret_cast<T*>(ptr);
            for (size_t x = 0; x < w; ++x)
            {
                *(p++) = val++;
            }
            ptr += img->rowPitch;
        }

        return S_OK;
    }

    bool TestSwizzleUnswizzle(const ScratchImage& image)
    {
        uint8_t digest[16];
        HRESULT hr = MD5Checksum(image, digest, 1);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating original digest (%08X)\n", static_cast<unsigned int>(hr));
            return false;
        }
        else
        {
            ScratchImage swizzle;
            hr = StandardSwizzle(*image.GetImage(0, 0, 0), true, swizzle);
            if (FAILED(hr))
            {
                printe("ERROR: Failed testing swizzle (%08X)\n", static_cast<unsigned int>(hr));
                return false;
            }
            else
            {
                uint8_t sdigest[16];
                hr = MD5Checksum(swizzle, sdigest, 1);
                if (FAILED(hr))
                {
                    printe("ERROR: Failed creating swizzle digest (%08X)\n", static_cast<unsigned int>(hr));
                    return false;
                }
                else if (memcmp(digest, sdigest, 16) == 0)
                {
                    printe("ERROR: Swizzle operation failed (%08X)\n", static_cast<unsigned int>(hr));
                    return false;
                }
                else
                {
                    ScratchImage unswizzle;
                    hr = StandardSwizzle(*swizzle.GetImage(0, 0, 0), false, unswizzle);
                    if (FAILED(hr))
                    {
                        printe("ERROR: Failed testing unswizzle (%08X)\n", static_cast<unsigned int>(hr));
                        return false;
                    }
                    else
                    {
                        uint8_t usdigest[16];
                        hr = MD5Checksum(unswizzle, usdigest, 1);
                        if (FAILED(hr))
                        {
                            printe("ERROR: Failed creating unswizzle digest (%08X)\n", static_cast<unsigned int>(hr));
                            return false;
                        }
                        else if (memcmp(sdigest, usdigest, 16) == 0)
                        {
                            printe("ERROR: Deswizzle operation failed (%08X)\n", static_cast<unsigned int>(hr));
                            return false;
                        }
                        else if (memcmp(digest, usdigest, 16) != 0)
                        {
                            printe("ERROR: Swizzle->Deswizzle operation failed (%08X)\n", static_cast<unsigned int>(hr));
                            return false;
                        }
                    }
                }
            }

            return true;
        }
    }
}

//-------------------------------------------------------------------------------------
// StandardSwizzle
bool TEXTest::Test20()
{
    bool success = true;

    // 8bpp
    {
        ScratchImage test;
        HRESULT hr = FillTexture<DXGI_FORMAT_R8_UNORM,uint8_t>(32, 32, 1, test);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating 2D 8bp test texture (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else if (!TestSwizzleUnswizzle(test))
        {
            success = false;
        }
    }

    // 16bpp
    // TODO -

    // 32bpp
    // TODO -

    // 64bpp
    // TODO -

    // 128bpp
    // TODO -

    return success;
}
