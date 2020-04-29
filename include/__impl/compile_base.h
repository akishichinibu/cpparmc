#ifndef CPPARMC_COMPILE_BASE_H
#define CPPARMC_COMPILE_BASE_H

#include <cstddef>
#include <spdlog/spdlog.h>

//spdlog::set_level(spdlog::level::debug);

typedef unsigned char u_char;

constexpr static u_char magic_1 = '\x12';
constexpr static u_char magic_2 = '\x46';

//#define CPPARMC_DEBUG_BIT_STREAM
//#define CPPARMC_DEBUG_PRINT_MODEL
//#define CPPARMC_DEBUG_ARITHMETIC_ENCODER
//#define CPPARMC_DEBUG_ARITHMETIC_DECODER
//#define CPPARMC_DEBUG_BWT_ENCODER
//#define CPPARMC_DEBUG_BWT_DECODER

//#define BWT_MULTI_THREAD

#ifdef BWT_MULTI_THREAD
#include <thread>
#endif

#endif //CPPARMC_COMPILE_BASE_H
