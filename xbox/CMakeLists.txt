# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.21)

project (xtxbox
  DESCRIPTION "DirectXTex Test Suite - Xbox extensions test"
  HOMEPAGE_URL "https://github.com/walbourn/directxtextest/wiki"
  LANGUAGES CXX)

if(PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "DirectXTex Test Suite should be built by the main CMakeLists")
endif()

if(MINGW)
  message(FATAL_ERROR "This test does not support MinGW")
endif()

add_executable(${PROJECT_NAME}
    dds.cpp
    directxtest.cpp
    testutils.cpp
    tiling.cpp
    )

target_include_directories(${PROJECT_NAME} PRIVATE ../inc ../../Auxiliary)

target_link_libraries(${PROJECT_NAME} PRIVATE DirectXTex bcrypt.lib)

if(directxmath_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXMath)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /GR-)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILER_DEFINES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_SWITCHES})
target_link_options(${PROJECT_NAME} PRIVATE ${LINKER_SWITCHES})

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Media
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
