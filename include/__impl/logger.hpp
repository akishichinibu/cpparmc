#ifndef CPPARMC_LOGGER_HPP
#define CPPARMC_LOGGER_HPP

#include "spdlog/sinks/stdout_color_sinks.h"

namespace cpparmc::log {

    auto logger = spdlog::stdout_color_mt("armc-cli");

}

#endif //CPPARMC_LOGGER_HPP
