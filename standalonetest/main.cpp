// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This test ensures that all public headers fully include all their dependancies, as well compile cleanly at maximum warning level

#ifdef _WIN32
#include <winapifamily.h>
#endif

#if defined(_WIN32) && (!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
extern void test9();
#endif

#ifdef _WIN32
extern void test11();
#endif

extern void test12();

int main()
{
#if defined(_WIN32) && (!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
    test9();
#endif

#ifdef _WIN32
    test11();
#endif

    test12();

    return 0;
}
