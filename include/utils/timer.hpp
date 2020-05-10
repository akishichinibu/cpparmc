#ifndef CPPARMC_TIMER_HPP
#define CPPARMC_TIMER_HPP

#include <chrono>
#include <cstdio>

#include "compile_base.h"

#define START_TIMER(name) const auto __t0_##name = std::chrono::steady_clock::now()

#define END_TIMER(name) const auto __tn_##name = std::chrono::steady_clock::now()

#define END_TIMER_AND_OUTPUT_MS(name) END_TIMER(name); \
const auto __d_##name = std::chrono::duration<double>(__tn_##name - __t0_##name); \
DEBUG_PRINT("[PROFILE][{:s}] Used time: {:.4f} ms. ", #name, __d_##name.count() * 1000);

#endif //CPPARMC_TIMER_HPP
