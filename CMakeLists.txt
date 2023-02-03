cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_CXX_STANDARD  14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECT_NAME Cpp-SocketLibrary)
set(SOURCE_DIR src)
set(FOO_SOURCE ${SOURCE_DIR}/**)
set(LIBRARIES_DIR libs)
# set(LIBRARIES A B)

project(${PROJECT_NAME} VERSION 1.0)

# GTest properties
enable_testing()
add_subdirectory(libs/googletest-1.13.0/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Project Configuration
add_executable(${PROJECT_NAME} ${FOO_SOURCE})

foreach(LIBRARY ${LIBRARIES})
    add_subdirectory("${LIBRARIES_DIR}/${LIBRARY}")
endforeach(LIBRARY)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

add_subdirectory(test)
