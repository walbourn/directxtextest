cmake_minimum_required (VERSION 3.20)

project (d3d9loadtest
  DESCRIPTION "DirectXTex Test Suite - DirectX 9 Standalone Loader Test"
  HOMEPAGE_URL "https://github.com/walbourn/directxtextest/wiki"
  LANGUAGES CXX)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  message(FATAL_ERROR "DirectXTex Test Suite should be built by the main CMakeLists")
endif()

add_executable(${PROJECT_NAME} WIN32
    Game.cpp
    Game.h
    Main.cpp
    StepTimer.h
    pch.h
    ../../DDSTextureLoader/DDSTextureLoader9.cpp
    ../../DDSTextureLoader/DDSTextureLoader9.h
    ../../ScreenGrab/ScreenGrab9.cpp
    ../../ScreenGrab/ScreenGrab9.h
    ../../WICTextureLoader/WICTextureLoader9.cpp
    ../../WICTextureLoader/WICTextureLoader9.h
)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)

target_include_directories(${PROJECT_NAME} PRIVATE
     ../../DDSTextureLoader
     ../../ScreenGrab
     ../../WICTextureLoader)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d9.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    )

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/cup_small.jpg
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/dx5_logo.dds
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
