cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_NAME CppSocketLibrary)

project(${PROJECT_NAME} VERSION 0.6.2)

set(HEADERS
        src/serversocket/ServerSocket.h
        src/serversocket/TCPServerSocket.h
        src/socket/Socket.h
        src/socket/ConnectionLessSocket.h
        src/socket/ConnectionOrientedSocket.h
        src/socket/TCPSocket.h
        src/socket/UDPSocket.h
        src/address/SocketAddress.h
        src/ipc/StreamIPCSocket.h
        src/ipc/IPCServerSocket.h
        src/ipc/IPCSocket.h
        src/ipc/DatagramIPCSocket.h
        src/socketexceptions/BindingException.hpp
        src/socketexceptions/SocketException.hpp
        src/socketexceptions/TimeoutException.hpp
        src/socketexceptions/SocketError.h

        src/enums/InternetProtocolVersion.h
)

set(SOURCE
        src/serversocket/TCPServerSocket.cpp
        src/socket/Socket.cpp
        src/socket/ConnectionOrientedSocket.cpp
        src/socket/TCPSocket.cpp
        src/socket/UDPSocket.cpp
        src/socketexceptions/SocketError.cpp
        src/address/SocketAddress.cpp
        src/ipc/StreamIPCSocket.cpp
        src/ipc/IPCServerSocket.cpp
        src/ipc/IPCSocket.cpp
        src/ipc/DatagramIPCSocket.cpp
)

# Project Configuration - Adding as a lib
add_library(${PROJECT_NAME} STATIC ${SOURCE} ${HEADERS})

if(CMAKE_HOST_WIN32)

endif()

if(CMAKE_HOST_UNIX)

endif()

add_subdirectory(tests)
