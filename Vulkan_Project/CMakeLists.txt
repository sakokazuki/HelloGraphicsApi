file(GLOB SOURCE *.h *.cpp)
file(GLOB LIB_SOURCE libs/*.h libs/*.cpp)


include_directories("$ENV{VK_SDK_PATH}\\include")
include_directories("${CMAKE_BINARY_DIR}\\packages\\glm.0.9.9.700\\build\\native\\include")
include_directories("${CMAKE_BINARY_DIR}\\packages\\glfw.3.3.2\\build\\native\\include")
include_directories("${}$(ProjectDir)\\packages\\glfw.3.3.2\\build\\native\\include")
link_directories("$ENV{VK_SDK_PATH}\\Lib")
link_directories("${CMAKE_BINARY_DIR}\\packages\\glfw.3.3.2\\build\\native\\lib\\static\\v${MSVC_TOOLSET_VERSION}\\${MSVC_CXX_ARCHITECTURE_ID}")

add_executable(Vulkan_Project
 ${SOURCE}
 ${LIB_SOURCE}
)
target_link_libraries(Vulkan_Project "vulkan-1.lib")
target_link_libraries(Vulkan_Project "glfw3.lib")


configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/packages.config ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/shaders/triangle.frag.spv ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/shaders/triangle.vert.spv ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/shaders/texture.frag.spv ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/shaders/texture.vert.spv ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Vulkan_Project/texture.tga ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")