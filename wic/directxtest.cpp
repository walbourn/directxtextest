//-------------------------------------------------------------------------------------
// DirectXTest.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "wic.h"

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
    { "Get/SetWICFactory", WICTest::Test00 },
    { "GetMetadataFromWICMemory", WICTest::Test01 },
    { "GetMetadataFromWICFile", WICTest::Test02 },
    { "LoadFromWICMemory", WICTest::Test03 },
    { "LoadFromWICFile", WICTest::Test04 },
    { "SaveWICToMemory", WICTest::Test05 },
    { "SaveWICToFile", WICTest::Test06 },
    { "*Transcode*", WICTest::Test07 },
    { "Fuzz", WICTest::Test08 },
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
