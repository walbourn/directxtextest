# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.20)

project (DirectXTexTest
  DESCRIPTION "DirectXTex Test Suite"
  HOMEPAGE_URL "https://github.com/walbourn/directxtextest/wiki"
  LANGUAGES CXX)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  message(FATAL_ERROR "DirectXTex Test Suite should be built by the main CMakeLists")
endif()

if(WIN32 AND (NOT DEFINED ENV{DIRECTXTEX_MEDIA_PATH}))
  message(WARNING "Tests make use of DIRECTXTEX_MEDIA_PATH environment variable to find media")
endif()

set(TEST_EXES headertest standalonetest)
add_executable(headertest headertest/main.cpp)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/standalonetest)

# d3d9/11/12loadtest
if(BUILD_DX11)
  list(APPEND TEST_EXES d3d9loadtest)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/d3d9loadtest)
  add_test(NAME "d3d9loadtest" COMMAND d3d9loadtest -ctest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set_tests_properties(d3d9loadtest PROPERTIES LABELS "Loaders")
  set_tests_properties(d3d9loadtest PROPERTIES TIMEOUT 30)

  list(APPEND TEST_EXES d3d11loadtest)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/d3d11loadtest)
  add_test(NAME "d3d11loadtest" COMMAND d3d11loadtest -ctest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set_tests_properties(d3d11loadtest PROPERTIES LABELS "Loaders")
  set_tests_properties(d3d11loadtest PROPERTIES TIMEOUT 30)
endif()

if(BUILD_DX12)
  list(APPEND TEST_EXES d3d12loadtest)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/d3d12loadtest)
  add_test(NAME "d3d12loadtest" COMMAND d3d12loadtest -ctest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set_tests_properties(d3d12loadtest PROPERTIES LABELS "Loaders")
  set_tests_properties(d3d12loadtest PROPERTIES TIMEOUT 30)
endif()

#TODO: platformtest

# bcvalidator
if(BUILD_DX11)
    list(APPEND TEST_EXES bcvalidator)
    add_executable(bcvalidator bcvalidator/bcvalidator.cpp)
endif()

# fuzzloaders
list(APPEND TEST_EXES fuzzloaders)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fuzzloaders)

# DDS
list(APPEND TEST_EXES xtdds)
add_executable(xtdds dds/dds.cpp dds/directxtest.cpp dds/testutils.cpp)
target_include_directories(xtdds PRIVATE ./inc)
add_test(NAME "dds" COMMAND xtdds)
set_tests_properties(dds PROPERTIES LABELS "ImageFormats")
set_tests_properties(dds PROPERTIES TIMEOUT 60)

# HDR
list(APPEND TEST_EXES xthdr)
add_executable(xthdr hdr/hdr.cpp hdr/directxtest.cpp hdr/testutils.cpp)
target_include_directories(xthdr PRIVATE ./inc)
add_test(NAME "hdr" COMMAND xthdr)
set_tests_properties(hdr PROPERTIES LABELS "ImageFormats")
set_tests_properties(hdr PROPERTIES TIMEOUT 450)

# PPM
list(APPEND TEST_EXES xtppm)
add_executable(xtppm ppm/ppm.cpp ppm/directxtest.cpp ppm/testutils.cpp ../Texconv/PortablePixMap.cpp)
target_include_directories(xtppm PRIVATE ./inc)
add_test(NAME "ppm" COMMAND xtppm)
set_tests_properties(ppm PROPERTIES LABELS "ImageFormats")
set_tests_properties(ppm PROPERTIES TIMEOUT 120)

# TGA
list(APPEND TEST_EXES xttga)
add_executable(xttga tga/tga.cpp tga/directxtest.cpp tga/testutils.cpp)
target_include_directories(xttga PRIVATE ./inc)
add_test(NAME "tga" COMMAND xttga)
set_tests_properties(tga PROPERTIES LABELS "ImageFormats")
set_tests_properties(tga PROPERTIES TIMEOUT 60)

# WIC
list(APPEND TEST_EXES xtwic)
add_executable(xtwic wic/wic.cpp wic/directxtest.cpp wic/testutils.cpp)
target_include_directories(xtwic PRIVATE ./inc)
add_test(NAME "wic" COMMAND xtwic)
set_tests_properties(wic PROPERTIES LABELS "ImageFormats")
set_tests_properties(wic PROPERTIES TIMEOUT 600)

# tex
list(APPEND TEST_EXES xttex)
add_executable(xttex
    tex/convert.cpp
    tex/fliprotate.cpp
    tex/misc.cpp
    tex/normalmaps.cpp
    tex/planar.cpp
    tex/premulalpha.cpp
    tex/utils.cpp
    tex/tex.h tex/directxtest.cpp tex/testutils.cpp)
target_include_directories(xttex PRIVATE ./inc)
target_link_libraries(xttex PRIVATE d3d11.lib)
add_test(NAME "tex" COMMAND xttex)
set_tests_properties(tex PROPERTIES LABELS "Library")
set_tests_properties(tex PROPERTIES TIMEOUT 500)

