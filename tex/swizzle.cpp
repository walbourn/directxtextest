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
            return E_NOTIMPL; // TODO: Need a FillTexture for 3D textures

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

    template <DXGI_FORMAT F> HRESULT FillTexture128(size_t w, size_t h, size_t d, ScratchImage& result)
    {
        if (d != 1)
            return E_NOTIMPL; // TODO: Need a FillTexture for 3D textures

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
            uint32_t* p = reinterpret_cast<uint32_t*>(ptr);
            for (size_t x = 0; x < w; ++x)
            {
                *(p++) = val++;
                *(p++) = val++;
            }
            ptr += img->rowPitch;
        }

        return S_OK;
    }

    // TODO: Need a FillBCTexture for compressed formats

    bool TestSwizzleUnswizzle(const ScratchImage& image, int instance)
    {
        uint8_t digest[16];
        HRESULT hr = MD5Checksum(image, digest, 1);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating original digest %d (%08X)\n", instance, static_cast<unsigned int>(hr));
            return false;
        }
        else
        {
            ScratchImage swizzle;
            hr = StandardSwizzle(*image.GetImage(0, 0, 0), true, swizzle);
            if (FAILED(hr))
            {
                printe("ERROR: Failed testing swizzle %d (%08X)\n", instance, static_cast<unsigned int>(hr));
                return false;
            }
            else
            {
                uint8_t sdigest[16];
                hr = MD5Checksum(swizzle, sdigest, 1);
                if (FAILED(hr))
                {
                    printe("ERROR: Failed creating swizzle digest %d (%08X)\n", instance, static_cast<unsigned int>(hr));
                    return false;
                }
                else if (memcmp(digest, sdigest, 16) == 0)
                {
                    printe("ERROR: Swizzle operation failed %d (%08X)\n", instance, static_cast<unsigned int>(hr));
                    return false;
                }
                else
                {
                    ScratchImage unswizzle;
                    hr = StandardSwizzle(*swizzle.GetImage(0, 0, 0), false, unswizzle);
                    if (FAILED(hr))
                    {
                        printe("ERROR: Failed testing unswizzle %d (%08X)\n", instance, static_cast<unsigned int>(hr));
                        return false;
                    }
                    else
                    {
                        uint8_t usdigest[16];
                        hr = MD5Checksum(unswizzle, usdigest, 1);
                        if (FAILED(hr))
                        {
                            printe("ERROR: Failed creating unswizzle digest %d (%08X)\n", instance, static_cast<unsigned int>(hr));
                            return false;
                        }
                        else if (memcmp(sdigest, usdigest, 16) == 0)
                        {
                            printe("ERROR: Deswizzle operation failed %d (%08X)\n", instance, static_cast<unsigned int>(hr));
                            return false;
                        }
                        else if (memcmp(digest, usdigest, 16) != 0)
                        {
                            printe("ERROR: Swizzle->Deswizzle operation failed %d (%08X)\n", instance, static_cast<unsigned int>(hr));
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
bool TEXTest::Test23()
{
    bool success = true;

    // 8bpp
    {
        ScratchImage test;
        HRESULT hr = FillTexture<DXGI_FORMAT_R8_UNORM,uint8_t>(256, 256, 1, test);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating 2D 8bpp test texture (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else if (!TestSwizzleUnswizzle(test, 1))
        {
            success = false;
        }

        // TODO: Mips/other formats, 3D, Cube
    }

    // 16bpp
    {
        ScratchImage test;
        HRESULT hr = FillTexture<DXGI_FORMAT_R16_UNORM, uint16_t>(256, 128, 1, test);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating 2D 16bpp test texture (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else if (!TestSwizzleUnswizzle(test, 2))
        {
            success = false;
        }

        // TODO: Mips/other formats, 3D, Cube
    }

    // 32bpp
    {
        ScratchImage test;
        HRESULT hr = FillTexture<DXGI_FORMAT_R32_UINT, uint32_t>(128, 128, 1, test);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating 2D 32bpp test texture (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else if (!TestSwizzleUnswizzle(test, 3))
        {
            success = false;
        }

        // TODO: Mips/other formats, 3D, Cube
    }

    // 64bpp
    {
        ScratchImage test;
        HRESULT hr = FillTexture<DXGI_FORMAT_R32G32_UINT, uint64_t>(128, 64, 1, test);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating 2D 64bpp test texture (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else if (!TestSwizzleUnswizzle(test, 4))
        {
            success = false;
        }

        // TODO: Mips/other formats, 3D, Cube
    }

    // 128bpp
    {
        ScratchImage test;
        HRESULT hr = FillTexture128<DXGI_FORMAT_R32G32_UINT>(64, 64, 1, test);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating 2D 128bpp test texture (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else if (!TestSwizzleUnswizzle(test, 5))
        {
            success = false;
        }

        // TODO: Mips/other formats, 3D, Cube
    }

    // TODO: Non-tile sized images

    // invalid args
    {
        ScratchImage test;
        HRESULT hr = FillTexture<DXGI_FORMAT_R8_UNORM, uint8_t>(256, 256, 1, test);
        if (FAILED(hr))
        {
            printe("ERROR: Failed creating 2D 8bpp test texture (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else
        {
            ScratchImage swizzle;
            Image img = *test.GetImage(0, 0, 0);
            img.format = DXGI_FORMAT_R32G32B32_FLOAT;
            hr = StandardSwizzle(img, true, swizzle);
            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            {
                success = false;
                printe("Failed invalid arg test for unsupported format\n");
            }

            TexMetadata mdata = test.GetMetadata();
            mdata.format = DXGI_FORMAT_R32G32B32_FLOAT;
            hr = StandardSwizzle(test.GetImages(), test.GetImageCount(), mdata, true, swizzle);
            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            {
                success = false;
                printe("Failed invalid arg test for unsupported format [complex]\n");
            }

            img = *test.GetImage(0, 0, 0);
            img.width = UINT32_MAX;
            hr = StandardSwizzle(img, true, swizzle);
            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            {
                success = false;
                printe("Failed invalid arg test for image too large\n");
            }

            mdata = test.GetMetadata();
            mdata.width = UINT32_MAX;
            hr = StandardSwizzle(test.GetImages(), test.GetImageCount(), mdata, true, swizzle);
            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            {
                success = false;
                printe("Failed invalid arg test for image too large [complex]\n");
            }

            img = *test.GetImage(0, 0, 0);
            img.height = 1;
            hr = StandardSwizzle(img, true, swizzle);
            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            {
                success = false;
                printe("Failed invalid arg test for 1D image\n");
            }

            mdata = test.GetMetadata();
            mdata.dimension = TEX_DIMENSION_TEXTURE1D;
            hr = StandardSwizzle(test.GetImages(), test.GetImageCount(), mdata, true, swizzle);
            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            {
                success = false;
                printe("Failed invalid arg test for 1D image [complex]\n");
            }

            img = *test.GetImage(0, 0, 0);
            img.format = DXGI_FORMAT_UNKNOWN;
            hr = StandardSwizzle(img, true, swizzle);
            if (hr != E_INVALIDARG)
            {
                success = false;
                printe("Failed invalid arg test for invalid format\n");
            }

            mdata = test.GetMetadata();
            mdata.format = DXGI_FORMAT_UNKNOWN;
            hr = StandardSwizzle(test.GetImages(), test.GetImageCount(), mdata, true, swizzle);
            if (hr != E_INVALIDARG)
            {
                success = false;
                printe("Failed invalid arg test for invalid format [complex]\n");
            }
        }
    }

    {
    #pragma warning(push)
    #pragma warning(disable:6385 6387)
        ScratchImage image;
        Image nullin = {};
        nullin.width = nullin.height = 256;
        nullin.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        HRESULT hr = StandardSwizzle(nullin, true, image);
        if (hr != E_INVALIDARG && hr != E_POINTER)
        {
            success = false;
            printe("Failed invalid arg test\n");
        }

        TexMetadata metadata = {};
        metadata.width = metadata.height = 256;
        metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        metadata.depth = metadata.arraySize = metadata.mipLevels = 1;
        metadata.dimension = TEX_DIMENSION_TEXTURE2D;
        hr = StandardSwizzle(nullptr, 0, metadata, true, image);
        if (hr != E_INVALIDARG)
        {
            success = false;
            printe("Failed invalid arg complex test\n");
        }
    #pragma warning(pop)
    }

    return success;
}
