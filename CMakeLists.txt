cmake_minimum_required(VERSION 3.3)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Player.cpp Player.h Upstream.cpp Upstream.h Downstream.cpp Downstream.h Socket.cpp Socket.h Packet.cpp Packet.h)
add_executable(server ${SOURCE_FILES})