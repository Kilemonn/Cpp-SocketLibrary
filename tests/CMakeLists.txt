cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME CppSocketLibraryTests)
project(${PROJECT_NAME})

# Reference: https://github.com/google/googletest/tree/main/googletest#incorporating-into-an-existing-cmake-project
include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


set(SOURCE
        serversocket/ServerSocketTest.cpp
        socket/SocketTCPTest.cpp
        socket/SocketUDPTest.cpp
        socket/SocketBluetoothTest.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})

if(CMAKE_HOST_WIN32)
   target_link_libraries(${PROJECT_NAME} PUBLIC
        gtest_main
        gtest
        CppSocketLibrary # Parent project
    )
endif()

if(CMAKE_HOST_UNIX)
   target_link_libraries(${PROJECT_NAME} PUBLIC
        gtest_main
        gtest
        bluetooth
        CppSocketLibrary # Parent project
    )
endif()

# Enable unit testing
enable_testing()

add_test(${PROJECT_NAME} ${PROJECT_NAME})

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${PROJECT_NAME})
