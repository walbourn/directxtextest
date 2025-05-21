cmake_minimum_required (VERSION 3.20)

project (xbox
  DESCRIPTION "DirectXTex Test Suite - Xbox extensions test"
  HOMEPAGE_URL "https://github.com/walbourn/directxtextest/wiki"
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
  message(FATAL_ERROR "DirectXTex Test Suite should be built by the main CMakeLists")
endif()

if(MINGW)
  message(FATAL_ERROR "This test does not support MinGW")
endif()

add_executable(${PROJECT_NAME} WIN32
    dds.cpp
    directxtex.cpp
    testutils.cpp
    tiling.cpp
    )

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Media
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
