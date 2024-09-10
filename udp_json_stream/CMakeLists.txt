cmake_minimum_required(VERSION 3.10)
project(UdpJsonStreaming)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../library/json")
include_directories(${JSON_DIR}/include)

add_executable(${PROJECT_NAME}_listener src/listener.cpp)
add_executable(${PROJECT_NAME}_talker src/talker.cpp)

if (WIN32)
    target_link_libraries(${PROJECT_NAME}_listener wsock32 ws2_32)
    target_link_libraries(${PROJECT_NAME}_talker wsock32 ws2_32)
endif()
