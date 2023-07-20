cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME CppSocketLibraryTests)
project(${PROJECT_NAME})

set(SOURCE
        serversocket/ServerSocketTest.cpp
        socket/SocketTCPTest.cpp
        socket/SocketUDPTest.cpp
        socket/SocketBluetoothTest.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} PUBLIC
        gtest_main
        gtest
        CppSocketLibrary # Parent project
)

# Enable unit testing
enable_testing()

add_test(
        Name ${PROJECT_NAME}
        COMMAND ${PROJECT_NAME}
)
