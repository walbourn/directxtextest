//-------------------------------------------------------------------------------------
// DirectXTest.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "directxtex.h"

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
extern bool Test04();
extern bool Test05();
extern bool Test05B();
extern bool Test05C();
extern bool Test05D();
extern bool Test06();
extern bool Test09();
extern bool Test10();
extern bool Test11();
extern bool Test12();
extern bool Test13();
extern bool Test14();
extern bool Test15();

TestInfo g_Tests[] =
{
    { "Is*/BPP/BPC", Test01 },
    { "ComputePitch/Scanlines", Test02 },
    { "Make*", Test12 },
    { "ComputeMSE", Test10 },
    { "IsAlphaAllOpaque", Test14 },
    { "FlipRotate", Test04 },
    { "Convert (internalA)", Test05 },
    { "Convert (internalB)", Test05B },
    { "Convert (internalC)", Test05C },
    { "Convert (internalD)", Test05D },
    { "Convert", Test06 },
    { "ConvertToSinglePlane", Test15 },
    { "CopyRectangle", Test09 },
    { "ComputeNormalMap", Test11 },
    { "PremultiplyAlpha", Test13 },
};


//-------------------------------------------------------------------------------------
bool RunTests()
{
    UINT nPass = 0;
    UINT nFail = 0;

    for( UINT i=0; i < ( sizeof(g_Tests) / sizeof(TestInfo) ); ++i)
    {
        print("%s: ", g_Tests[i].name );

        if ( g_Tests[i].func() )
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

    print("Ran %d tests, %d pass, %d fail\n", nPass+nFail, nPass, nFail);

    return (nFail == 0);
}


//-------------------------------------------------------------------------------------
int __cdecl main()
{
    print("**************************************************************\n");
    print("*** " _DIRECTX_TEST_NAME_ " test built on " __DATE__ "\n" );
    print("*** Library Version %03d\n", DIRECTX_TEX_VERSION );
    print("**************************************************************\n");

    if ( !XMVerifyCPUSupport() )
    {
        printe("ERROR: XMVerifyCPUSupport fails on this system, not a supported platform\n");
        return -1;
    }

    HRESULT hr = CoInitializeEx( NULL,  COINIT_MULTITHREADED );
    if (FAILED(hr))
    {
        printe("ERROR: CoInitializeEx fails (%08X)\n", hr);
        return -1;
    }

    if ( !RunTests() )
        return -1;
       
    return 0;
}