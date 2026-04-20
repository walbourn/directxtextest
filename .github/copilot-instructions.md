# GitHub Copilot Instructions for DirectXTex Test Suite

These instructions define how GitHub Copilot should assist with the DirectXTex test suite located in this repository ([Test Suite](https://github.com/walbourn/directxtextest)). The goal is to ensure consistent, high-quality test code generation aligned with the existing conventions. They supplement the [parent project instructions](https://github.com/microsoft/DirectXTex/blob/main/.github/copilot-instructions.md) with test-specific conventions.

## Context

- **Project**: Test Suite for DirectXTex Texture Processing Library
- **Repository**: <https://github.com/walbourn/directxtextest>
- **Language**: C++17
- **Test Framework**: Custom lightweight framework (no third-party dependency)
- **Build System**: CMake / CTest (built as subdirectory of the main DirectXTex repo)
- **Documentation**: https://github.com/walbourn/directxtextest/wiki
- **Parent Project**: <https://github.com/microsoft/DirectXTex>

> This test suite **cannot** be built standalone. The `CMakeLists.txt` is designed to be included by the main DirectXTex project's CMake build. Building top-level will produce a fatal error.

## File Structure

```txt
Tests/
  inc/              # Shared test headers (directxtest.h, getname.h, d3dx12.h)
  media/            # BVT test media files (minimal set for build verification)
  dds/              # DDS format I/O tests
  hdr/              # HDR format I/O tests
  tga/              # TGA format I/O tests
  wic/              # WIC format I/O tests
  exr/              # OpenEXR format tests (requires ENABLE_OPENEXR_SUPPORT)
  jpg/              # JPEG format tests (requires ENABLE_LIBJPEG_SUPPORT)
  png/              # PNG format tests (requires ENABLE_LIBPNG_SUPPORT)
  ppm/              # PPM format tests
  tex/              # Core texture processing tests (convert, flip, misc, normals, etc.)
  filter/           # Filtering, resize, and mipmap generation tests
  compress/         # BC1-BC7 block compression tests
  d3d11/            # Direct3D 11 interop tests (requires BUILD_DX11)
  d3d12/            # Direct3D 12 interop tests (requires BUILD_DX12)
  d3d9loadtest/     # D3D9 DDS texture loader tests (requires BUILD_DX11)
  d3d11loadtest/    # D3D11 DDS texture loader tests (requires BUILD_DX11)
  d3d12loadtest/    # D3D12 DDS texture loader tests (requires BUILD_DX12)
  headertest/       # Compilation-only test validating public headers
  standalonetest/   # Tests for standalone DDSTextureLoader/ScreenGrab/WICTextureLoader
  fuzzloaders/      # Fuzz testing harness for image loaders
  bcvalidator/      # BC format validation tool
  platformtest/     # Game-style platform test harness (GDK/UWP/Win32)
  xbox/             # Xbox extension tests (requires BUILD_XBOX_EXTS_*)
  toolscripts/      # Scripts for testing CLI tools (texconv, texassemble, texdiag)
  codecov/          # Code coverage helper
```

## Test Framework Conventions

### Standard Runner Pattern (most `xt*` suites)

Most core format and library test suites (`xtdds`, `xthdr`, `xttga`, `xtwic`, `xttex`, `xtfilter`, `xtcompress`, `xtppm`) follow this pattern:

```cpp
// directxtest.cpp - Test runner
typedef bool (*TestFN)();

struct TestInfo
{
    const char *name;
    TestFN func;
};

TestInfo g_Tests[] =
{
    { "DescriptiveName", Test01 },
    { "AnotherTest", Test02 },
    // ...
};

bool RunTests()
{
    size_t nPass = 0;
    size_t nFail = 0;

    for (size_t i = 0; i < std::size(g_Tests); ++i)
    {
        print("%s: ", g_Tests[i].name);

        if (g_Tests[i].func())
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

    print("Ran %zu tests, %zu pass, %zu fail\n", nPass + nFail, nPass, nFail);
    return (nFail == 0);
}

int __cdecl wmain()
{
    // Banner
    print("*** " _DIRECTX_TEST_NAME_ " test\n");
    print("*** Library Version %03d\n", DIRECTX_TEX_VERSION);

    if (!XMVerifyCPUSupport())
    {
        printe("ERROR: XMVerifyCPUSupport fails\n");
        return -1;
    }

#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // CoInitializeEx if WIC is used
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return -1;

    if (!RunTests())
        return -1;

    SetWICFactory(nullptr); // Cleanup if WIC was used
    return 0;
}
```

### Notable Exceptions to the Standard Pattern

- **`headertest`**: Compilation-only test; just includes headers and returns 0
- **`standalonetest`**: Calls separate `test9()`, `test11()`, `test12()` functions
- **`fuzzloaders`**: Command-line fuzzing harness; not a pass/fail test suite
- **`bcvalidator`**: Validation tool, not a standard test runner
- **`d3d9/11/12loadtest`**: Use game-style `wWinMain` entry points and `-ctest` flag
- **`platformtest`**: Full game-loop test harness with `DeviceResources`

### SEH Exception Handling

Suites testing computationally sensitive code (e.g., `tex/`, `filter/`, `compress/`) wrap test calls in SEH:

```cpp
__try
{
    pass = g_Tests[i].func();
}
__except (DescribeException(GetExceptionInformation()))
{
    pass = false;
}
```

On MinGW, tests run without SEH (`#ifdef __MINGW32__` guard).

### Floating-Point Exception Validation

Some suites (`tex/`, `filter/`) enable floating-point exceptions for validation:

```cpp
unsigned int fpcw = 0;
_controlfp_s(&fpcw, _MCW_EM, _MCW_EM);
_clearfp();
_controlfp_s(nullptr, 0, _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_INVALID);
```

## Test Function Conventions

### Naming

- Standard tests: `Test01()`, `Test02()`, ... `TestNN()`
- Variant tests: `Test05B()`, `Test05C()`, `Test05D()`
- Namespaced tests (e.g., `tex/`): `TEXTest::Test01()`, `TEXTest::Test05B()`

### Return Value

All test functions return `bool` — `true` for pass, `false` for fail. Do not use assertion macros or throw exceptions.

### Error Reporting

- Use `print()` for informational output
- Use `printe()` for error messages
- Use helper macros from `directxtest.h`: `printmeta()`, `printmetachk()`, `printdigest()`, `printdds()`, `printxmv()`

### Standard Test Function Body Pattern

```cpp
bool Test01()
{
    bool success = true;
    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < std::size(g_TestMedia); ++index)
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

        // Load/process/validate ...
        TexMetadata metadata;
        HRESULT hr = GetMetadataFromDDSFile(szPath, DDS_FLAGS_NONE, metadata);
        if (FAILED(hr))
        {
            success = false;
            printe("Failed loading (%08X):\n%ls\n", static_cast<unsigned int>(hr), szPath);
        }
        else
        {
            // Validate metadata, MD5, etc.
            ++npass;
        }
        ++ncount;
    }

    print("%zu images tested, %zu images passed ", ncount, npass);
    return success;
}
```

## Test Data Conventions

### TestMedia Struct Pattern

```cpp
struct TestMedia
{
    DWORD options;
    TexMetadata metadata;
    const wchar_t *fname;
    uint8_t md5[16];
};

const TestMedia g_TestMedia[] =
{
    // { options, { width, height, depth, arraySize, mipLevels, miscFlags, miscFlags2, format, dimension }, path, md5 }
    { FLAGS_NONE, { 256, 256, 1, 1, 9, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_DIMENSION_TEXTURE2D },
      MEDIA_PATH L"reftexture.dds",
      { 0x9d,0xf2,0x92,0xf3,0x45,0x5f,0x4e,0xb4,0xa3,0x01,0xe5,0x47,0x43,0x4f,0x40,0xc5 } },
#ifndef BUILD_BVT_ONLY
    // Extended test cases ...
#endif
};
```

### Media Path Handling

- **Desktop**: Paths use `MEDIA_PATH` macro which expands to `%DIRECTXTEX_MEDIA_PATH%\\`
- **UWP/App**: Uses `Assets\\`
- **Xbox**: Uses `Assets\\` with `S:\\dxtmp` or `T:\\` temp paths
- Always expand via `ExpandEnvironmentStringsW()` before use
- `TEMP_PATH` macro for temporary output files

### Validation Methods

- **MD5 checksums**: Validate deterministic image output (pixel data)
- **Metadata comparison**: Verify width, height, depth, arraySize, mipLevels, format, dimension
- **HRESULT checks**: Verify success/failure of API calls
- **Dimension/pitch checks**: Validate ComputePitch, ComputeScanlines results

## Build Verification Tests (BVT)

### Purpose

BVT mode provides fast CI validation using minimal media bundled in `Tests/media/`.

### Configuration

- CMake option: `BUILD_BVT=ON`
- Compile define: `BUILD_BVT_ONLY`
- Disables `BUILD_DX11` and `BUILD_DX12` in BVT mode
- Uses `Tests/media/` instead of external media path

### Usage in Test Code

Guard extended test cases (large arrays, exhaustive format coverage) with:

```cpp
#ifndef BUILD_BVT_ONLY
    // Extended test data requiring full DIRECTXTEX_MEDIA_PATH
    { FLAGS_NONE, { 512, 256, 1, 1, 10, 0, 0, DXGI_FORMAT_R10G10B10A2_UNORM, ... },
      MEDIA_PATH L"earth_A2B10G10R10.dds", {...} },
#endif
```

BVT test cases should use only files present in `Tests/media/`.

## Build Gates

Tests are conditionally compiled based on CMake options:

| CMake Option | Tests Affected |
|---|---|
| `BUILD_DX11` | d3d9loadtest, d3d11loadtest, xtd3d11, xtcompress, bcvalidator |
| `BUILD_DX12` | d3d12loadtest, xtd3d12 |
| `ENABLE_OPENEXR_SUPPORT` | xtexr |
| `ENABLE_LIBJPEG_SUPPORT` | xtjpg |
| `ENABLE_LIBPNG_SUPPORT` | xtpng |
| `BUILD_XBOX_EXTS_XBOXONE` / `BUILD_XBOX_EXTS_SCARLETT` | xtxbox |
| `BUILD_BVT` | All tests (reduced scope) |

## Naming Conventions

| Element | Convention | Example |
|---|---|---|
| Test functions | `TestNN()` or `Namespace::TestNN()` | `Test01()`, `TEXTest::Test05B()` |
| Test executables | `xt{topic}` for library tests | `xtdds`, `xthdr`, `xttex`, `xtfilter` |
| Test executables | `{api}loadtest` for loader tests | `d3d11loadtest`, `d3d12loadtest` |
| CTest names | Short descriptive | `"dds"`, `"hdr"`, `"tex"`, `"filter"` |
| CTest labels | `Category;Subcategory` | `"ImageFormats;DDS"`, `"Library"`, `"Loaders;DDS;DX11"`, `"BC"` |
| Media structs | `g_TestMedia`, `g_SaveMedia` | Arrays of `TestMedia` |
| Test options flags | `FLAGS_*` | `FLAGS_NONE`, `FLAGS_BGR`, `FLAGS_YUV` |

## Patterns to Follow

- Use `bool` return values for pass/fail, not assertions or exceptions
- Use `printe()` for error messages with context (filename, HRESULT, expected vs actual)
- Use MD5 checksums to validate deterministic image output
- Use `ExpandEnvironmentStringsW()` for all media file path resolution
- Guard extended test data with `#ifndef BUILD_BVT_ONLY`
- Use SEH `__try/__except` wrapping in suites that test computation-heavy paths
- Call `XMVerifyCPUSupport()` in `wmain()` before running tests
- Call `CoInitializeEx()` in suites that use WIC functions
- Call `SetWICFactory(nullptr)` for cleanup before exit in WIC-using suites
- Enable CRT memory leak detection: `_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)`
- Check `FAILED(hr)` and print the HRESULT as hex: `static_cast<unsigned int>(hr)`
- Link against `bcrypt.lib` (all test executables use this for MD5)

## Patterns to Avoid

- Do not use external test frameworks (Google Test, Catch2, doctest, etc.)
- Do not use assertion macros (`ASSERT_EQ`, `EXPECT_TRUE`, etc.)
- Do not introduce exceptions in test functions
- Do not assume media files exist without environment variable path expansion
- Do not add tests that require network access
- Do not hardcode absolute paths to media files
- Do not add `_WIN32_WINNT` defines in source; these come from CMake
- Do not build the test suite as a top-level CMake project

## Building and Running Tests

```bash
# Configure with BVT mode (no external media needed)
cmake --preset=x64-Debug -DBUILD_BVT=ON

# Configure with full test suite (requires DIRECTXTEX_MEDIA_PATH)
cmake --preset=x64-Debug

# Build
cmake --build out/build/x64-Debug

# Run all tests
ctest --test-dir out/build/x64-Debug

# Run specific test by label
ctest --test-dir out/build/x64-Debug -L "ImageFormats"
ctest --test-dir out/build/x64-Debug -L "BC"

# Run specific test by name
ctest --test-dir out/build/x64-Debug -R "dds"
```

## Code Review Instructions

- Verify MD5 checksums match expected output for any new or modified test data
- Check that `#ifndef BUILD_BVT_ONLY` guards are applied to new extended test cases
- Verify CTest labels match the appropriate category (`ImageFormats`, `Library`, `Loaders`, `BC`, `Xbox`)
- Check CTest timeouts are reasonable for the test scope
- Ensure new test executables are added to the `TEST_EXES` list and linked against `DirectXTex` and `bcrypt.lib`
- Verify that any new test requiring optional features is gated behind the correct CMake option
- Confirm test functions follow the `bool TestNN()` naming pattern
- Check that error messages include enough context for debugging (file path, HRESULT, expected vs actual values)
