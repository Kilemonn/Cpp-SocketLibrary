cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME CppSocketLibraryTests)
project(${PROJECT_NAME})

set(SOURCE
        serversocket/ServerSocketTest.cpp
        socket/SocketTest.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} PUBLIC
        gtest_Main
        CppSocketLibrary # Parent project
)

add_test(
        Name ServerSocketTest
        COMMAND ServerSocketTest
)

add_test(
        Name SocketTest
        COMMAND SocketTest
)

#include(GoogleTest)
#gtest_discover_tests(tests)
