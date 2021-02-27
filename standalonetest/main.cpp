// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This test ensures that all public headers fully include all their dependancies, as well compile cleanly at maximum warning level

#ifdef WIN32
extern void test9();
extern void test11();
#endif

extern void test12();

int main()
{
#ifdef WIN32
    test9();
    test11();
#endif
    test12();

    return 0;
}
