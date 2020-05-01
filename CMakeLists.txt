CMAKE_MINIMUM_REQUIRED(VERSION 3.13 FATAL_ERROR)
PROJECT(cpparmc LANGUAGES CXX)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include/
        ${CMAKE_SOURCE_DIR}/src/
        ${CMAKE_SOURCE_DIR}/external/spdlog/include/
        ${CMAKE_SOURCE_DIR}/external/fmt/include/
        ${CMAKE_SOURCE_DIR}/external/CRCpp/inc/
        ${CMAKE_SOURCE_DIR}/external/googletest/googletest/include/
        ${CMAKE_SOURCE_DIR}/external/CLI11/include/
)

ADD_SUBDIRECTORY(external/spdlog)
ADD_SUBDIRECTORY(external/fmt)
ADD_SUBDIRECTORY(external/googletest)

ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(src)
SET(CMAKE_VERBOSE_MAKEFILE ON)
