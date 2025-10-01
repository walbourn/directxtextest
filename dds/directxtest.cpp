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
extern bool Test04();
extern bool Test05();
extern bool Test06();
extern bool Test07();
extern bool Test08();
extern bool Test09();
extern bool Test10();

TestInfo g_Tests[] =
{
    { "GetMetadataFromDDSMemory/File", Test01 },
    { "LoadFromDDSMemory", Test02 },
    { "LoadFromDDSFile", Test03 },
    { "GetMetadataFromDDSMemory/FileEx", Test07 },
    { "LoadFromDDSMemoryEx", Test08 },
    { "LoadFromDDSFileEx", Test09 },
    { "EncodeDDSHeader", Test10 },
    { "SaveDDSToMemory", Test04 },
    { "SaveDDSToFile", Test05 },
    { "Fuzz", Test06 },
};


//-------------------------------------------------------------------------------------
bool RunTests()
{
    size_t nPass = 0;
    size_t nFail = 0;

    for(size_t i=0; i < std::size(g_Tests); ++i)
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

    print("Ran %zu tests, %zu pass, %zu fail\n", nPass+nFail, nPass, nFail);

    return (nFail == 0);
}


//-------------------------------------------------------------------------------------
int __cdecl wmain()
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

    if ( !RunTests() )
        return -1;

    return 0;
}
