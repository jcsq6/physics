cmake_minimum_required(VERSION 3.8)

project(physics)
set(CMAKE_CXX_STANDARD 20)

add_executable(physics src/apps/main.cpp src/src/bound.cpp src/src/world.cpp)
add_executable(collisions src/apps/test_collisions.cpp src/src/bound.cpp src/src/world.cpp)

add_compile_definitions($<$<CONFIG:Debug>:PHYSICS_DEBUG>)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

target_include_directories(physics PUBLIC src/gl src/physics)
target_include_directories(collisions PUBLIC src/gl src/physics)

target_link_libraries(physics PUBLIC OpenGL::GL GLEW::GLEW glfw glm::glm Eigen3::Eigen)
target_link_libraries(collisions PUBLIC OpenGL::GL GLEW::GLEW glfw glm::glm Eigen3::Eigen)