cmake_minimum_required(VERSION 3.8)

project(physics)
set(CMAKE_CXX_STANDARD 20)

add_executable(physics src/main.cpp src/bound.cpp src/world.cpp)
add_executable(collisions src/test_collisions.cpp src/bound.cpp src/world.cpp)

add_compile_definitions($<$<CONFIG:Debug>:PHYSICS_DEBUG>)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(physics PUBLIC OpenGL::GL GLEW::GLEW glfw glm::glm)
target_link_libraries(collisions PUBLIC OpenGL::GL GLEW::GLEW glfw glm::glm)