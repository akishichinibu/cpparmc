#ifndef CPPARMC_COMPILE_BASE_H
#define CPPARMC_COMPILE_BASE_H

#include <cstddef>
#include <cassert>
#include <iostream>
#include <fmt/format.h>

constexpr static std::uint8_t magic_1 = '\x12';
constexpr static std::uint8_t magic_2 = '\x46';

//#define CPPARMC_DEBUG_BIT_STREAM
//#define CPPARMC_DEBUG_PRINT_MODEL
//#define CPPARMC_DEBUG_ARITHMETIC_ENCODER
//#define CPPARMC_DEBUG_ARITHMETIC_DECODER
//#define CPPARMC_DEBUG_BWT_ENCODER
//#define CPPARMC_DEBUG_BWT_DECODER
#define CPPARMC_TIMING

#define CPPARMC_DEBUG_FIBONACCI_ENCODE

//#define BWT_MULTI_THREAD

#define USING_PARALLEL_STL

#ifdef BWT_MULTI_THREAD
#include <thread>
#endif

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(_fmt, args...) std::cerr << fmt::format(_fmt, ##args) << "\n"
#else
#define DEBUG_PRINT(format, args...) do{} while(false)
#endif

namespace cpparmc::consts {
    constexpr auto allow_max_buffer_size = 128 * 1024 * 1;
    using DefaultSizeType = std::uint64_t;
}

#endif //CPPARMC_COMPILE_BASE_H
