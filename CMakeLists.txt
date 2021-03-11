cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS_DEBUG "/MDd")
set(CMAKE_CXX_FLAGS_RELEASE "/MD")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("physics")

add_subdirectory("src")