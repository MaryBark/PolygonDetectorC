## ecnfyjdrf
##sudo apt-get install -y \
##    libopencv-dev \
##    libjson-c-dev \
##    libglib2.0-dev \
##    zlib1g-dev \
##    pkg-config
# Облом с OpenCV: она не совместима напрямую с Си

#cmake_minimum_required(VERSION 3.10)
#project(PolygonDetectorC VERSION 1.0.0 LANGUAGES C)

#set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_STANDARD_REQUIRED ON)

## Options
#option(BUILD_TESTS "Build unit tests" OFF)
#option(BUILD_CLIENT "Build client application" ON)

## Find packages
#find_package(PkgConfig REQUIRED)

## OpenCV
#pkg_check_modules(OpenCV REQUIRED opencv4)
#message(STATUS "OpenCV found: ${OpenCV_VERSION}")

## json-c
#pkg_check_modules(JSON_C REQUIRED json-c)
#message(STATUS "json-c found")

## GLib
#pkg_check_modules(GLIB REQUIRED glib-2.0)
#message(STATUS "GLib found")

## Find all source files
#file(GLOB DETECTOR_SOURCES "src/*.c")
#file(GLOB SERVER_SOURCES "src/server.c" "src/main.c")
#file(GLOB CLIENT_SOURCES "client/*.c")

#message(STATUS "Found source files: ${DETECTOR_SOURCES}")

## Main library
#add_library(detector_lib STATIC ${DETECTOR_SOURCES})

#target_include_directories(detector_lib PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${OpenCV_INCLUDE_DIRS}
#    ${JSON_C_INCLUDE_DIRS}
#    ${GLIB_INCLUDE_DIRS}
#)

#target_link_libraries(detector_lib
#    ${OpenCV_LIBRARIES}
#    ${JSON_C_LIBRARIES}
#    ${GLIB_LIBRARIES}
#    m
#    pthread
#    z
#)

## Server executable
#if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/server.c AND
#   EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)
#    add_executable(detector_server src/server.c src/main.c)
#    target_link_libraries(detector_server detector_lib)
#    target_include_directories(detector_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
#    message(STATUS "Building server executable")
#else()
#    message(WARNING "Server source files not found, skipping server build")
#endif()

## Client executable
#if(BUILD_CLIENT AND CLIENT_SOURCES)
#    add_executable(detector_client ${CLIENT_SOURCES})
#    target_link_libraries(detector_client detector_lib)
#    target_include_directories(detector_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
#    message(STATUS "Building client executable")
#else()
#    message(STATUS "Client not built (sources not found or BUILD_CLIENT=OFF)")
#endif()

## Simple tests (only if BUILD_TESTS is ON)
#if(BUILD_TESTS)
#    file(GLOB TEST_SOURCES "tests/*.c")
#    if(TEST_SOURCES)
#        add_executable(simple_tests ${TEST_SOURCES})
#        target_link_libraries(simple_tests detector_lib m)
#        enable_testing()
#        add_test(NAME SimpleTests COMMAND simple_tests)
#        message(STATUS "Building tests")
#    else()
#        message(WARNING "Test sources not found in tests/ directory")
#    endif()
#endif()

## Print summary
#message(STATUS "")
#message(STATUS "========================================")
#message(STATUS "Configuration Summary:")
#message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
#message(STATUS "  OpenCV: ${OpenCV_VERSION}")
#message(STATUS "  json-c: found")
#message(STATUS "  GLib: found")
#message(STATUS "  Build client: ${BUILD_CLIENT}")
#message(STATUS "  Build tests: ${BUILD_TESTS}")
#message(STATUS "========================================")
#message(STATUS "")



cmake_minimum_required(VERSION 3.10)
project(PolygonDetectorC VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_CLIENT "Build client application" ON)

# Find json-c
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSON_C REQUIRED json-c)
pkg_check_modules(GLIB REQUIRED glib-2.0)

# Define source files (without OpenCV dependency)
set(DETECTOR_SOURCES
    src/geometry.c
    src/detector_stub.c
    src/polygon_processor.c
    src/protocol.c
)

set(SERVER_SOURCES
    src/server.c
    src/main.c
)

# Main library
add_library(detector_lib STATIC ${DETECTOR_SOURCES})

target_include_directories(detector_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${JSON_C_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(detector_lib
    ${JSON_C_LIBRARIES}
    ${GLIB_LIBRARIES}
    m
    pthread
    z
)

# Server executable
add_executable(detector_server ${SERVER_SOURCES})
target_link_libraries(detector_server detector_lib)
target_include_directories(detector_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Client executable (without OpenCV)
if(BUILD_CLIENT)
    add_executable(detector_client client/client.c)
    target_link_libraries(detector_client detector_lib)
    target_include_directories(detector_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  json-c: found")
message(STATUS "  GLib: found")
message(STATUS "  Using stub detector (no OpenCV required)")
message(STATUS "  Client: ${BUILD_CLIENT}")
message(STATUS "")
