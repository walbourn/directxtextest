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

if (WIN32 AND (NOT DEFINED ENV{DIRECTXTEX_MEDIA_PATH}))
  message(WARNING "Tests make use of DIRECTXTEX_MEDIA_PATH environment variable to find media")
endif()

set(TEST_EXES headertest standalonetest)
add_executable(headertest headertest/main.cpp)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/standalonetest)

# d3d9/11/12loadtest
list(APPEND TEST_EXES d3d9loadtest d3d11loadtest d3d12loadtest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/d3d9loadtest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/d3d11loadtest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/d3d12loadtest)

add_test(NAME "d3d9loadtest" COMMAND d3d9loadtest -ctest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(d3d9loadtest PROPERTIES TIMEOUT 30)

add_test(NAME "d3d11loadtest" COMMAND d3d11loadtest -ctest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(d3d11loadtest PROPERTIES TIMEOUT 30)

add_test(NAME "d3d12loadtest" COMMAND d3d12loadtest -ctest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(d3d12loadtest PROPERTIES TIMEOUT 30)

#TODO: platformtest

# bcvalidator
list(APPEND TEST_EXES bcvalidator)
add_executable(bcvalidator bcvalidator/bcvalidator.cpp)

# fuzzloaders
list(APPEND TEST_EXES fuzzloaders)
add_executable(fuzzloaders fuzzloaders/fuzzloaders.cpp ../Texconv/PortablePixMap.cpp)

# DDS
list(APPEND TEST_EXES xtdds)
add_executable(xtdds dds/dds.cpp dds/directxtest.cpp dds/testutils.cpp)
target_include_directories(xtdds PRIVATE ./inc)
add_test(NAME "dds" COMMAND xtdds)
set_tests_properties(dds PROPERTIES TIMEOUT 60)

# HDR
list(APPEND TEST_EXES xthdr)
add_executable(xthdr hdr/hdr.cpp hdr/directxtest.cpp hdr/testutils.cpp)
target_include_directories(xthdr PRIVATE ./inc)
add_test(NAME "hdr" COMMAND xthdr)
set_tests_properties(hdr PROPERTIES TIMEOUT 450)

# PPM
list(APPEND TEST_EXES xtppm)
add_executable(xtppm ppm/ppm.cpp ppm/directxtest.cpp ppm/testutils.cpp ../Texconv/PortablePixMap.cpp)
target_include_directories(xtppm PRIVATE ./inc)
add_test(NAME "ppm" COMMAND xtppm)
set_tests_properties(ppm PROPERTIES TIMEOUT 120)

# TGA
list(APPEND TEST_EXES xttga)
add_executable(xttga tga/tga.cpp tga/directxtest.cpp tga/testutils.cpp)
target_include_directories(xttga PRIVATE ./inc)
add_test(NAME "tga" COMMAND xttga)
set_tests_properties(tga PROPERTIES TIMEOUT 60)

# WIC
list(APPEND TEST_EXES xtwic)
add_executable(xtwic wic/wic.cpp wic/directxtest.cpp wic/testutils.cpp)
target_include_directories(xtwic PRIVATE ./inc)
add_test(NAME "wic" COMMAND xtwic)
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
set_tests_properties(tex PROPERTIES TIMEOUT 400)

# filter
list(APPEND TEST_EXES xtfilter)
add_executable(xtfilter
    filter/coverage.cpp
    filter/mipmaps.cpp
    filter/resize.cpp
    filter/filter.h filter/directxtest.cpp filter/testutils.cpp)
target_include_directories(xtfilter PRIVATE ./inc)
add_test(NAME "filter" COMMAND xtfilter)
set_tests_properties(filter PROPERTIES TIMEOUT 240)

# compress
list(APPEND TEST_EXES xtcompress)
add_executable(xtcompress compress/compress.cpp compress/directxtest.cpp compress/testutils.cpp)
target_include_directories(xtcompress PRIVATE ./inc)
target_link_libraries(xtcompress PRIVATE d3d11.lib)
add_test(NAME "compress" COMMAND xtcompress)
set_tests_properties(compress PROPERTIES TIMEOUT 2000)

# d3d11
list(APPEND TEST_EXES xtd3d11)
add_executable(xtd3d11 d3d11/d3d11.cpp d3d11/directxtest.cpp d3d11/testutils.cpp)
target_include_directories(xtd3d11 PRIVATE ./inc ./d3d11/shaders)
target_link_libraries(xtd3d11 PRIVATE d3d11.lib)
add_test(NAME "d3d11" COMMAND xtd3d11)
set_tests_properties(d3d11 PROPERTIES TIMEOUT 120)

# d3d12
list(APPEND TEST_EXES xtd3d12)
add_executable(xtd3d12 d3d12/d3d12.cpp d3d12/directxtest.cpp d3d12/testutils.cpp)
target_include_directories(xtd3d12 PRIVATE ./inc ./d3d12/shaders)
target_link_libraries(xtd3d12 PRIVATE d3d12.lib dxguid.lib dxgi.lib)
add_test(NAME "d3d12" COMMAND xtd3d12)
set_tests_properties(d3d12 PROPERTIES TIMEOUT 120)

message("INFO: Enabled tests: ${TEST_EXES}")
foreach(t IN LISTS TEST_EXES)
  target_link_libraries(${t} PRIVATE DirectXTex bcrypt.lib)
endforeach()

if (MINGW OR (NOT WIN32) OR VCPKG_TOOLCHAIN)
    foreach(t IN LISTS TEST_EXES)
      target_link_libraries(${t} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectXMath)
      target_compile_definitions(${t} PRIVATE USING_DIRECTX_HEADERS)
    endforeach()
    target_link_libraries(d3d12loadtest PRIVATE Microsoft::DirectX-Guids)
    target_link_libraries(xtd3d12 PRIVATE Microsoft::DirectX-Guids)
endif()

if (MSVC)
  foreach(t IN LISTS TEST_EXES)
    target_compile_options(${t} PRIVATE "$<IF:$<STREQUAL:${t},headertest>,/Wall,/W4>" /GR- /fp:fast "$<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>")
    target_link_options(${t} PRIVATE /DYNAMICBASE /NXCOMPAT)
  endforeach()

  if((${CMAKE_SIZEOF_VOID_P} EQUAL 4) AND (NOT (${DIRECTX_ARCH} MATCHES "^arm")))
    foreach(t IN LISTS TEST_EXES)
      target_link_options(${t} PRIVATE /SAFESEH)
    endforeach()
  endif()

  if((MSVC_VERSION GREATER_EQUAL 1928) AND (CMAKE_SIZEOF_VOID_P EQUAL 8)
    AND NOT ENABLE_OPENEXR_SUPPORT
    AND ((NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang")) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)))
      foreach(t IN LISTS TEST_EXES)
        target_compile_options(${t} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>")
        target_link_options(${t} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>")
      endforeach()
    endif()
endif()

if(NOT (${DIRECTX_ARCH} MATCHES "^arm"))
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        set(ARCH_SSE2 $<$<CXX_COMPILER_ID:MSVC>:/arch:SSE2> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    else()
        set(ARCH_SSE2 $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    endif()

    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE ${ARCH_SSE2})
    endforeach()
endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options(headertest PRIVATE -Wall -Wpedantic -Wextra)
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-language-extension-token")
    endforeach()
endif()
if(MINGW)
    foreach(t IN LISTS TEST_EXES)
      target_compile_options(${t} PRIVATE -Wno-ignored-attributes)
    endforeach()
    target_link_options(bcvalidator PRIVATE -municode)
    target_link_options(fuzzloaders PRIVATE -municode)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    foreach(t IN LISTS TEST_EXE)
      target_compile_options(${t} PRIVATE /sdl /permissive- /JMC- /Zc:__cplusplus)
    endforeach()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.24)
      foreach(t IN LISTS TEST_EXES)
        target_compile_options(${t} PRIVATE /ZH:SHA_256)
      endforeach()
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
      foreach(t IN LISTS TEST_EXES)
        target_compile_options(${t} PRIVATE /Zc:preprocessor /wd5105)
      endforeach()
    endif()

    if ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.27) AND (NOT ((${DIRECTX_ARCH} MATCHES "^arm"))))
      foreach(t IN LISTS TEST_EXES)
        target_link_options(${t} PRIVATE /CETCOMPAT)
      endforeach()
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.34)
      foreach(t IN LISTS TEST_EXES)
        target_compile_options(${t} PRIVATE /wd5262 /wd5264)
      endforeach()
    endif()
endif()

if(WIN32)
  foreach(t IN LISTS TEST_EXES)
    target_compile_definitions(${t} PRIVATE _UNICODE UNICODE _WIN32_WINNT=${WINVER})
  endforeach()
endif()