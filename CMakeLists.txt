cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECT_NAME Cpp-SocketLibrary)
set(SOURCE src/enums/SocketProtocol.cpp src/enums/SocketType.cpp src/serversocket/ServerSocket.cpp src/serversocket/ServerSocket.h src/socket/Socket.cpp src/socket/Socket.h src/socketexceptions/BindingException.hpp src/socketexceptions/SocketException.hpp src/socketexceptions/TimeoutException.hpp)
set(LIBRARIES_DIR libs)
# set(LIBRARIES A B)

project(${PROJECT_NAME} VERSION 1.0)

# GTest properties
enable_testing()
add_subdirectory(libs/googletest-1.13.0/googletest)
include_directories(libs/googletest-1.13.0/googletest/include libs/googletest-1.13.0/googletest/src)

# Project Configuration
add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} PRIVATE gtest) # link google test to this executable
target_link_libraries(${PROJECT_NAME} PRIVATE bluetooth)
target_link_libraries(${PROJECT_NAME} PRIVATE glib-2.0)

foreach(LIBRARY ${LIBRARIES})
    add_subdirectory("${LIBRARIES_DIR}/${LIBRARY}")
endforeach(LIBRARY)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# add_subdirectory(tests)
