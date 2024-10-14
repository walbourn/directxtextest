cmake_minimum_required (VERSION 3.20)

project (d3d11loadtest
  DESCRIPTION "DirectXTex Test Suite - DirectX 11 Standalone Loader Test"
  HOMEPAGE_URL "https://github.com/walbourn/directxtextest/wiki"
  LANGUAGES CXX)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  message(FATAL_ERROR "DirectXTex Test Suite should be built by the main CMakeLists")
endif()

add_executable(${PROJECT_NAME} WIN32
    Game.cpp
    Game.h
    DeviceResources.cpp
    DeviceResources.h
    Main.cpp
    MSAAHelper.cpp
    MSAAHelper.h
    ReadData.h
    StepTimer.h
    pch.h
    ../../DDSTextureLoader/DDSTextureLoader11.cpp
    ../../DDSTextureLoader/DDSTextureLoader11.h
    ../../ScreenGrab/ScreenGrab11.cpp
    ../../ScreenGrab/ScreenGrab11.h
    ../../WICTextureLoader/WICTextureLoader11.cpp
    ../../WICTextureLoader/WICTextureLoader11.h
)

if(NOT MINGW)
    target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
     ../../DDSTextureLoader
     ../../ScreenGrab
     ../../WICTextureLoader)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    DirectXTex
    )

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()

# Build HLSL shaders
add_custom_target(shaders11)

set_source_files_properties(VertexShader11.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(PixelShader11.hlsl PROPERTIES ShaderType "ps")

foreach(FILE VertexShader11.hlsl PixelShader11.hlsl)
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  get_source_file_property(shadertype ${FILE} ShaderType)
  add_custom_command(TARGET shaders11
                     COMMAND fxc.exe /nologo /Emain /T${shadertype}_4_0 $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi /Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.cso /Fd ${CMAKE_BINARY_DIR}/${FILE_WE}.pdb ${FILE}
                     MAIN_DEPENDENCY ${FILE}
                     COMMENT "HLSL ${FILE}"
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     VERBATIM)
endforeach(FILE)

add_dependencies(${PROJECT_NAME} shaders11)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/VertexShader11.cso
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/PixelShader11.cso
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/cup_small.jpg
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/dx5_logo.dds
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
