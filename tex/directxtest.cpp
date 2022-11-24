//-------------------------------------------------------------------------------------
// DirectXTest.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "tex.h"

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

TestInfo g_Tests[] =
{
    { "Is*/BPP/BPC", TEXTest::Test01 },
    { "ComputePitch/Scanlines", TEXTest::Test02 },
    { "Make*", TEXTest::Test12 },
    { "ComputeMSE", TEXTest::Test10 },
    { "EvaluateImage", TEXTest::Test16 },
    { "TransformImage", TEXTest::Test17 },
    { "IsAlphaAllOpaque", TEXTest::Test14 },
    { "FlipRotate", TEXTest::Test04 },
    { "Convert (internalA)", TEXTest::Test05 },
    { "Convert (internalB)", TEXTest::Test05B },
    { "Convert (internalC)", TEXTest::Test05C },
    { "Convert (internalD)", TEXTest::Test05D },
    { "Convert", TEXTest::Test06 },
    { "ConvertToSinglePlane", TEXTest::Test15 },
    { "CopyRectangle", TEXTest::Test09 },
    { "ComputeNormalMap", TEXTest::Test11 },
    { "PremultiplyAlpha", TEXTest::Test13 },
};

extern int __cdecl DescribeException(PEXCEPTION_POINTERS pData);

//-------------------------------------------------------------------------------------
bool RunTests()
{
    UINT nPass = 0;
    UINT nFail = 0;

    // Enable floating-point exceptions for validation purposes
    // NOTE: This should only be done in test code, NOT production code!
    unsigned int fpcw = 0;
    _controlfp_s(&fpcw, _MCW_EM, _MCW_EM);

    _clearfp();

    _controlfp_s(nullptr, 0, _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_INVALID);

    for( UINT i=0; i < ( sizeof(g_Tests) / sizeof(TestInfo) ); ++i)
    {
        print("%s: ", g_Tests[i].name );

        bool pass = false;

    #ifdef __MINGW32__
        pass = g_Tests[i].func();
    #else
        __try
        {
            pass = g_Tests[i].func();
        }
        __except (DescribeException(GetExceptionInformation()))
        {
            pass = false;
        }
    #endif

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

#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

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
