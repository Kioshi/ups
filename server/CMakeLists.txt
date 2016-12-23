cmake_minimum_required (VERSION 2.6)

set (PROJECT_NAME Bomberman)
project (${PROJECT_NAME})   
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})


add_subdirectory(src)    
#add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)