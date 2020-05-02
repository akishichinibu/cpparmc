#ifndef CPPARMC_COMPILE_BASE_H
#define CPPARMC_COMPILE_BASE_H

#include <cstddef>
#include <spdlog/spdlog.h>

constexpr static std::uint8_t magic_1 = '\x12';
constexpr static std::uint8_t magic_2 = '\x46';

//#define CPPARMC_DEBUG_BIT_STREAM
//#define CPPARMC_DEBUG_PRINT_MODEL
//#define CPPARMC_DEBUG_ARITHMETIC_ENCODER
//#define CPPARMC_DEBUG_ARITHMETIC_DECODER
#define CPPARMC_DEBUG_BWT_ENCODER
#define CPPARMC_DEBUG_BWT_DECODER
#define CPPARMC_TIMING

#define CPPARMC_DEBUG_FIBONACCI_ENCODE

//#define BWT_MULTI_THREAD

#define USING_PARALLEL_STL

#ifdef USING_PARALLEL_STL
//#include <pstl/algorithm_fwd.h>
#endif

#ifdef BWT_MULTI_THREAD
#include <thread>
#endif

#endif //CPPARMC_COMPILE_BASE_H
