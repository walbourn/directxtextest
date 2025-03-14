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
extern HRESULT SaveScratchImage( _In_z_ const wchar_t* szFile, _In_ DirectX::DDS_FLAGS flags, _In_ const ScratchImage& image );

//-------------------------------------------------------------------------------------
// StandardSwizzle
bool TEXTest::Test20()
{
    // TODO -
    return false;
}
