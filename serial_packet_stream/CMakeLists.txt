cmake_minimum_required(VERSION 3.10)
project(SerialPacketStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    if(NOT DEFINED BOOST_ROOT)
        set(BOOST_ROOT "C:/Program Files/Boost/boost_1_86_0")
    endif()
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

find_package(Boost 1.8 REQUIRED COMPONENTS system)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    message(STATUS "Boost at ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Boost not found!")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}_parser src/parser.cpp)
add_executable(${PROJECT_NAME}_talker src/sample_talker.cpp)

target_link_libraries(${PROJECT_NAME}_parser Boost::system)
target_link_libraries(${PROJECT_NAME}_talker Boost::system)