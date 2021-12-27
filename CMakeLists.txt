project(quad)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(glfw)
add_library(glad glad/glad.c)

add_executable(quad main.cpp gfx.cpp)
target_link_libraries(quad glad glfw)
include_directories(glfw/include)

