file(GLOB SOURCE *.h *.cpp)
file(GLOB LIB_SOURCE libs/*.h libs/*.cpp)

add_executable(D3D12_Project
 ${SOURCE}
 ${LIB_SOURCE}
)

configure_file(${CMAKE_SOURCE_DIR}/D3D12_Project/packages.config ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/D3D12_Project/shaders/triangleVs.hlsl ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/D3D12_Project/shaders/trianglePs.hlsl ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/D3D12_Project/shaders/textureVs.hlsl ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/D3D12_Project/shaders/texturePs.hlsl ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/D3D12_Project/texture.tga ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

add_custom_command(TARGET D3D12_Project POST_BUILD
 COMMAND copy /Y
 "C:\\Program Files (x86)\\Windows Kits\\10\\Redist\\D3D\\x64\\dxcompiler.dll"
 "$(TargetDir)dxcompiler.dll"
)
add_custom_command(TARGET D3D12_Project POST_BUILD
 COMMAND copy /Y
 "C:\\Program Files (x86)\\Windows Kits\\10\\Redist\\D3D\\x64\\dxil.dll"
 "$(TargetDir)dxil.dll"
)

