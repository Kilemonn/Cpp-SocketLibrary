cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_NAME CppSocketLibrary)

project(${PROJECT_NAME} VERSION 1.0)

set(HEADERS
        src/serversocket/ServerSocket.h
        src/socket/Socket.h
        src/socketexceptions/BindingException.hpp
        src/socketexceptions/SocketException.hpp
        src/socketexceptions/TimeoutException.hpp
)

set(SOURCE
        src/enums/SocketProtocol.cpp
        src/enums/SocketType.cpp
        src/serversocket/ServerSocket.cpp
        src/socket/Socket.cpp
)

# Project Configuration - Adding as a lib
add_library(${PROJECT_NAME} STATIC ${SOURCE} ${HEADERS})

# Add libs
add_subdirectory(googletest)

include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib/x86_64-linux-gnu/glib-2.0/include)

target_link_libraries(${PROJECT_NAME} PRIVATE bluetooth)
target_link_libraries(${PROJECT_NAME} PRIVATE glib-2.0)

#foreach(LIBRARY ${LIBRARIES})
#    add_subdirectory("${LIBRARIES_DIR}/${LIBRARY}")
#endforeach(LIBRARY)
#target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

add_subdirectory(tests)
