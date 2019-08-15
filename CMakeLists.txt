cmake_minimum_required(VERSION 3.12)
project(HelloGraphicsApi)
add_subdirectory(D3D12_Project)
add_subdirectory(Vulkan_Project)

configure_file(${CMAKE_SOURCE_DIR}/.editorconfig ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)