# filter
list(APPEND TEST_EXES xtfilter)
add_executable(xtfilter
    filter/coverage.cpp
    filter/mipmaps.cpp
    filter/resize.cpp
    filter/filter.h filter/directxtest.cpp filter/testutils.cpp)
target_include_directories(xtfilter PRIVATE ./inc)
add_test(NAME "filter" COMMAND xtfilter)
set_tests_properties(filter PROPERTIES LABELS "Library")
set_tests_properties(filter PROPERTIES TIMEOUT 360)

# compress
if(BUILD_DX11)
  list(APPEND TEST_EXES xtcompress)
  add_executable(xtcompress compress/compress.cpp compress/directxtest.cpp compress/testutils.cpp)
  target_include_directories(xtcompress PRIVATE ./inc)
  target_link_libraries(xtcompress PRIVATE d3d11.lib)
  add_test(NAME "compress" COMMAND xtcompress)
  set_tests_properties(compress PROPERTIES LABELS "BC")
  set_tests_properties(compress PROPERTIES TIMEOUT $<IF:$<CONFIG:DEBUG>,10000,3000>)
endif()

# d3d11
if(BUILD_DX11)
  list(APPEND TEST_EXES xtd3d11)
  add_executable(xtd3d11 d3d11/d3d11.cpp d3d11/directxtest.cpp d3d11/testutils.cpp)
  target_include_directories(xtd3d11 PRIVATE ./inc ./d3d11/shaders)
  target_link_libraries(xtd3d11 PRIVATE d3d11.lib)
  add_test(NAME "d3d11" COMMAND xtd3d11)
  set_tests_properties(d3d11 PROPERTIES LABELS "Library")
  set_tests_properties(d3d11 PROPERTIES TIMEOUT 120)
endif()

# d3d12
if(BUILD_DX12)
  list(APPEND TEST_EXES xtd3d12)
  add_executable(xtd3d12 d3d12/d3d12.cpp d3d12/directxtest.cpp d3d12/testutils.cpp)
  target_include_directories(xtd3d12 PRIVATE ./inc ./d3d12/shaders)
  target_link_libraries(xtd3d12 PRIVATE d3d12.lib dxguid.lib dxgi.lib)
  add_test(NAME "d3d12" COMMAND xtd3d12)
  set_tests_properties(d3d12 PROPERTIES LABELS "Library")
  set_tests_properties(d3d12 PROPERTIES TIMEOUT 120)
endif()

message("INFO: Enabled tests: ${TEST_EXES}")
foreach(t IN LISTS TEST_EXES)
  target_link_libraries(${t} PRIVATE DirectXTex bcrypt.lib)
endforeach()

if(directxmath_FOUND)
    foreach(t IN LISTS TEST_EXES)
      target_link_libraries(${t} PRIVATE Microsoft::DirectXMath)
    endforeach()
endif()

if(directx-headers_FOUND)
    foreach(t IN LISTS TEST_EXES)
      target_link_libraries(${t} PRIVATE Microsoft::DirectX-Headers)
      target_compile_definitions(${t} PRIVATE USING_DIRECTX_HEADERS)
    endforeach()
    target_link_libraries(d3d12loadtest PRIVATE Microsoft::DirectX-Guids)
    target_link_libraries(xtd3d12 PRIVATE Microsoft::DirectX-Guids)
endif()

if(MSVC)
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE "$<IF:$<STREQUAL:${t},headertest>,/Wall,/W4>" /GR-)
    endforeach()
endif()

if(DEFINED COMPILER_DEFINES)
    foreach(t IN LISTS TEST_EXES)
      target_compile_definitions(${t} PRIVATE ${COMPILER_DEFINES})
      target_compile_options(${t} PRIVATE ${COMPILER_SWITCHES})
      target_link_options(${t} PRIVATE ${LINKER_SWITCHES})
    endforeach()
endif()

if(MINGW AND BUILD_DX11)
    target_link_options(bcvalidator PRIVATE -municode)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|IntelLLVM")
    target_compile_options(headertest PRIVATE -Wall -Wpedantic -Wextra)

    set(WarningsEXE "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-language-extension-token")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
        list(APPEND WarningsEXE "-Wno-unsafe-buffer-usage")
    endif()
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE ${WarningsEXE})
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE "-Wno-ignored-attributes" "-Walloc-size-larger-than=4GB")
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    foreach(t IN LISTS TEST_EXES)
      set_target_properties(${t} PROPERTIES CXX_STANDARD 14)
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(BUILD_FUZZING
       AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.32))
        foreach(t IN LISTS TEST_EXES)
          target_compile_options(${t} PRIVATE ${ASAN_SWITCHES})
          target_link_libraries(${t} PRIVATE ${ASAN_LIBS})
        endforeach()
    endif()
endif()

if(WIN32)
    foreach(t IN LISTS TEST_EXES)
      target_compile_definitions(${t} PRIVATE _WIN32_WINNT=${WINVER})
    endforeach()

    if(DISABLE_MSVC_ITERATOR_DEBUGGING)
      foreach(t IN LISTS TEST_EXES)
        target_compile_definitions(${t} PRIVATE _ITERATOR_DEBUG_LEVEL=0)
      endforeach()
    endif()
endif()
