file(GLOB SOURCE *.h *.cpp)
file(GLOB LIB_SOURCE libs/*.h libs/*.cpp)

include_directories("$ENV{VK_SDK_PATH}\\include")
link_directories("$ENV{VK_SDK_PATH}\\Lib")

add_executable(Vulkan_Project
 ${SOURCE}
 ${LIB_SOURCE}
)


configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/packages.config ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/shaders/triangle.frag.spv ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/shaders/triangle.vert.spv ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")