cmake_minimum_required (VERSION 3.8)

add_executable(physics WIN32 "main.cpp" "world.cpp" "world.h" "object.cpp" "object.h" "poly.cpp" "poly.h" "mat.h" "vec2.cpp" "vec2.h" "vector_arithmetic.h" "collision.h"  "mat_transforms.cpp" "constraint.h")

target_include_directories(physics PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_compile_definitions(physics PRIVATE "$<$<CONFIG:Debug>:" "_DEBUG" ">" "$<$<CONFIG:Release>:" "NDEBUG" ">" "WIN32;" "_CONSOLE;" "UNICODE;" "_UNICODE")

target_compile_options(physics PRIVATE /std:c++latest; /Oi; /Gy; /permissive-; /sdl; /W2; /doc; $<$<CONFIG:Debug>:/Z7>; ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}; ${DEFAULT_CXX_EXCEPTION_HANDLING})

target_link_options(physics PRIVATE $<$<CONFIG:Debug>: /INCREMENTAL; /NODEFAULTLIB:LIBCMTD; /NODEFAULTLIB:MSVCRT /SUBSYSTEM:CONSOLE> $<$<CONFIG:Release>: /DEBUG:FULL /OPT:REF; /OPT:ICF; /INCREMENTAL:NO> /NODEFAULTLIB:LIBCMT)

target_link_libraries(physics PRIVATE SDL2main SDL2 winmm version Imm32 Setupapi)

target_link_directories(physics PRIVATE "${CMAKE_SOURCE_DIR}/lib")