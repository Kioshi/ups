file(GLOB_RECURSE shared_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)

add_executable(${PROJECT_NAME}.exe ${shared_SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME}.exe pthread)