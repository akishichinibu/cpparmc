PROJECT(test_cpparmc LANGUAGES CXX)

FILE(GLOB CPPARMC_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
MESSAGE("Test file found: ${CPPARMC_TEST_FILES}")

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS "-std=c++17 -g3 ${CMAKE_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG " -O0 -pg")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ftree-vectorize -fopt-info-vec-missed -fopt-info-inline-missed")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)

MESSAGE(CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS})

FOREACH (testfile ${CPPARMC_TEST_FILES})

    GET_FILENAME_COMPONENT(testname ${testfile} NAME_WE)
    MESSAGE("*** Now compile `${testname}` for current test file ${testfile}. ")

    ADD_EXECUTABLE(${testname} ${testfile})
    TARGET_LINK_LIBRARIES(${testname} spdlog gtest tbb)
    TARGET_COMPILE_FEATURES(${testname} PUBLIC cxx_std_17)

ENDFOREACH (testfile ${CPPARMC_TEST_FILES})
