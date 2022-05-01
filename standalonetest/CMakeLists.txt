# DirectXTex Standalone Module Test
#
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
#
# http://go.microsoft.com/fwlink/?LinkId=248926

cmake_minimum_required (VERSION 3.11)

project (standalonetest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/CMake")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/CMake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/CMake")

if (DEFINED VCPKG_TARGET_ARCHITECTURE)
    set(DIRECTX_ARCH ${VCPKG_TARGET_ARCHITECTURE})
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Ww][Ii][Nn]32$")
    set(DIRECTX_ARCH x86)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Xx]64$")
    set(DIRECTX_ARCH x64)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]$")
    set(DIRECTX_ARCH arm)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]64$")
    set(DIRECTX_ARCH arm64)
endif()

if(MSVC)
    # Use max Warning Level
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif()

set(TEST_SOURCES
    test12.cpp
    ../../DDSTextureLoader/DDSTextureLoader12.h
    ../../DDSTextureLoader/DDSTextureLoader12.cpp
    ../../ScreenGrab/ScreenGrab12.h
    ../../ScreenGrab/ScreenGrab12.cpp)

if(WIN32)
    set(TEST_SOURCES ${TEST_SOURCES}
        ../../WICTextureLoader/WICTextureLoader12.h
        ../../WICTextureLoader/WICTextureLoader12.cpp)

    if (NOT WINDOWS_STORE)
        set(TEST_SOURCES ${TEST_SOURCES}
            test9.cpp
            ../../DDSTextureLoader/DDSTextureLoader9.h
            ../../DDSTextureLoader/DDSTextureLoader9.cpp
            ../../ScreenGrab/ScreenGrab9.h
            ../../ScreenGrab/ScreenGrab9.cpp
            ../../WICTextureLoader/WICTextureLoader9.h
           ../../WICTextureLoader/WICTextureLoader9.cpp)
    endif()

    set(TEST_SOURCES ${TEST_SOURCES}
        test11.cpp
        ../../DDSTextureLoader/DDSTextureLoader11.h
        ../../DDSTextureLoader/DDSTextureLoader11.cpp
        ../../ScreenGrab/ScreenGrab11.h
        ../../ScreenGrab/ScreenGrab11.cpp
        ../../WICTextureLoader/WICTextureLoader11.h
        ../../WICTextureLoader/WICTextureLoader11.cpp)
endif()

add_executable(${PROJECT_NAME} main.cpp ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ../inc
    ../../DDSTextureLoader
    ../../ScreenGrab
    ../../WICTextureLoader)

if (MINGW OR (NOT WIN32) OR VCPKG_TOOLCHAIN)
    message("INFO: Using VCPKG for DirectX-Headers and DirectXMath.")
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxmath CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectXMath)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USING_DIRECTX_HEADERS)
endif()

if (WIN32 AND NOT MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE dxguid.lib)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Guids)
endif()

if(NOT ${DIRECTX_ARCH} MATCHES "^arm")
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        set(ARCH_SSE2 $<$<CXX_COMPILER_ID:MSVC>:/arch:SSE2> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    else()
        set(ARCH_SSE2 $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    endif()

    foreach(t IN LISTS TOOL_EXES ITEMS ${PROJECT_NAME})
      target_compile_options(${t} PRIVATE ${ARCH_SSE2})
    endforeach()
endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    set(WarningsEXE "-Wpedantic" "-Wextra" "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-float-equal" "-Wno-global-constructors" "-Wno-language-extension-token" "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro")
    target_compile_options(${PROJECT_NAME} PRIVATE ${WarningsEXE})
endif()
if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--allow-multiple-definition)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive- /Zc:__cplusplus)

    set(WarningsEXE "/wd4061" "/wd4365" "/wd4668" "/wd4820" "/wd5031" "/wd5032" "/wd5039" "/wd5045" )
    target_compile_options(${PROJECT_NAME} PRIVATE ${WarningsEXE})
endif()

if(WIN32)
    if(WINDOWS_STORE)
      target_compile_definitions(${PROJECT_NAME} PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_APP)
    endif()

    # Windows 10 is used here to build the DirectX 12 code paths as well
    target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE _WIN32_WINNT=0x0A00)
endif()
