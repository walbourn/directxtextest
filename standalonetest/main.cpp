// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// This test ensures that all public headers fully include all their dependancies, as well compile cleanly at maximum warning level

#include "DDSTextureLoader9.h"
#include "ScreenGrab9.h"
#include "WICTextureLoader9.h"

extern void test11();
extern void test12();

int main()
{
    test11();
    test12();

    return 0;
}
