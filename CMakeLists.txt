cmake_minimum_required(VERSION 3.8)

project(physics)
set(CMAKE_CXX_STANDARD 20)

add_library(plib STATIC src/src/bound.cpp src/src/world.cpp src/src/constraint.cpp)

add_executable(physics src/apps/main.cpp src/src/draw.cpp)
add_executable(collisions src/apps/test_collisions.cpp src/src/draw.cpp)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

target_include_directories(plib PUBLIC src/gl src/physics)
target_link_libraries(plib PUBLIC OpenGL::GL GLEW::GLEW glfw glm::glm Eigen3::Eigen)

target_link_libraries(physics PUBLIC plib)
target_link_libraries(collisions PUBLIC plib)