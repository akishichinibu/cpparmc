PROJECT(cppacd LANGUAGES CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.13 FATAL_ERROR)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include/
        ${CMAKE_SOURCE_DIR}/src/
        ${CMAKE_SOURCE_DIR}/external/spdlog/include/
        ${CMAKE_SOURCE_DIR}/external/fmt/include/
        ${CMAKE_SOURCE_DIR}/external/CRCpp/inc/
)

ADD_SUBDIRECTORY(external/spdlog)
ADD_SUBDIRECTORY(external/fmt)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)
