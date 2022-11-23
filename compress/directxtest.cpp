//-------------------------------------------------------------------------------------
// DirectXTest.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------
// Types and globals

typedef bool (*TestFN)();

struct TestInfo
{
    const char *name;
    TestFN func;
};

extern bool Test01();
extern bool Test02();
extern bool Test03();

TestInfo g_Tests[] =
{
    { "Decompress", Test01 },
#ifndef _M_ARM64
    { "Compress (CPU)", Test02 },
    { "Compress (GPU)", Test03 },
#endif
};

extern int __cdecl DescribeException(PEXCEPTION_POINTERS pData);

//-------------------------------------------------------------------------------------
bool RunTests()
{
    // Enable floating-point exceptions for validation purposes
    // NOTE: This should only be done in test code, NOT production code!
    unsigned int fpcw = 0;
    _controlfp_s(&fpcw, _MCW_EM, _MCW_EM);

    _clearfp();

    _controlfp_s(nullptr, 0, _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_INVALID);

    UINT nPass = 0;
    UINT nFail = 0;

    for( UINT i=0; i < ( sizeof(g_Tests) / sizeof(TestInfo) ); ++i)
    {
        print("%s: ", g_Tests[i].name );

        bool pass = false;

        __try
        {
            pass = g_Tests[i].func();
        }
        __except (DescribeException(GetExceptionInformation()))
        {
            pass = false;
        }

        if (pass)
        {
            ++nPass;
            print("PASS\n");
        }
        else
        {
            ++nFail;
            print("FAIL\n");
        }
    }

    _clearfp();
    _controlfp_s(nullptr, fpcw, _MCW_EM);

    print("Ran %d tests, %d pass, %d fail\n", nPass+nFail, nPass, nFail);

    return (nFail == 0);
}


//-------------------------------------------------------------------------------------
int __cdecl main()
{
    print("**************************************************************\n");
    print("*** " _DIRECTX_TEST_NAME_ " test\n" );
    print("*** Library Version %03d\n", DIRECTX_TEX_VERSION );
    print("**************************************************************\n");

    if ( !XMVerifyCPUSupport() )
    {
        printe("ERROR: XMVerifyCPUSupport fails on this system, not a supported platform\n");
        return -1;
    }

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    HRESULT hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
    if (FAILED(hr))
    {
        printe("ERROR: CoInitializeEx fails (%08X)\n", static_cast<unsigned int>(hr));
        return -1;
    }

    if ( !RunTests() )
        return -1;

    SetWICFactory(nullptr);

    return 0;
}
