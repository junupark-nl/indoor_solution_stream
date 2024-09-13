cmake_minimum_required(VERSION 3.10)
project(SerialPacketStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(BOOST_ROOT "C:/Program Files/Boost/boost_1_86_0")
endif()

find_package(Boost 1.8 REQUIRED COMPONENTS system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    message(DEBUG "Boost at ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Boost not found!")
endif()

add_executable(${PROJECT_NAME}_parser src/parser.cpp)
add_executable(${PROJECT_NAME}_talker src/sample_talker.cpp)

target_link_libraries(${PROJECT_NAME}_parser ${Boost_LIBRARIES})