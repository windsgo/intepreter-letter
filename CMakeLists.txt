cmake_minimum_required(VERSION 3.14)

project(intp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-std=c++17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (NOT CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

add_compile_definitions(__ROOT__="${PROJECT_SOURCE_DIR}/")
include_directories(${PROJECT_SOURCE_DIR}/third_party/meojson/include/)

add_subdirectory(src)
add_subdirectory(tests)
