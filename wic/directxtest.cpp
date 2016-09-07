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

extern bool Test00();
extern bool Test01();
extern bool Test02();
extern bool Test03();
extern bool Test04();
extern bool Test05();
extern bool Test06();

TestInfo g_Tests[] =
{
    { "Get/SetWICFactory", Test00 },
    { "GetMetadataFromWICMemory", Test01 },
    { "GetMetadataFromWICFile", Test02 },
    { "LoadFromWICMemory", Test03 },
    { "LoadFromWICFile", Test04 },
    { "SaveWICToMemory", Test05 },
    { "SaveWICToFile", Test06 },
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

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    HRESULT hr = CoInitializeEx( NULL,  COINIT_MULTITHREADED );
    if (FAILED(hr))
    {
        printe("ERROR: CoInitializeEx fails (%08X)\n", hr);
        return -1;
    }

    if ( !RunTests() )
        return -1;

    SetWICFactory(nullptr);
       
    return 0;
}