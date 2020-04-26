PROJECT(test_cpparmc LANGUAGES CXX)

FILE(GLOB CPPARMC_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
MESSAGE("Test file found: ${CPPARMC_TEST_FILES}")

FOREACH(testfile ${CPPARMC_TEST_FILES})

    GET_FILENAME_COMPONENT(testname ${testfile} NAME_WE)
    MESSAGE("Target `${testname}` for current test file ${testfile}. ")

    ADD_EXECUTABLE(${testname} ${testfile})
    TARGET_LINK_LIBRARIES(${testname} cpparmc spdlog)
    SET(CMAKE_CXX_FLAGS "-std=c++17 -O0 -g3 ${CMAKE_CXX_FLAGS}")
    TARGET_COMPILE_FEATURES(${testname} PUBLIC cxx_std_17)

ENDFOREACH(testfile ${CPPARMC_TEST_FILES})
