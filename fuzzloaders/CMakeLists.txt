# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.20)

project (fuzzloaders
  DESCRIPTION "DirectXTex File Fuzzer"
  HOMEPAGE_URL "http://go.microsoft.com/fwlink/?LinkId=248926"
  LANGUAGES CXX)

if(CMAKE_VERSION VERSION_LESS 3.21)
  get_property(not_top DIRECTORY PROPERTY PARENT_DIRECTORY)
  if(not_top)
    set(PROJECT_IS_TOP_LEVEL false)
  else()
    set(PROJECT_IS_TOP_LEVEL true)
  endif()
endif()

if(PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "DirectXTex Fuzz Tester should be built by the main CMakeLists")
endif()

add_executable(${PROJECT_NAME}
    fuzzloaders.cpp
    ../../Texconv/PortablePixMap.cpp
    ../../Common/CmdLineHelpers.h)

target_include_directories(${PROJECT_NAME} PRIVATE ../../Common)

if(BUILD_FUZZING)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FUZZING_BUILD_MODE)
endif()

if(MINGW OR (NOT WIN32))
    find_package(directxmath CONFIG REQUIRED)
    find_package(directx-headers CONFIG REQUIRED)
else()
    find_package(directxmath CONFIG QUIET)
    find_package(directx-headers CONFIG QUIET)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE DirectXTex bcrypt.lib version.lib)

if(directxmath_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft::DirectXMath)
endif()

if(directx-headers_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USING_DIRECTX_HEADERS)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE dxguid.lib)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Wall /GR-)
endif()

if(DEFINED COMPILER_DEFINES)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILER_DEFINES})
    target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_SWITCHES})
    target_link_options(${PROJECT_NAME} PRIVATE ${LINKER_SWITCHES})
endif()

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|IntelLLVM")
    set(WarningsEXE "-Wpedantic" "-Wextra" "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-float-equal" "-Wno-global-constructors" "-Wno-language-extension-token" "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro" "-Wno-unused-macros" "-Wno-switch-enum")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
        list(APPEND WarningsEXE "-Wno-unsafe-buffer-usage")
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE ${WarningsEXE})
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-ignored-attributes" "-Walloc-size-larger-than=4GB")
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--allow-multiple-definition)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(BUILD_FUZZING
       AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.32))
          target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=fuzzer ${ASAN_SWITCHES})
          target_link_libraries(${PROJECT_NAME} PRIVATE ${ASAN_LIBS})
          target_link_options(${PROJECT_NAME} PRIVATE /IGNORE:4291)
    endif()

    set(WarningsEXE /wd4061 /wd4365 /wd4668 /wd4710 /wd4820 /wd5031 /wd5032 /wd5039 /wd5045)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.34)
      list(APPEND WarningsEXE /wd5262 /wd5264)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.50)
      list(APPEND WarningsEXE /wd4865)
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE ${WarningsEXE})
endif()

if(WIN32)
    # Windows 10 is used here to build the DirectX 12 code paths as well
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=${WINVER})
endif()
