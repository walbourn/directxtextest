// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This test ensures that all public headers fully include all their dependancies, as well compile cleanly at maximum warning level

#include "test.h"

#include "DDSTextureLoader12.h"
#include "ScreenGrab12.h"

#ifdef _WIN32
#include "WICTextureLoader12.h"
#endif

void test12()
{
    return;
}
