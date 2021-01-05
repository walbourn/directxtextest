# DirectXTex Standalone Module Test
#
# Copyright (c) Microsoft Corporation. All rights reserved.
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

    set(TEST_SOURCES ${TEST_SOURCES}
        test9.cpp
        ../../DDSTextureLoader/DDSTextureLoader9.h
        ../../DDSTextureLoader/DDSTextureLoader9.cpp
        ../../ScreenGrab/ScreenGrab9.h
        ../../ScreenGrab/ScreenGrab9.cpp
        ../../WICTextureLoader/WICTextureLoader9.h
        ../../WICTextureLoader/WICTextureLoader9.cpp)

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

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dxguid.lib)
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

    find_package(directx-headers CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids)
endif()

if(MSVC)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:SSE2)
    endif()
endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    set(WarningsEXE "-Wpedantic" "-Wextra" "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-float-equal" "-Wno-global-constructors" "-Wno-language-extension-token" "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro")
    target_compile_options(${PROJECT_NAME} PRIVATE ${WarningsEXE})
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive- /Zc:__cplusplus)

    set(WarningsEXE "/wd4061" "/wd4365" "/wd4668" "/wd4820" "/wd5031" "/wd5032" "/wd5039" "/wd5045" )
    target_compile_options(${PROJECT_NAME} PRIVATE ${WarningsEXE})
endif()

if(WIN32)
    # Windows 10 is used here to build the DirectX 12 code paths as well
    target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE _WIN32_WINNT=0x0A00)
endif()
