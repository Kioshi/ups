cmake_minimum_required (VERSION 2.8)

set (PROJECT_NAME Bomberman)
project (${PROJECT_NAME})   
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}.exe)

if(CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options(-std=c++11) # CMake 2.8.12 or newer
  set(CMAKE_CXX_STANDARD 11) # C++11...
  set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
  set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
endif()

add_subdirectory(src)
