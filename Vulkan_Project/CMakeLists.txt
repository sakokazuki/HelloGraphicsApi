file(GLOB SOURCE *.h *.cpp)
file(GLOB LIB_SOURCE libs/*.h libs/*.cpp)

add_executable(Vulkan_Project
 ${SOURCE}
 ${LIB_SOURCE}
)



set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")


