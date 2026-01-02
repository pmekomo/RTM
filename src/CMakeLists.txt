# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name
project(RTM VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable linker dependency files
set(CMAKE_LINK_DEPENDS_USE_COMPILER FALSE)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -ansi -pedantic -O0 -g")

# Find required libraries
find_package(Threads REQUIRED)
find_package(LibXml2)

# Include directories
if(LibXml2_FOUND)
include_directories(${LIBXML2_INCLUDE_DIR})
endif()

# Source files for each executable
set(SERVER_SOURCES
    server.cpp
    equipement.cpp
    files.cpp
    utile.cpp
)

set(CLIENT_SOURCES
    client.cpp
    service.cpp
    files.cpp
    utile.cpp
)

set(XMLMANAGER_SOURCES
    xmlmanager.cpp
)

# Add executables
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server Threads::Threads)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client Threads::Threads)

if(LibXml2_FOUND)
add_executable(xmlmanager ${XMLMANAGER_SOURCES})
target_link_libraries(xmlmanager ${LIBXML2_LIBRARIES})
endif()

# Set output directory for executables
if(LibXml2_FOUND)
set_target_properties(server client xmlmanager PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../build
)
else()
set_target_properties(server client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../build
)
endif()