CMAKE_MINIMUM_REQUIRED(VERSION 3.13 FATAL_ERROR)
PROJECT(cpparmc LANGUAGES CXX)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include/
        ${CMAKE_SOURCE_DIR}/src/
        ${CMAKE_SOURCE_DIR}/external/spdlog/include/
        ${CMAKE_SOURCE_DIR}/external/fmt/include/
        ${CMAKE_SOURCE_DIR}/external/CRCpp/inc/
        ${CMAKE_SOURCE_DIR}/external/googletest/googletest/include/
)

ADD_SUBDIRECTORY(external/spdlog)
ADD_SUBDIRECTORY(external/fmt)
ADD_SUBDIRECTORY(external/googletest)

ADD_SUBDIRECTORY(test)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS -std=c++17)
SET(CMAKE_CXX_FLAGS_DEBUG " -O0 -g3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -march=native")
