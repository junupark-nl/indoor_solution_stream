cmake_minimum_required(VERSION 3.10)
project(OptitrackStreaming)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(NATNET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/NatNetSDK")

if(WIN32)
    set(NATNET_LIB_DIR "${NATNET_DIR}/lib/windows")
    set(NATNET_LIB_NAME "NatNetLib")
    set(NATNET_LIB_EXT "dll")
else()
    set(NATNET_LIB_DIR "${NATNET_DIR}/lib/linux")
    set(NATNET_LIB_NAME "NatNet")
    set(NATNET_LIB_EXT "so")
endif()
message(STATUS "NatNet library directory: ${NATNET_LIB_DIR}")
message(STATUS "NatNet include directory: ${NATNET_DIR}/include")

include_directories(${NATNET_DIR}/include)
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${NATNET_LIB_DIR}/${NATNET_LIB_NAME}.${NATNET_LIB_EXT})
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NATNET_DIR}/lib/windows/${NATNET_LIB_NAME}.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NATNET_DIR}/lib/linux/lib${NATNET_LIB_NAME}.so"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()