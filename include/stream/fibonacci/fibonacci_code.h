#ifndef CPPARMC_FIBONACCI_CODE_H
#define CPPARMC_FIBONACCI_CODE_H

#include <array>

namespace cpparmc::consts {
    constexpr std::array<std::pair<u_char, u_int32_t>, 65536> fibonacci_code {{
#ifndef __CLION_IDE__
#include "stream/fibonacci/fibonacci_code.inc"
#endif
                                                                              }};
}
#endif //CPPARMC_FIBONACCI_CODE_H
