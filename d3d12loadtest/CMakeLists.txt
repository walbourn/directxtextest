cmake_minimum_required (VERSION 3.20)

project (d3d12loadtest
  DESCRIPTION "DirectXTex Test Suite - DirectX 12 Standalone Loader Test"
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
    ../../DDSTextureLoader/DDSTextureLoader12.cpp
    ../../DDSTextureLoader/DDSTextureLoader12.h
    ../../ScreenGrab/ScreenGrab12.cpp
    ../../ScreenGrab/ScreenGrab12.h
    ../../WICTextureLoader/WICTextureLoader12.cpp
    ../../WICTextureLoader/WICTextureLoader12.h
)

if(NOT MINGW)
    target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
     ../inc
     ../../DDSTextureLoader
     ../../ScreenGrab
     ../../WICTextureLoader)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d12.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib runtimeobject.lib
    DirectXTex
    )

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()

# Build HLSL shaders
set_source_files_properties(VertexShader12.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(PixelShader12.hlsl PROPERTIES ShaderType "ps")

foreach(FILE VertexShader12.hlsl PixelShader12.hlsl)
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  get_source_file_property(shadertype ${FILE} ShaderType)
  add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
                     COMMAND fxc.exe /nologo /Emain /T${shadertype}_4_0 $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi /Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.cso /Fd ${CMAKE_BINARY_DIR}/${FILE_WE}.pdb ${FILE}
                     COMMENT "HLSL ${FILE}"
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     VERBATIM)
endforeach(FILE)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/VertexShader12.cso
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/PixelShader12.cso
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/cup_small.jpg
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/dx5_logo.dds
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
