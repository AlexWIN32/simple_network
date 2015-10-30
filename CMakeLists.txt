cmake_minimum_required(VERSION 2.8.6)
project(simple_network)

set(VERSION 1.0)
set(BUILD_NUM 001)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(CMAKE_CXX_FLAGS "-g -Wall -Wno-deprecated ${CMAKE_CXX_FLAGS} -fPIC")
    if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif()
endif()

if(WIN32)
    find_library(SSL_LIB NAMES ssl libeay32 PATHS ${SSL_LIB_PATH})
else()
    find_library(SSL_LIB NAMES ssl libeay32)
endif()

if(NOT SSL_LIB)
   message(FATAL_ERROR "!! ssl library not found")
endif()

if(WIN32)
    find_library(SSL_CRYPTO_LIB ssleay32 PATHS ${SSL_LIB_PATH})
else()
    find_library(SSL_CRYPTO_LIB crypto)
endif()

if(NOT SSL_CRYPTO_LIB)
    message(FATAL_ERROR "!! crypto library not found")
endif()

set(SSL_LIB ${SSL_CRYPTO_LIB} ${SSL_LIB})

if(WIN32)
    add_definitions(-DWIN32)
else()
    add_definitions(-DUNIX)
endif()

include_directories(${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(lib)
if(UNIX)
    add_subdirectory(simpleMessanger)
endif()
