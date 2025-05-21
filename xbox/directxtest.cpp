//-------------------------------------------------------------------------------------
// DirectXTest.cpp
//  
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "DirectXTex.h"
#include "DirectXTexXbox.h"

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
extern bool Test04();
extern bool Test05();
extern bool Test06();
extern bool Test07();

TestInfo g_Tests[] =
{
    { "GetMetadataFromDDSMemory", Test01 },
    { "GetMetadataFromDDSFile", Test02 },
    { "LoadFromDDSMemory", Test03 },
    { "LoadFromDDSFile", Test04 },
    { "SaveDDSToMemory", Test05 },
    { "SaveDDSToFile", Test06 },
    { "Tile/Detile", Test07 },
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
    print("*** " _DIRECTX_TEST_NAME_ " test\n");
    print("*** Library Version %03d (DirectXTex %03d)\n", DIRECTX_TEX_XBOX_VERSION, DIRECTX_TEX_VERSION);
    print("**************************************************************\n");

    if (!XMVerifyCPUSupport())
    {
        printe("ERROR: XMVerifyCPUSupport fails on this system, not a supported platform\n");
        return -1;
    }

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printe("ERROR: CoInitializeEx failed (%08X)\n", static_cast<unsigned int>(hr));
        return -1;
    }

    if (!RunTests())
        return -1;

    return 0;
}
